#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <jansson.h>

#include "encryption.h"

const unsigned char *ENCRYPTION_KEY = "3559b435f24b297a79c68b9709ef2125";
const unsigned char *ENCRYPTION_IV = "0000000000000000"; // FIXME: hardcoded, this should be random
const int IV_LENGTH = 16;

int encode_base64(unsigned const char* input, char** buffer, int input_size)
{ 
  BIO *bio, *b64;
  FILE* stream;
  int encoded_size = 4 * ceil((double)input_size / 3);
  *buffer = malloc(encoded_size + 1);

  stream = fmemopen(*buffer, encoded_size + 1, "w");
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
  *buffer = (char*)malloc(decode_len + 1);
  FILE* stream = fmemopen(base64_input, strlen(base64_input), "r");

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

int aes_init_enc(EVP_CIPHER_CTX *e_ctx)
{
  EVP_CIPHER_CTX_init(e_ctx);
  EVP_EncryptInit_ex(e_ctx, EVP_aes_256_cbc(), NULL, ENCRYPTION_KEY, ENCRYPTION_IV);
  return 0;
}

int aes_init_dec(EVP_CIPHER_CTX *d_ctx, unsigned char *decryption_iv)
{
  EVP_CIPHER_CTX_init(d_ctx);
  EVP_DecryptInit_ex(d_ctx, EVP_aes_256_cbc(), NULL, ENCRYPTION_KEY, decryption_iv);
  return 0;
}

/*
 * Encrypt *len bytes of data
 * All data going in & out is considered binary (unsigned char[])
 */
unsigned char *aes_encrypt(EVP_CIPHER_CTX *e, char *plain_text, int *len)
{
  /* max ciphertext len for a n bytes of plain_text is n + AES_BLOCK_SIZE -1 bytes */
  int c_len = *len + AES_BLOCK_SIZE, f_len = 0;
  unsigned char *cipher_text = malloc(c_len);

  /* allows reusing of 'e' for multiple encryption cycles */
  EVP_EncryptInit_ex(e, NULL, NULL, NULL, NULL);

  /* update ciphertext, c_len is filled with the length of ciphertext generated,
    *len is the size of plain_text in bytes */
  EVP_EncryptUpdate(e, cipher_text, &c_len, plain_text, *len);

  /* update ciphertext with the final remaining bytes */
  EVP_EncryptFinal_ex(e, cipher_text+c_len, &f_len);

  *len = c_len + f_len;

  return cipher_text;
}

/*
 * Decrypt *len bytes of ciphertext
 */
char *aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *cipher_text, int *len)
{
  /* plain_text will always be equal to or lesser than length of ciphertext*/
  int p_len = *len, f_len = 0;
  char *plain_text = malloc(p_len);
  
  EVP_DecryptInit_ex(e, NULL, NULL, NULL, NULL);
  EVP_DecryptUpdate(e, plain_text, &p_len, cipher_text, *len);
  EVP_DecryptFinal_ex(e, plain_text+p_len, &f_len);

  *len = p_len + f_len;
  return plain_text;
}

unsigned char *encrypt_packet(json_t *json)
{
  EVP_CIPHER_CTX *enc_context = EVP_CIPHER_CTX_new();
  aes_init_enc(enc_context);
  
  unsigned char *ciphered_json = NULL;

  char *json_string = json_dumps(json, 0);
  int len = strlen(json_string);

  ciphered_json = aes_encrypt(enc_context, json_string, &len);
  
  unsigned char* cipher_with_appended_iv = 
    (unsigned char*)malloc((len + IV_LENGTH) * sizeof(unsigned char));

  memset(cipher_with_appended_iv, '0', IV_LENGTH); // FIXME: hardcoded IV
  memcpy(cipher_with_appended_iv + IV_LENGTH, ciphered_json, len);

  char *base64_buffer = NULL;
  encode_base64(cipher_with_appended_iv, &base64_buffer, len + IV_LENGTH);

  free(cipher_with_appended_iv);
  free(json_string);

  return base64_buffer;
}

json_t *decrypt_packet(char *base64_input) 
{
  unsigned char *base64_buffer = NULL;
  int len = decode_base64(base64_input, &base64_buffer) - IV_LENGTH;

  unsigned char *decryption_iv = (unsigned char*)malloc(IV_LENGTH * sizeof(unsigned char));
  memcpy(decryption_iv, base64_buffer, IV_LENGTH);
  memcpy(base64_buffer, base64_buffer + IV_LENGTH, len);

  EVP_CIPHER_CTX *dec_context = EVP_CIPHER_CTX_new();
  aes_init_dec(dec_context, decryption_iv);

  char *decrypted_json_string = aes_decrypt(dec_context, base64_buffer, &len);

  json_t *json = json_loads(decrypted_json_string, 0, NULL);

  free(decryption_iv);
  free(base64_buffer);

  return json;
}
