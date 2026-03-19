/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#include "response.h"
#include "../encryption/encryption.h"

struct response *response_new(struct _u_response *response)
{
  struct response *br = malloc(sizeof(struct response));
  if (br == NULL) {
    return NULL;
  }

  json_t *master_json = json_object();
  if (master_json == NULL) {
    free(br);
    return NULL;
  }

  json_t *data_json = json_object();
  if (data_json == NULL) {
    json_decref(master_json);
    free(br);
    return NULL;
  }

  if (json_object_set_new(master_json, "code", json_integer(0)) != 0 ||
      json_object_set_new(master_json, "server_time", json_integer((unsigned long)time(NULL))) != 0) {
    json_decref(data_json);
    json_decref(master_json);
    free(br);
    return NULL;
  }

  if (json_object_set_new(master_json, "data", data_json) != 0) {
    json_decref(data_json);
    json_decref(master_json);
    free(br);
    return NULL;
  }

  ulfius_add_header_to_response(response, "Content-Type", "application/json");

  br->master_json = master_json;
  br->data_json = data_json;

  return br;
}

void return_code(struct _u_response *response, const int code)
{
  char code_str[16];
  snprintf(code_str, sizeof(code_str), "Error %d", code);
  ulfius_set_string_body_response(response, code, code_str);
}

int response_send(struct _u_response *response, struct response *br)
{
  char *encrypt_buffer = encrypt_packet(br->master_json);
  if (encrypt_buffer == NULL) {
    json_decref(br->master_json);
    free(br);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  ulfius_set_string_body_response(response, 200, encrypt_buffer);

  free(encrypt_buffer);
  json_decref(br->master_json);
  free(br);

  return U_CALLBACK_CONTINUE;
}

int response_error(struct _u_response *response, struct response *br, int code)
{
  json_decref(br->master_json);
  free(br);
  return_code(response, code);
  return U_CALLBACK_CONTINUE;
}
