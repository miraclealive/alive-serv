#include <ulfius.h>
#include <jansson.h>
#include <string.h>
#include <time.h>

#include "start.h"
#include "../encryption/encryption.h"

int callback_assethash(const struct _u_request *request, struct _u_response *response, void *user_data) 
{
  json_t *json = json_object();
  json_t *sub_json = json_object();

  // FIXME: hardcoded base response json structure
  
  if (json_object_set_new(json, "code", json_integer(0)) != 0) {
    ulfius_set_string_body_response(response, 500, "Internal Server Error");
    return U_CALLBACK_CONTINUE;
  }
  if (json_object_set_new(sub_json, "asset_hash", json_string("a582d735ccff596433e66ea520dcc260")) != 0) {
    ulfius_set_string_body_response(response, 500, "Internal Server Error");
    return U_CALLBACK_CONTINUE;
  }
  if (json_object_set_new(json, "data", sub_json) != 0) {
    ulfius_set_string_body_response(response, 500, "Internal Server Error");
    return U_CALLBACK_CONTINUE;
  }
  if (json_object_set_new(json, "server_time", json_integer((unsigned long)time(NULL))) != 0) {
    ulfius_set_string_body_response(response, 500, "Internal Server Error");
    return U_CALLBACK_CONTINUE;
  }

  ulfius_add_header_to_response(response, "Content-Type", "application/json");
  ulfius_set_string_body_response(response, 200, encrypt_packet(json));

  return U_CALLBACK_CONTINUE;
}
