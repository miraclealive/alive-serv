/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <jansson.h>
#include <openssl/rand.h>

#include "encryption.h"

static const char *ENCRYPTION_KEY = "3559b435f24b297a79c68b9709ef2125";

typedef struct {
  EVP_CIPHER_CTX *enc_ctx;
  EVP_CIPHER_CTX *dec_ctx;
} thread_cipher_ctx_t;

static pthread_key_t thread_ctx_slot;
static pthread_once_t thread_ctx_once = PTHREAD_ONCE_INIT;
static int thread_ctx_init_failed = 0;

static void destroy_thread_cipher_ctx(void *ptr)
{
  if (ptr != NULL) {
    thread_cipher_ctx_t *ctx = ptr;
    if (ctx->enc_ctx != NULL) {
      EVP_CIPHER_CTX_free(ctx->enc_ctx);
    }
    if (ctx->dec_ctx != NULL) {
      EVP_CIPHER_CTX_free(ctx->dec_ctx);
    }
    free(ctx);
  }
}

static void create_thread_ctx_slot(void)
{
  if (pthread_key_create(&thread_ctx_slot, destroy_thread_cipher_ctx) != 0) {
    thread_ctx_init_failed = 1;
  }
}

static thread_cipher_ctx_t *get_thread_cipher_ctx(void)
{
  pthread_once(&thread_ctx_once, create_thread_ctx_slot);
  if (thread_ctx_init_failed) {
    return NULL;
  }

  thread_cipher_ctx_t *ctx = pthread_getspecific(thread_ctx_slot);
  if (ctx != NULL) {
    return ctx;
  }

  ctx = malloc(sizeof(thread_cipher_ctx_t));
  if (ctx == NULL) {
    return NULL;
  }

  ctx->enc_ctx = NULL;
  ctx->dec_ctx = NULL;

  unsigned char dummy_iv[IV_LENGTH] = {0};

  ctx->enc_ctx = EVP_CIPHER_CTX_new();
  if (ctx->enc_ctx == NULL) {
    free(ctx);
    return NULL;
  }

  ctx->dec_ctx = EVP_CIPHER_CTX_new();
  if (ctx->dec_ctx == NULL) {
    EVP_CIPHER_CTX_free(ctx->enc_ctx);
    free(ctx);
    return NULL;
  }

  if (EVP_EncryptInit_ex(ctx->enc_ctx, EVP_aes_256_cbc(), NULL,
                         (const unsigned char *)ENCRYPTION_KEY, dummy_iv) != 1) {
    EVP_CIPHER_CTX_free(ctx->enc_ctx);
    EVP_CIPHER_CTX_free(ctx->dec_ctx);
    free(ctx);
    return NULL;
  }

  if (EVP_DecryptInit_ex(ctx->dec_ctx, EVP_aes_256_cbc(), NULL,
                         (const unsigned char *)ENCRYPTION_KEY, dummy_iv) != 1) {
    EVP_CIPHER_CTX_free(ctx->enc_ctx);
    EVP_CIPHER_CTX_free(ctx->dec_ctx);
    free(ctx);
    return NULL;
  }

  if (pthread_setspecific(thread_ctx_slot, ctx) != 0) {
    EVP_CIPHER_CTX_free(ctx->enc_ctx);
    EVP_CIPHER_CTX_free(ctx->dec_ctx);
    free(ctx);
    return NULL;
  }

  return ctx;
}

int aes_init(void)
{
  pthread_once(&thread_ctx_once, create_thread_ctx_slot);
  if (thread_ctx_init_failed) {
    return -1;
  }

  if (get_thread_cipher_ctx() == NULL) {
    return -1;
  }

  return 0;
}

void aes_cleanup(void)
{
  // Other threads' contexts are cleaned up by
  // their destructors, so they aren't handled here
  thread_cipher_ctx_t *ctx = pthread_getspecific(thread_ctx_slot);
  if (ctx != NULL) {
    destroy_thread_cipher_ctx(ctx);
    pthread_setspecific(thread_ctx_slot, NULL);
  }
}

int encode_base64(unsigned const char* input, char** buffer, int input_size)
{
  int encoded_size = ((input_size + 2) / 3) * 4;
  *buffer = malloc(encoded_size + 1);
  if (*buffer == NULL) {
    return -1;
  }

  EVP_EncodeBlock((unsigned char *)*buffer, input, input_size);

  return 0;
}

int decode_base64(char* base64_input, unsigned char** buffer)
{
  int len = strlen(base64_input);
  if (len == 0) {
    return -1;
  }

  int decoded_size = (len / 4) * 3;
  *buffer = (unsigned char*)malloc(decoded_size + 1);
  if (*buffer == NULL) {
    return -1;
  }

  int actual_len = EVP_DecodeBlock(*buffer, (unsigned char*)base64_input, len);
  if (actual_len < 0) {
    free(*buffer);
    *buffer = NULL;
    return -1;
  }

  // Subtract padding from return value
  if (len > 0 && base64_input[len - 1] == '=') actual_len--;
  if (len > 1 && base64_input[len - 2] == '=') actual_len--;

  (*buffer)[actual_len] = '\0';

  return actual_len;
}

char *encrypt_packet(json_t *json_input)
{
  thread_cipher_ctx_t *ctx = get_thread_cipher_ctx();
  if (ctx == NULL) {
    return NULL;
  }

  unsigned char iv[IV_LENGTH];
  if (RAND_bytes(iv, IV_LENGTH) != 1) {
    return NULL;
  }

  char *json_string = json_dumps(json_input, 0);
  if (json_string == NULL) {
    return NULL;
  }

  int input_len = strlen(json_string);
  int c_len = input_len + AES_BLOCK_SIZE, f_len = 0;

  unsigned char *output = malloc(IV_LENGTH + c_len);
  if (output == NULL) {
    free(json_string);
    return NULL;
  }

  memcpy(output, iv, IV_LENGTH);

  if (EVP_EncryptInit_ex(ctx->enc_ctx, NULL, NULL, NULL, iv) != 1) {
    free(json_string);
    free(output);
    return NULL;
  }

  if (EVP_EncryptUpdate(ctx->enc_ctx, output + IV_LENGTH, &c_len,
                        (unsigned char *)json_string, input_len) != 1) {
    free(json_string);
    free(output);
    return NULL;
  }

  free(json_string);

  if (EVP_EncryptFinal_ex(ctx->enc_ctx, output + IV_LENGTH + c_len, &f_len) != 1) {
    free(output);
    return NULL;
  }

  int total_len = IV_LENGTH + c_len + f_len;

  char *base64_buffer = NULL;
  if (encode_base64(output, &base64_buffer, total_len) != 0) {
    free(output);
    return NULL;
  }

  free(output);

  return base64_buffer;
}

int decrypt_packet(char *base64_input, json_t **json_output)
{
  thread_cipher_ctx_t *ctx = get_thread_cipher_ctx();
  if (ctx == NULL) {
    return -1;
  }

  unsigned char *decoded = NULL;
  int decoded_len = decode_base64(base64_input, &decoded);
  if (decoded_len < 0 || decoded == NULL) {
    return -1;
  }

  if (decoded_len < IV_LENGTH) {
    free(decoded);
    return -1;
  }

  unsigned char iv[IV_LENGTH];
  memcpy(iv, decoded, IV_LENGTH);

  unsigned char *ciphertext = decoded + IV_LENGTH;
  int cipher_len = decoded_len - IV_LENGTH;

  if (EVP_DecryptInit_ex(ctx->dec_ctx, NULL, NULL, NULL, iv) != 1) {
    free(decoded);
    return -1;
  }

  int p_len = cipher_len, f_len = 0;
  unsigned char *plaintext = malloc(p_len + 1);
  if (plaintext == NULL) {
    free(decoded);
    return -1;
  }

  if (EVP_DecryptUpdate(ctx->dec_ctx, plaintext, &p_len, ciphertext, cipher_len) != 1) {
    free(decoded);
    free(plaintext);
    return -1;
  }

  if (EVP_DecryptFinal_ex(ctx->dec_ctx, plaintext + p_len, &f_len) != 1) {
    free(decoded);
    free(plaintext);
    return -1;
  }

  free(decoded);

  int total_len = p_len + f_len;
  plaintext[total_len] = '\0';

  *json_output = json_loads((char *)plaintext, 0, NULL);

  free(plaintext);

  if (*json_output == NULL) {
    return -1;
  }

  return 0;
}
