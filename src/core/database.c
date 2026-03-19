/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <openssl/rand.h>

#include "database.h"
#include "../encryption/encryption.h"

struct database_config db_config;

static MYSQL *pool[DB_POOL_SIZE];
static int pool_available[DB_POOL_SIZE];
static pthread_mutex_t pool_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t pool_cond = PTHREAD_COND_INITIALIZER;

void db_set_config(char *host, const int port, char *username, char *password, char *db_name)
{
  db_config.db_host = host;
  db_config.db_port = port;
  db_config.db_username = username;
  db_config.db_password = password;
  db_config.db_name = db_name;
}

int db_create_connection(MYSQL **conn)
{
  *conn = mysql_init(NULL);

  if (*conn == NULL) {
    printf("MariaDB Error: mysql_init() failed\n");
    return 1;
  }

  if (mysql_real_connect(*conn, db_config.db_host, db_config.db_username,
                         db_config.db_password, db_config.db_name,
                         db_config.db_port, NULL, 0) == NULL) {
    printf("MariaDB Error: %s\n", mysql_error(*conn));
    mysql_close(*conn);
    *conn = NULL;
    return 1;
  }

  return 0;
}

int db_init_pool(void)
{
  for (int i = 0; i < DB_POOL_SIZE; i++) {
    if (db_create_connection(&pool[i]) != 0) {
      for (int j = 0; j < i; j++) {
        mysql_close(pool[j]);
        pool[j] = NULL;
      }
      return 1;
    }
    pool_available[i] = 1;
  }
  return 0;
}

void db_destroy_pool(void)
{
  pthread_mutex_lock(&pool_mutex);
  for (int i = 0; i < DB_POOL_SIZE; i++) {
    if (pool[i] != NULL) {
      mysql_close(pool[i]);
      pool[i] = NULL;
    }
    pool_available[i] = 0;
  }
  pthread_mutex_unlock(&pool_mutex);
}

MYSQL *db_get_connection(void)
{
  pthread_mutex_lock(&pool_mutex);

  while (1) {
    for (int i = 0; i < DB_POOL_SIZE; i++) {
      if (pool_available[i] && pool[i] != NULL) {
        if (mysql_ping(pool[i]) != 0) {
          mysql_close(pool[i]);
          if (db_create_connection(&pool[i]) != 0) {
            pool[i] = NULL;
            pool_available[i] = 0;
            continue;
          }
        }
        pool_available[i] = 0;
        pthread_mutex_unlock(&pool_mutex);
        return pool[i];
      }
    }
    pthread_cond_wait(&pool_cond, &pool_mutex);
  }
}

void db_free_connection(const MYSQL *conn)
{
  if (conn == NULL) return;

  pthread_mutex_lock(&pool_mutex);
  for (int i = 0; i < DB_POOL_SIZE; i++) {
    if (pool[i] == conn) {
      pool_available[i] = 1;
      pthread_cond_signal(&pool_cond);
      break;
    }
  }
  pthread_mutex_unlock(&pool_mutex);
}

int db_init(MYSQL **conn)
{
  // TODO: Find a better way to do this??
  if (mysql_query((*conn), "CREATE TABLE IF NOT EXISTS alive_players (uuid VARCHAR(36) NOT NULL, twxuid VARCHAR(72) NOT NULL, token VARCHAR(40) NOT NULL, user_id BIGINT NOT NULL, name VARCHAR(10) NOT NULL, comment VARCHAR(96) NOT NULL, exp MEDIUMINT NOT NULL, main_deck_slot TINYINT NOT NULL, favorite_master_card_id MEDIUMINT NOT NULL, favorite_card_evolve TINYINT NOT NULL, guest_smile_master_card MEDIUMINT NOT NULL, guest_cool_master_card MEDIUMINT NOT NULL, guest_pure_master_card MEDIUMINT NOT NULL, friend_request_disabled TINYINT NOT NULL, master_title_ids MEDIUMTEXT NOT NULL, profile_settings TINYTEXT NOT NULL, sif_user_id BIGINT NOT NULL, ss_user_id BIGINT NOT NULL, birthday TINYTEXT NOT NULL, last_login_time BIGINT NOT NULL, total_love_gems BIGINT NOT NULL, paid_love_gems BIGINT NOT NULL, free_love_gems BIGINT NOT NULL, stamina TINYINT NOT NULL, stamina_last_update BIGINT NOT NULL, tutorial_step TINYINT NOT NULL)") != 0 ||
      mysql_query((*conn), "CREATE TABLE IF NOT EXISTS alive_card_lists (user_id BIGINT NOT NULL, card_id BIGINT NOT NULL, master_card_id BIGINT NOT NULL, exp MEDIUMINT NOT NULL, skill_exp MEDIUMINT NOT NULL, evolve TINYTEXT NOT NULL, created_date_time BIGINT NOT NULL)") != 0 ||
      mysql_query((*conn), "CREATE TABLE IF NOT EXISTS alive_deck_lists (user_id BIGINT NOT NULL, slot TINYINT NOT NULL, leader_role TINYINT NOT NULL, main_card_ids MEDIUMTEXT NOT NULL)") != 0 ||
      mysql_query((*conn), "CREATE TABLE IF NOT EXISTS alive_character_lists (user_id BIGINT NOT NULL, master_character_id BIGINT NOT NULL, exp MEDIUMINT NOT NULL)") != 0 ||
      mysql_query((*conn), "CREATE TABLE IF NOT EXISTS alive_item_lists (user_id BIGINT NOT NULL, item_id BIGINT NOT NULL, master_item_id BIGINT NOT NULL, amount TINYINT NOT NULL, expire_date_time BIGINT NOT NULL)") != 0 ||
      mysql_query((*conn), "CREATE TABLE IF NOT EXISTS alive_point_lists (user_id BIGINT NOT NULL, type TINYINT NOT NULL, amount BIGINT NOT NULL)") != 0 ||
      mysql_query((*conn), "CREATE TABLE IF NOT EXISTS alive_story_lists (user_id BIGINT NOT NULL, master_story_id BIGINT NOT NULL, master_story_part_ids MEDIUMTEXT NOT NULL)") != 0 ||
      mysql_query((*conn), "CREATE TABLE IF NOT EXISTS alive_live_lists (user_id BIGINT NOT NULL, master_live_id BIGINT NOT NULL, level TINYINT NOT NULL, clear_count MEDIUMINT NOT NULL, high_score BIGINT NOT NULL, max_combo TINYINT NOT NULL, auto_enable TINYINT NOT NULL, updated_time BIGINT NOT NULL)") != 0 ||
      mysql_query((*conn), "CREATE TABLE IF NOT EXISTS alive_live_mission_lists (user_id BIGINT NOT NULL, master_live_id BIGINT NOT NULL, clear_master_live_mission_ids MEDIUMTEXT NOT NULL)") != 0 ||
      mysql_query((*conn), "CREATE TABLE IF NOT EXISTS alive_event_point_lists (user_id BIGINT NOT NULL, master_event_id BIGINT NOT NULL, type TINYINT NOT NULL, amount BIGINT NOT NULL, reward_status MEDIUMTEXT NOT NULL)") != 0 ||
      mysql_query((*conn), "CREATE TABLE IF NOT EXISTS alive_lottery_lists (user_id BIGINT NOT NULL, master_lottery_id BIGINT NOT NULL, master_lottery_price_number TINYINT NOT NULL, count TINYINT NOT NULL, daily_count TINYINT NOT NULL, last_count_date TINYTEXT NOT NULL)") != 0) {
    printf("MariaDB Error: %s\n", mysql_error((*conn)));
    return 1;
  }

  return 0;
}

char *db_escape_string(MYSQL *conn, const char *input)
{
  const size_t len = strlen(input);
  char *escaped = malloc(len * 2 + 1);
  if (escaped == NULL) {
    return NULL;
  }
  mysql_real_escape_string(conn, escaped, input, len);
  return escaped;
}

int create_new_player(MYSQL *conn, const char *uuid, const char *twxuid,
                         long long *user_id_out)
{
  char *uuid_escaped = db_escape_string(conn, uuid);
  char *twxuid_escaped = db_escape_string(conn, twxuid);

  if (uuid_escaped == NULL || twxuid_escaped == NULL) {
    free(uuid_escaped);
    free(twxuid_escaped);
    return -1;
  }

  char token[41];
  if (generate_token(token, 40) != 0) {
    free(uuid_escaped);
    free(twxuid_escaped);
    return -1;
  }

  // Generate user_id (loop is a collision check)
  long long user_id = 0;
  for (int attempts = 0; attempts < 15; attempts++) {
    unsigned int r;
    if (RAND_bytes((unsigned char *)&r, sizeof(r)) != 1) {
      free(uuid_escaped);
      free(twxuid_escaped);
      return -1;
    }
    user_id = (long long)time(NULL) * 1000000LL + (r % 1000000);

    char check_query[128];
    snprintf(check_query, sizeof(check_query),
             "SELECT 1 FROM alive_players WHERE user_id = %lld LIMIT 1", user_id);

    if (mysql_query(conn, check_query) != 0) {
      free(uuid_escaped);
      free(twxuid_escaped);
      return -1;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) {
      free(uuid_escaped);
      free(twxuid_escaped);
      return -1;
    }

    const int exists = mysql_num_rows(result) > 0;
    mysql_free_result(result);

    if (!exists) {
      break;
    }

    if (attempts == 14) {
      free(uuid_escaped);
      free(twxuid_escaped);
      return -1;
    }
  }

  const long long current_time = (long long)time(NULL);

  // Technically the name and description new player values here aren't
  // accurate to the original EN server, but they look bad, so I'm changing them
  const char *insert_base =
    "INSERT INTO alive_players ("
    "uuid, twxuid, token, user_id, name, comment, exp, main_deck_slot, "
    "favorite_master_card_id, favorite_card_evolve, guest_smile_master_card, "
    "guest_cool_master_card, guest_pure_master_card, friend_request_disabled, "
    "master_title_ids, profile_settings, sif_user_id, ss_user_id, birthday, "
    "last_login_time, total_love_gems, paid_love_gems, free_love_gems, "
    "stamina, stamina_last_update, tutorial_step"
    ") VALUES ("
    "\"%s\", \"%s\", \"%s\", %lld, \"Tutorial\", \"Learn and Play!\", "
    "0, 1, 0, 0, 0, 0, 0, 1, \"[0,0]\", \"[1,2,3,4,5,6,7]\", 0, 0, \"\", "
    "%lld, 0, 0, 0, 100, %lld, 0)";

  const size_t query_len = strlen(insert_base) + strlen(uuid_escaped) +
                     strlen(twxuid_escaped) + 41 + 100;
  char *insert_query = malloc(query_len);
  if (insert_query == NULL) {
    free(uuid_escaped);
    free(twxuid_escaped);
    return -1;
  }

  snprintf(insert_query, query_len, insert_base,
           uuid_escaped, twxuid_escaped, token, user_id,
           current_time, current_time);

  free(uuid_escaped);
  free(twxuid_escaped);

  if (mysql_query(conn, insert_query) != 0) {
    printf("MariaDB Error creating player: %s\n", mysql_error(conn));
    free(insert_query);
    return -1;
  }

  free(insert_query);

  const int slots[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 100};
  constexpr int num_slots = sizeof(slots) / sizeof(slots[0]);

  for (int i = 0; i < num_slots; i++) {
    char deck_query[256];
    snprintf(deck_query, sizeof(deck_query),
             "INSERT INTO alive_deck_lists (user_id, slot, leader_role, main_card_ids) "
             "VALUES (%lld, %d, 0, \"[0,0,0,0,0,0,0,0,0]\")",
             user_id, slots[i]);

    if (mysql_query(conn, deck_query) != 0) {
      printf("MariaDB Error creating deck slot %d: %s\n", slots[i], mysql_error(conn));
    }
  }

  *user_id_out = user_id;
  return 0;
}
