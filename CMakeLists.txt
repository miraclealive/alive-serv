# Copyright (c) Ramen2X
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.27)

project(
  alive-serv
  LANGUAGES C)

find_package(OpenSSL REQUIRED)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

add_executable(${PROJECT_NAME} 
              src/main.c
              src/core/static_file_callback.c
              src/core/database.c
              src/encryption/encryption.c
              src/response_structs/base_response.c
              src/callback_logic/start.c
              src/callback_logic/login.c
              src/callback_logic/debug.c
)

# This is a bit of a mess because I don't know how to write find<lib>.cmake files right now
# Any help on cleaning this up would be greatly appreciated!!

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_link_directories(${PROJECT_NAME}
                        PUBLIC /opt/homebrew/lib /opt/homebrew/lib/mariadb)
  target_include_directories(${PROJECT_NAME}
                        PUBLIC /opt/homebrew/include /opt/homebrew/include/mysql)
endif()

target_link_directories(${PROJECT_NAME}
                        PUBLIC /usr/local/lib 
                        /usr/local/lib/mariadb
                        /usr/lib)

target_include_directories(${PROJECT_NAME}
                        PUBLIC /usr/local/include
                        /usr/local/include/mariadb
                        /usr/include
                        /usr/include/mariadb)

target_link_libraries(${PROJECT_NAME}
                      ulfius
                      jansson
                      orcania
                      mariadb
                      OpenSSL::Crypto)
