/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <string.h>
#include <jansson.h>
#include <openssl/evp.h>

#define AES_BLOCK_SIZE 16
#define IV_LENGTH 16

int aes_init(void);
void aes_cleanup(void);

int encode_base64(unsigned const char* input, char** buffer, int input_size);
int decode_base64(char* base64_input, unsigned char** buffer);
char *encrypt_packet(json_t *json_input);
int decrypt_packet(char *base64_input, json_t **json_output);

#endif // ENCRYPTION_H
