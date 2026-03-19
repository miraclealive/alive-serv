/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <jansson.h>

#define AES_BLOCK_SIZE 16
#define MAX_REQUEST_BODY_SIZE (1024 * 1024)
#define IV_LENGTH 16

int aes_init(void);
void aes_cleanup(void);

int encode_base64(unsigned const char* input, char** buffer, int input_size);
char *encrypt_packet(const json_t *json_input);
int decrypt_packet(const char *base64_input, size_t input_len, json_t **json_output);

int generate_token(char *buf, size_t len);

#endif // ENCRYPTION_H
