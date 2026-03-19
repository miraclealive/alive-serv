/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#include <ulfius.h>

#include "debug.h"
#include "../encryption/encryption.h"
#include "../response/response.h"

int callback_error(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  json_t *request_json = NULL;
  const int err = decrypt_packet(request->binary_body, request->binary_body_length, &request_json);
  if (err == -1) {
    return_code(response, 400);
    return U_CALLBACK_CONTINUE;
  }
  if (err == -2) {
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  const json_t *code_json = json_object_get(request_json, "code");

  if (!json_is_string(code_json)) {
    json_decref(request_json);
    return_code(response, 400);
    return U_CALLBACK_CONTINUE;
  }

  const char *code = json_string_value(code_json);

  const char *user_id = u_map_get(request->map_header, "aoharu-user-id");
  printf("Client from user %s reported an error:\n%s\n",
         user_id != NULL ? user_id : "NULL", code);

  json_decref(request_json);

  struct response *br = response_new(response);
  if (br == NULL) {
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  return response_send(response, br);
}
