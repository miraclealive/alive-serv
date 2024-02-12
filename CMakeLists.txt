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
              src/encryption/encryption.c
              src/response_structs/base_response.c
              src/callback_logic/start.c)


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_link_directories(${PROJECT_NAME}
                        PUBLIC /opt/homebrew/lib)
endif()

target_link_directories(${PROJECT_NAME}
                        PUBLIC /usr/local/lib)

target_link_libraries(${PROJECT_NAME}
                      ulfius
                      jansson
                      orcania
                      OpenSSL::Crypto)
