/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <ulfius.h>

#include "core/database.h"

#include "game/debug.h"
#include "game/start.h"
#include "game/login.h"
#include "encryption/encryption.h"

int main(const int argc, char *argv[])
{
  if (argc < 7) {
    printf("Usage: %s <server_port> <db_host> <db_port> <db_username> <db_password> <db_name>\n", argv[0]);
    return 1;
  }

  db_set_config(argv[2], atoi(argv[3]), argv[4], argv[5], argv[6]);

  struct _u_instance instance;

  // Initialize instance with the port number
  if (ulfius_init_instance(&instance, atoi(argv[1]), NULL, NULL) != U_OK) {
    fprintf(stderr, "Error with ulfius_init_instance, aborting\n");
    return 1;
  }

  // Endpoint list declaration

  // Add callbacks to game endpoints
  ulfius_add_endpoint_by_val(&instance, "POST", "/api/start/assetHash",
                             NULL, 0, &callback_assethash, NULL);
  ulfius_add_endpoint_by_val(&instance, "POST", "/api/dummy/login",
                             NULL, 0, &callback_login, NULL);
  ulfius_add_endpoint_by_val(&instance, "POST", "/api/start",
                             NULL, 0, &callback_start, NULL);
  ulfius_add_endpoint_by_val(&instance, "POST", "/api/debug/error",
                         NULL, 0, &callback_error, NULL);


  MYSQL *init_conn;
  if (db_create_connection(&init_conn) != 0) {
    printf("Error creating connection to database\n");
    ulfius_clean_instance(&instance);
    return 1;
  }

  if (db_init(&init_conn) != 0) {
    printf("Error initializing database\n");
    mysql_close(init_conn);
    ulfius_clean_instance(&instance);
    return 1;
  }
  mysql_close(init_conn);

  if (db_init_pool() != 0) {
    printf("Error initializing database connection pool\n");
    ulfius_clean_instance(&instance);
    return 1;
  }

  if (aes_init() != 0) {
    printf("Error initializing AES context\n");
    db_destroy_pool();
    ulfius_clean_instance(&instance);
    return 1;
  }

  // Start the framework
  if (ulfius_start_framework(&instance) == U_OK) {
    printf("Starting server on port %d\n", instance.port);

    // Wait for the user to press <enter> on the console to quit the application
    getchar();
  } else {
    fprintf(stderr, "Error starting server\n");
  }
  printf("Ending server\n");

  ulfius_stop_framework(&instance);
  aes_cleanup();
  db_destroy_pool();
  ulfius_clean_instance(&instance);

  return 0;
}
