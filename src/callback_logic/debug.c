/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include <ulfius.h>

#include "debug.h"
#include "../encryption/encryption.h"
#include "../response_structs/base_response.h"

int callback_error(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  char request_body[request->binary_body_length + 1];
  strncpy(request_body, request->binary_body, request->binary_body_length);
  request_body[request->binary_body_length] = '\0';

  json_t *request_json = json_object();
  decrypt_packet(request_body, &request_json);

  json_t *code_json = json_object_get(request_json, "code");

  if (!json_is_string(code_json)) {
    json_delete(code_json);

    json_delete(request_json);
    return_code(response, 400);
    return U_CALLBACK_CONTINUE;
  }

  const char *code = json_string_value(code_json);

  const char *user_id = u_map_get(request->map_header, "aoharu-user-id");
  printf("Client from user %s reported an error:\n%s\n", user_id, code);

  json_delete(code_json);

  struct base_response *br = base_response_new(response);

  if (br == NULL) {
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  char *encrypt_buffer = NULL;
  encrypt_buffer = encrypt_packet(br->master_json);

  ulfius_set_string_body_response(response, 200, encrypt_buffer);

  free(encrypt_buffer);

  json_delete(br->master_json);

  free(br);

  return U_CALLBACK_CONTINUE;
}
