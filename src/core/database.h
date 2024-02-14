/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <mysql.h>

struct database_config {
  char *db_host;
  char *db_username;
  char *db_password;
  char *db_name;
  int db_port;
};

struct database_config db_config;

int create_connection(MYSQL **conn);
int init_database(MYSQL **conn);

#endif // DATABASE_H
