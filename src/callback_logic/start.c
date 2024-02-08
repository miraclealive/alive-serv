#include <string.h>
#include <ulfius.h>

#include "start.h"
#include "../encryption/encryption.h"
#include "../response_structs/base_response.h"

int callback_assethash(const struct _u_request *request, struct _u_response *response, void *user_data) 
{
  struct base_response *br = base_response_new(response);

  if (br == NULL) {
    goto internal_server_error;
  }
  
  if (json_object_set_new(br->data_json, "asset_hash", json_string("a582d735ccff596433e66ea520dcc260")) != 0) {
    free(br);
    goto internal_server_error;
  }
  if (json_object_set_new(br->master_json, "data", br->data_json) != 0) {
    free(br);
    goto internal_server_error;
  }

  ulfius_set_string_body_response(response, 200, encrypt_packet(br->master_json));
  free(br);
  return U_CALLBACK_CONTINUE;

  internal_server_error:
    ulfius_set_string_body_response(response, 500, "Internal Server Error");
    return U_CALLBACK_CONTINUE;
}
