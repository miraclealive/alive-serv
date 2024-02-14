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

void set_db_config(char *host, int port, char *username, char *password, char *db_name);

int create_connection(MYSQL **conn);
int init_database(MYSQL **conn);

#endif // DATABASE_H
