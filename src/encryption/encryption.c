/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <jansson.h>

#include "encryption.h"

const char *ENCRYPTION_KEY = "3559b435f24b297a79c68b9709ef2125";

int encode_base64(unsigned const char* input, char** buffer, int input_size)
{
  BIO *bio, *b64;
  FILE* stream;
  int encoded_size = 4 * ceil((double)input_size / 3);
  *buffer = malloc(encoded_size + 1);
  if (*buffer == NULL) {
    return -1;
  }

  stream = fmemopen(*buffer, encoded_size + 1, "w");
  if (stream == NULL) {
    free(*buffer);
    *buffer = NULL;
    return -1;
  }
  b64 = BIO_new(BIO_f_base64());
  bio = BIO_new_fp(stream, BIO_NOCLOSE);
  bio = BIO_push(b64, bio);
  BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); 
  BIO_write(bio, input, input_size);
  BIO_flush(bio);
  BIO_free_all(bio);
  fclose(stream);

  return 0; 
}

int calculate_decoded_len(const char* base64_input) 
{ 
  int len = strlen(base64_input);
  int padding = 0;

  if (base64_input[len - 1] == '=' && base64_input[len - 2] == '=') 
    padding = 2;
  else if (base64_input[len - 1] == '=') 
    padding = 1;

  return (int)len * 0.75 - padding;
}

int decode_base64(char* base64_input, unsigned char** buffer)
{
  BIO *bio, *b64;
  int decode_len = calculate_decoded_len(base64_input),
      len = 0;
  *buffer = (unsigned char*)malloc(decode_len + 1);
  if (*buffer == NULL) {
    return -1;
  }

  FILE* stream = fmemopen(base64_input, strlen(base64_input), "r");
  if (stream == NULL) {
    free(*buffer);
    *buffer = NULL;
    return -1;
  }

  b64 = BIO_new(BIO_f_base64());
  bio = BIO_new_fp(stream, BIO_NOCLOSE);
  bio = BIO_push(b64, bio);
  BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
  len = BIO_read(bio, *buffer, strlen(base64_input));
  (*buffer)[len] = '\0';

  BIO_free_all(bio);
  fclose(stream);

  return decode_len;
}

int aes_init_enc(EVP_CIPHER_CTX *e_ctx, unsigned char *encryption_iv)
{
  EVP_CIPHER_CTX_init(e_ctx);
  if (EVP_EncryptInit_ex(e_ctx, EVP_aes_256_cbc(), NULL,
                         (const unsigned char *)ENCRYPTION_KEY, encryption_iv) != 1) {
    return -1;
  }
  return 0;
}

int aes_init_dec(EVP_CIPHER_CTX *d_ctx, unsigned char *decryption_iv)
{
  EVP_CIPHER_CTX_init(d_ctx);
  if (EVP_DecryptInit_ex(d_ctx, EVP_aes_256_cbc(), NULL,
                         (const unsigned char *)ENCRYPTION_KEY, decryption_iv) != 1) {
    return -1;
  }
  return 0;
}

/*
 * Encrypt *len bytes of data
 * All data going in & out is considered binary (unsigned char[])
 */
unsigned char *aes_encrypt(EVP_CIPHER_CTX *e, unsigned char *plain_text, int *len)
{
  /* max ciphertext len for a n bytes of plain_text is n + AES_BLOCK_SIZE -1 bytes */
  int c_len = *len + AES_BLOCK_SIZE, f_len = 0;
  unsigned char *cipher_text = malloc(c_len);
  if (cipher_text == NULL) {
    return NULL;
  }

  /* allows reusing of 'e' for multiple encryption cycles */
  if (EVP_EncryptInit_ex(e, NULL, NULL, NULL, NULL) != 1) {
    free(cipher_text);
    return NULL;
  }

  /* update ciphertext, c_len is filled with the length of ciphertext generated,
    *len is the size of plain_text in bytes */
  if (EVP_EncryptUpdate(e, cipher_text, &c_len, plain_text, *len) != 1) {
    free(cipher_text);
    return NULL;
  }

  /* update ciphertext with the final remaining bytes */
  if (EVP_EncryptFinal_ex(e, cipher_text+c_len, &f_len) != 1) {
    free(cipher_text);
    return NULL;
  }

  *len = c_len + f_len;

  return cipher_text;
}

/*
 * Decrypt *len bytes of ciphertext
 */
unsigned char *aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *cipher_text, int *len)
{
  /* plain_text will always be equal to or lesser than length of ciphertext*/
  int p_len = *len, f_len = 0;
  unsigned char *plain_text = malloc(p_len + 1);
  if (plain_text == NULL) {
    return NULL;
  }

  if (EVP_DecryptInit_ex(e, NULL, NULL, NULL, NULL) != 1) {
    free(plain_text);
    return NULL;
  }

  if (EVP_DecryptUpdate(e, plain_text, &p_len, cipher_text, *len) != 1) {
    free(plain_text);
    return NULL;
  }

  if (EVP_DecryptFinal_ex(e, plain_text+p_len, &f_len) != 1) {
    free(plain_text);
    return NULL;
  }

  *len = p_len + f_len;
  return plain_text;
}

char *encrypt_packet(json_t *json_input)
{
  unsigned char encryption_iv[IV_LENGTH];

  for (int i = 0; i < IV_LENGTH; i++) {
    encryption_iv[i] = (unsigned char)rand() % 256;
  }

  EVP_CIPHER_CTX *enc_context = EVP_CIPHER_CTX_new();
  if (enc_context == NULL) {
    return NULL;
  }

  if (aes_init_enc(enc_context, encryption_iv) != 0) {
    EVP_CIPHER_CTX_free(enc_context);
    return NULL;
  }

  char *json_string = json_dumps(json_input, 0);
  if (json_string == NULL) {
    EVP_CIPHER_CTX_free(enc_context);
    return NULL;
  }

  int len = strlen(json_string);
  unsigned char *ciphered_json = aes_encrypt(enc_context, (unsigned char *)json_string, &len);

  free(json_string);
  EVP_CIPHER_CTX_free(enc_context);

  if (ciphered_json == NULL) {
    return NULL;
  }

  unsigned char *cipher_with_appended_iv = malloc((len + IV_LENGTH) * sizeof(unsigned char));
  if (cipher_with_appended_iv == NULL) {
    free(ciphered_json);
    return NULL;
  }

  memcpy(cipher_with_appended_iv, encryption_iv, IV_LENGTH);
  memcpy(cipher_with_appended_iv + IV_LENGTH, ciphered_json, len);

  free(ciphered_json);

  char *base64_buffer = NULL;
  if (encode_base64(cipher_with_appended_iv, &base64_buffer, len + IV_LENGTH) != 0) {
    free(cipher_with_appended_iv);
    return NULL;
  }

  free(cipher_with_appended_iv);

  return base64_buffer;
}

int decrypt_packet(char *base64_input, json_t **json_output)
{
  unsigned char *base64_buffer = NULL;
  int decoded_len = decode_base64(base64_input, &base64_buffer);
  if (decoded_len < 0 || base64_buffer == NULL) {
    return -1;
  }

  if (decoded_len < IV_LENGTH) {
    free(base64_buffer);
    return -1;
  }

  int len = decoded_len - IV_LENGTH;

  unsigned char *decryption_iv = malloc(IV_LENGTH * sizeof(unsigned char));
  if (decryption_iv == NULL) {
    free(base64_buffer);
    return -1;
  }

  memcpy(decryption_iv, base64_buffer, IV_LENGTH);
  memmove(base64_buffer, base64_buffer + IV_LENGTH, len);

  EVP_CIPHER_CTX *dec_context = EVP_CIPHER_CTX_new();
  if (dec_context == NULL) {
    free(decryption_iv);
    free(base64_buffer);
    return -1;
  }

  if (aes_init_dec(dec_context, decryption_iv) != 0) {
    EVP_CIPHER_CTX_free(dec_context);
    free(decryption_iv);
    free(base64_buffer);
    return -1;
  }

  unsigned char *decrypted_json_string = aes_decrypt(dec_context, base64_buffer, &len);

  EVP_CIPHER_CTX_free(dec_context);
  free(decryption_iv);
  free(base64_buffer);

  if (decrypted_json_string == NULL) {
    return -1;
  }

  decrypted_json_string[len] = '\0';

  *json_output = json_loads((char *)decrypted_json_string, 0, NULL);

  free(decrypted_json_string);

  if (*json_output == NULL) {
    return -1;
  }

  return 0;
}
