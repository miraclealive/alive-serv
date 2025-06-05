/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include <ulfius.h>

#include "start.h"
#include "../encryption/encryption.h"
#include "../core/database.h"
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

int callback_start(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  MYSQL *conn;
  if (create_connection(&conn) != 0) {
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  const char *user_id = u_map_get(request->map_header, "aoharu-user-id");

  char *sql_query = malloc(sizeof(char) * 1000);
  char *sql_query_base = "SELECT token FROM alive_players WHERE user_id = \"%s\"";

  char *user_id_escaped = malloc(strlen(user_id) * 2);

  mysql_escape_string(user_id_escaped, user_id, strlen(user_id));

  sprintf(sql_query, sql_query_base, user_id_escaped);

  free(user_id_escaped);

  if (mysql_query(conn, sql_query) != 0) {
    printf("MariaDB Error: %s\n", mysql_error(conn));
    mysql_close(conn);
    free(sql_query);

    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  MYSQL_RES *result = mysql_store_result(conn);
  MYSQL_ROW row = mysql_fetch_row(result);
  const int length = mysql_fetch_lengths(result)[0];

  char *token = malloc(length + 1);
  sprintf(token, "%s", row[0]);
  token[length] = '\0';

  mysql_free_result(result);
  free(sql_query);
  mysql_close(conn);

  struct base_response *br = base_response_new(response);

  if (br == NULL) {
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  if (json_object_set_new(br->data_json, "asset_hash", json_string("a582d735ccff596433e66ea520dcc260")) != 0 || json_object_set_new(br->data_json, "token", json_string(token)) != 0) {
    json_delete(br->master_json);
    json_delete(br->data_json);

    free(token);
    free(br);

    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  free(token);
  
  char *encrypt_buffer = NULL;
  encrypt_buffer = encrypt_packet(br->master_json);

  ulfius_set_string_body_response(response, 200, encrypt_buffer);
  
  free(encrypt_buffer);

  json_delete(br->master_json);
  json_delete(br->data_json);

  free(br);
 
  return U_CALLBACK_CONTINUE;
}
