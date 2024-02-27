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
    mysql_close((*conn));
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
  if (mysql_query((*conn), "CREATE TABLE IF NOT EXISTS alive_players (uuid VARCHAR(36) NOT NULL, twxuid VARCHAR(72) NOT NULL, token VARCHAR(40) NOT NULL, user_id BIGINT NOT NULL, name VARCHAR(10) NOT NULL, comment VARCHAR(96) NOT NULL, exp MEDIUMINT NOT NULL, main_deck_slot TINYINT NOT NULL, favorite_master_card_id MEDIUMINT NOT NULL, favorite_card_evolve TINYINT NOT NULL, guest_smile_master_card MEDIUMINT NOT NULL, guest_cool_master_card MEDIUMINT NOT NULL, guest_pure_master_card MEDIUMINT NOT NULL, friend_request_disabled TINYINT NOT NULL, master_title_ids TINYTEXT NOT NULL, profile_settings TINYTEXT NOT NULL, sif_user_id BIGINT NOT NULL, ss_user_id BIGINT NOT NULL, birthday TINYTEXT NOT NULL, last_login_time BIGINT NOT NULL)") != 0) {
    printf("MariaDB Error: %s\n", mysql_error((*conn)));
    mysql_close((*conn));
    return 1;
  }

  mysql_close((*conn));

  return 0; 
}
