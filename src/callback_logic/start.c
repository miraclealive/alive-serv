/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include <ulfius.h>

#include "start.h"
#include "../encryption/encryption.h"
#include "../response_structs/base_response.h"

int callback_assethash(const struct _u_request *request, struct _u_response *response, void *user_data) 
{
  struct base_response *br = base_response_new(response);

  if (br == NULL) {
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }
  
  if (json_object_set_new(br->data_json, "asset_hash", json_string("a582d735ccff596433e66ea520dcc260")) != 0) {
    json_delete(br->master_json);
    json_delete(br->data_json);

    free(br);

    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }
  
  char *encrypt_buffer = NULL;
  encrypt_buffer = encrypt_packet(br->master_json);

  ulfius_set_string_body_response(response, 200, encrypt_buffer);
  
  free(encrypt_buffer);

  json_delete(br->master_json);
  json_delete(br->data_json);

  free(br);

  return U_CALLBACK_CONTINUE;
}
