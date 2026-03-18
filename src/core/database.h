/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <mysql.h>

#define DB_POOL_SIZE 8

struct database_config {
  char *db_host;
  char *db_username;
  char *db_password;
  char *db_name;
  int db_port;
};

void db_set_config(char *host, int port, char *username, char *password, char *db_name);

int db_init_pool(void);
void db_destroy_pool(void);
MYSQL *db_get_connection(void);
void db_free_connection(MYSQL *conn);

// These are for internal use only, they
// should not be called by any callback logic!
int db_create_connection(MYSQL **conn);
int db_init(MYSQL **conn);

#endif // DATABASE_H
