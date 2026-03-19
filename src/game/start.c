/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include <ulfius.h>

#include "start.h"
#include "../encryption/encryption.h"
#include "../core/database.h"
#include "../response/response.h"

static const char *ASSET_HASH = "dd7175e4bcdab476f38c33c7f34b5e4d";

int callback_assethash(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  struct response *br = response_new(response);

  if (br == NULL) {
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  if (json_object_set_new(br->data_json, "asset_hash", json_string(ASSET_HASH)) != 0) {
    return response_error(response, br, 500);
  }

  return response_send(response, br);
}

int callback_start(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  MYSQL *conn = db_get_connection();
  if (conn == NULL) {
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  const char *user_id = u_map_get(request->map_header, "aoharu-user-id");
  if (user_id == NULL) {
    db_free_connection(conn);
    return_code(response, 400);
    return U_CALLBACK_CONTINUE;
  }

  char *user_id_escaped = db_escape_string(conn, user_id);
  if (user_id_escaped == NULL) {
    db_free_connection(conn);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  const char *sql_query_base = "SELECT token FROM alive_players WHERE user_id = \"%s\"";
  const size_t sql_query_len = strlen(sql_query_base) + strlen(user_id_escaped) + 1;

  char *sql_query = malloc(sql_query_len);
  if (sql_query == NULL) {
    free(user_id_escaped);
    db_free_connection(conn);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  snprintf(sql_query, sql_query_len, sql_query_base, user_id_escaped);

  free(user_id_escaped);

  if (mysql_query(conn, sql_query) != 0) {
    printf("MariaDB Error: %s\n", mysql_error(conn));
    db_free_connection(conn);
    free(sql_query);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  MYSQL_RES *result = mysql_store_result(conn);
  if (result == NULL) {
    db_free_connection(conn);
    free(sql_query);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  const MYSQL_ROW row = mysql_fetch_row(result);
  if (row == NULL || row[0] == NULL) {
    mysql_free_result(result);
    db_free_connection(conn);
    free(sql_query);
    return_code(response, 404);
    return U_CALLBACK_CONTINUE;
  }

  const unsigned long *lengths = mysql_fetch_lengths(result);
  if (lengths == NULL) {
    mysql_free_result(result);
    db_free_connection(conn);
    free(sql_query);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  const size_t token_length = lengths[0];
  char *token = malloc(token_length + 1);
  if (token == NULL) {
    mysql_free_result(result);
    db_free_connection(conn);
    free(sql_query);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  memcpy(token, row[0], token_length);
  token[token_length] = '\0';

  mysql_free_result(result);
  free(sql_query);
  db_free_connection(conn);

  struct response *br = response_new(response);
  if (br == NULL) {
    free(token);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  if (json_object_set_new(br->data_json, "asset_hash", json_string(ASSET_HASH)) != 0 ||
      json_object_set_new(br->data_json, "token", json_string(token)) != 0) {
    free(token);
    return response_error(response, br, 500);
  }

  free(token);

  return response_send(response, br);
}
