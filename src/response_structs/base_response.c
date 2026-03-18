/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#include "base_response.h"

struct base_response *base_response_new(struct _u_response *response)
{
  struct base_response *br = malloc(sizeof(struct base_response));
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

void return_code(struct _u_response *response, int code)
{
  char code_str[16];
  snprintf(code_str, sizeof(code_str), "Error %d", code);
  ulfius_set_string_body_response(response, code, code_str);
}
