/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <stdio.h>

#include "database.h"

struct database_config db_config;

void set_db_config(char *host, int port, char *username, char *password, char *db_name)
{
  db_config.db_host = host;
  db_config.db_port = port;
  db_config.db_username = username;
  db_config.db_password = password;
  db_config.db_name = db_name;
}

int create_connection(MYSQL **conn)
{
  (*conn) = mysql_init(NULL);

  if ((*conn) == NULL) {
    printf("MariaDB Error: %s\n", mysql_error(*conn));
    return 1;
  }
  
  if (mysql_real_connect((*conn), db_config.db_host, db_config.db_username,
                         db_config.db_password, db_config.db_name,
                         db_config.db_port, NULL, 0) == NULL) {
    printf("MariaDB Error: %s\n", mysql_error((*conn)));
    mysql_close((*conn));
    return 1; 
  }

  return 0;
}

int init_database(MYSQL **conn)
{
  if (mysql_query((*conn), "CREATE TABLE IF NOT EXISTS alive_players (uuid VARCHAR(64) NOT NULL, twxuid VARCHAR(64) NOT NULL, user_id BIGINT NOT NULL, user_name VARCHAR(10) NOT NULL)") != 0) {
    printf("MariaDB Error: %s\n", mysql_error((*conn)));
    mysql_close((*conn));
    return 1;
  }

  mysql_close((*conn));

  return 0; 
}
