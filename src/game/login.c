/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include <ulfius.h>

#include "login.h"
#include "../encryption/encryption.h"
#include "../core/database.h"
#include "../response/response.h"

int callback_login(const struct _u_request *request, struct _u_response *response, void *user_data)
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

  MYSQL *conn = db_get_connection();
  if (conn == NULL) {
    json_decref(request_json);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  const json_t *uuid_json = json_object_get(request_json, "uuid");
  const json_t *twxuid_json = json_object_get(request_json, "twxuid");

  if (!json_is_string(uuid_json) || !json_is_string(twxuid_json)) {
    db_free_connection(conn);
    json_decref(request_json);
    return_code(response, 400);
    return U_CALLBACK_CONTINUE;
  }

  const char *uuid = json_string_value(uuid_json);
  const char *twxuid = json_string_value(twxuid_json);

  char *uuid_escaped = db_escape_string(conn, uuid);
  char *twxuid_escaped = db_escape_string(conn, twxuid);

  if (uuid_escaped == NULL || twxuid_escaped == NULL) {
    free(uuid_escaped);
    free(twxuid_escaped);
    db_free_connection(conn);
    json_decref(request_json);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  const char *sql_query_base = "SELECT user_id FROM alive_players WHERE uuid = \"%s\" AND twxuid = \"%s\"";
  const size_t sql_query_len = strlen(sql_query_base) + strlen(uuid_escaped) + strlen(twxuid_escaped) + 1;

  char *sql_query = malloc(sql_query_len);
  if (sql_query == NULL) {
    free(uuid_escaped);
    free(twxuid_escaped);
    db_free_connection(conn);
    json_decref(request_json);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  snprintf(sql_query, sql_query_len, sql_query_base, uuid_escaped, twxuid_escaped);

  free(uuid_escaped);
  free(twxuid_escaped);

  if (mysql_query(conn, sql_query) != 0) {
    printf("MariaDB Error: %s\n", mysql_error(conn));
    db_free_connection(conn);
    free(sql_query);
    json_decref(request_json);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  MYSQL_RES *result = mysql_store_result(conn);
  if (result == NULL) {
    db_free_connection(conn);
    free(sql_query);
    json_decref(request_json);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  const MYSQL_ROW row = mysql_fetch_row(result);
  long long user_id;

  if (row == NULL || row[0] == NULL) {
    // New player, create using defaults
    mysql_free_result(result);
    free(sql_query);

    if (create_new_player(conn, uuid, twxuid, &user_id) != 0) {
      db_free_connection(conn);
      json_decref(request_json);
      return_code(response, 500);
      return U_CALLBACK_CONTINUE;
    }

    db_free_connection(conn);
    json_decref(request_json);
  } else {
    // Existing player, return user_id from DB
    user_id = atoll(row[0]);
    mysql_free_result(result);
    free(sql_query);
    db_free_connection(conn);
    json_decref(request_json);
  }

  struct response *br = response_new(response);

  if (br == NULL) {
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  if (json_object_set_new(br->data_json, "user_id", json_integer(user_id)) != 0) {
    return response_error(response, br, 500);
  }

  return response_send(response, br);
}
