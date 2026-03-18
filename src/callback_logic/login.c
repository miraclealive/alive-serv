/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include <ulfius.h>

#include "login.h"
#include "../encryption/encryption.h"
#include "../core/database.h"
#include "../response_structs/base_response.h"

#define MAX_REQUEST_BODY_SIZE (1024 * 1024)

int callback_login(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  if (request->binary_body_length == 0 || request->binary_body_length > MAX_REQUEST_BODY_SIZE) {
    return_code(response, 400);
    return U_CALLBACK_CONTINUE;
  }

  char *request_body = malloc(request->binary_body_length + 1);
  if (request_body == NULL) {
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  memcpy(request_body, request->binary_body, request->binary_body_length);
  request_body[request->binary_body_length] = '\0';

  json_t *request_json = NULL;
  if (decrypt_packet(request_body, &request_json) != 0 || request_json == NULL) {
    free(request_body);
    return_code(response, 400);
    return U_CALLBACK_CONTINUE;
  }

  free(request_body);

  MYSQL *conn;
  if (create_connection(&conn) != 0) {
    json_decref(request_json);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  json_t *uuid_json = json_object_get(request_json, "uuid");
  json_t *twxuid_json = json_object_get(request_json, "twxuid");

  if (!json_is_string(uuid_json) || !json_is_string(twxuid_json)) {
    mysql_close(conn);
    json_decref(request_json);
    return_code(response, 400);
    return U_CALLBACK_CONTINUE;
  }

  const char *uuid = json_string_value(uuid_json);
  const char *twxuid = json_string_value(twxuid_json);

  size_t uuid_len = strlen(uuid);
  size_t twxuid_len = strlen(twxuid);

  char *uuid_escaped = malloc(uuid_len * 2 + 1);
  char *twxuid_escaped = malloc(twxuid_len * 2 + 1);

  if (uuid_escaped == NULL || twxuid_escaped == NULL) {
    free(uuid_escaped);
    free(twxuid_escaped);
    mysql_close(conn);
    json_decref(request_json);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  mysql_real_escape_string(conn, uuid_escaped, uuid, uuid_len);
  mysql_real_escape_string(conn, twxuid_escaped, twxuid, twxuid_len);

  const char *sql_query_base = "SELECT user_id FROM alive_players WHERE uuid = \"%s\" AND twxuid = \"%s\"";
  size_t sql_query_len = strlen(sql_query_base) + strlen(uuid_escaped) + strlen(twxuid_escaped) + 1;

  char *sql_query = malloc(sql_query_len);
  if (sql_query == NULL) {
    free(uuid_escaped);
    free(twxuid_escaped);
    mysql_close(conn);
    json_decref(request_json);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  snprintf(sql_query, sql_query_len, sql_query_base, uuid_escaped, twxuid_escaped);

  free(uuid_escaped);
  free(twxuid_escaped);

  if (mysql_query(conn, sql_query) != 0) {
    printf("MariaDB Error: %s\n", mysql_error(conn));
    mysql_close(conn);
    free(sql_query);
    json_decref(request_json);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  json_decref(request_json);

  MYSQL_RES *result = mysql_store_result(conn);
  if (result == NULL) {
    mysql_close(conn);
    free(sql_query);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  MYSQL_ROW row = mysql_fetch_row(result);
  if (row == NULL || row[0] == NULL) {
    mysql_free_result(result);
    mysql_close(conn);
    free(sql_query);
    return_code(response, 404);
    return U_CALLBACK_CONTINUE;
  }

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
    json_decref(br->master_json);
    free(br);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

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
