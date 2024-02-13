/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#include "base_response.h"

struct base_response *base_response_new(struct _u_response *response)
{
  struct base_response *br = malloc(sizeof(struct base_response));
  json_t *master_json = json_object();
  json_t *data_json = json_object();

  if (json_object_set_new(master_json, "code", json_integer(0)) != 0 
    || json_object_set_new(master_json, "server_time", json_integer((unsigned long)time(NULL))) != 0 
    || json_object_set_new(master_json, "data", data_json) != 0) {
    json_delete(master_json);
    json_delete(data_json);

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
  char *code_str = malloc(sizeof(char) * 10);
  sprintf(code_str, "%d", code);
  ulfius_set_string_body_response(response, code, code_str);
  free(code_str);
}
