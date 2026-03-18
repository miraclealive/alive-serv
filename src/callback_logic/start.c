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

  if (json_object_set_new(br->data_json, "asset_hash", json_string("dd7175e4bcdab476f38c33c7f34b5e4d")) != 0) {
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

  size_t user_id_len = strlen(user_id);

  char *user_id_escaped = malloc(user_id_len * 2 + 1);
  if (user_id_escaped == NULL) {
    db_free_connection(conn);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  mysql_real_escape_string(conn, user_id_escaped, user_id, user_id_len);

  const char *sql_query_base = "SELECT token FROM alive_players WHERE user_id = \"%s\"";
  size_t sql_query_len = strlen(sql_query_base) + strlen(user_id_escaped) + 1;

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

  MYSQL_ROW row = mysql_fetch_row(result);
  if (row == NULL || row[0] == NULL) {
    mysql_free_result(result);
    db_free_connection(conn);
    free(sql_query);
    return_code(response, 404);
    return U_CALLBACK_CONTINUE;
  }

  unsigned long *lengths = mysql_fetch_lengths(result);
  if (lengths == NULL) {
    mysql_free_result(result);
    db_free_connection(conn);
    free(sql_query);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  size_t token_length = lengths[0];
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

  struct base_response *br = base_response_new(response);
  if (br == NULL) {
    free(token);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  if (json_object_set_new(br->data_json, "asset_hash", json_string("dd7175e4bcdab476f38c33c7f34b5e4d")) != 0 ||
      json_object_set_new(br->data_json, "token", json_string(token)) != 0) {
    json_decref(br->master_json);
    free(token);
    free(br);
    return_code(response, 500);
    return U_CALLBACK_CONTINUE;
  }

  free(token);

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
