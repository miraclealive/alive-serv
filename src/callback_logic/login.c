/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#include <ulfius.h>

#include "login.h"
#include "../encryption/encryption.h"
#include "../core/database.h"
#include "../response_structs/base_response.h"

int callback_login(const struct _u_request *request, struct _u_response *response)
{
  char request_body[request->binary_body_length + 1];
  strncpy(request_body, request->binary_body, request->binary_body_length);       
  request_body[request->binary_body_length] = '\0';

  json_t *request_json = json_object();
  decrypt_packet(request_body, &request_json);
 
  MYSQL *conn;
  if (create_connection(&conn) != 0) {
    json_delete(request_json);

    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  json_t *uuid_json = json_object_get(request_json, "uuid");
  json_t *twxuid_json = json_object_get(request_json, "twxuid");

  if (!json_is_string(uuid_json) || !json_is_string(twxuid_json)) {
    mysql_close(conn);
    json_delete(uuid_json);
    json_delete(twxuid_json);

    json_delete(request_json);
    return_code(response, 400);
    return U_CALLBACK_CONTINUE;
  }

  const char *uuid = json_string_value(uuid_json);
  const char *twxuid = json_string_value(twxuid_json);

  char *sql_query = malloc(sizeof(char) * 1000);
  char *sql_query_base = "SELECT user_id FROM alive_players WHERE uuid = \"%s\" AND twxuid = \"%s\"";

  char *uuid_escaped = malloc(strlen(uuid) * 2);
  char *twxuid_escaped = malloc(strlen(twxuid) * 2);

  mysql_escape_string(uuid_escaped, uuid, strlen(uuid));
  mysql_escape_string(twxuid_escaped, twxuid, strlen(twxuid));

  sprintf(sql_query, sql_query_base, uuid_escaped, twxuid_escaped);

  free(uuid_escaped);
  free(twxuid_escaped);

  if (mysql_query(conn, sql_query) != 0) {
    printf("MariaDB Error: %s\n", mysql_error(conn));
    mysql_close(conn);
    free(sql_query);

    json_delete(uuid_json);
    json_delete(twxuid_json);

    json_delete(request_json);

    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  json_delete(uuid_json);
  json_delete(twxuid_json);

  json_delete(request_json);

  MYSQL_RES *result = mysql_store_result(conn);
  MYSQL_ROW row = mysql_fetch_row(result);

  long user_id = atol(row[0]);

  mysql_free_result(result);
  free(sql_query);
  mysql_close(conn);

  struct base_response *br = base_response_new(response);

  if (br == NULL) {
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  if (json_object_set_new(br->data_json, "user_id", json_integer(user_id)) != 0) {
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
