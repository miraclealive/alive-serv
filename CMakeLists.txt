# Copyright (c) Ramen2X
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.27)

project(
  alive-serv
  LANGUAGES C)

find_package(OpenSSL REQUIRED)

find_program(MARIADB_CONFIG mariadb_config REQUIRED)
execute_process(COMMAND ${MARIADB_CONFIG} --include
                OUTPUT_VARIABLE MARIADB_INCLUDE_DIRS
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${MARIADB_CONFIG} --libs
                OUTPUT_VARIABLE MARIADB_LIBRARIES
                OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REPLACE "-I" "" MARIADB_INCLUDE_DIRS "${MARIADB_INCLUDE_DIRS}")
separate_arguments(MARIADB_INCLUDE_DIRS)

option(DOWNLOAD_DEPENDENCIES "Download dependencies" ON)

if(DOWNLOAD_DEPENDENCIES)
  message(STATUS "Fetching dependencies...")
  include(FetchContent)

  FetchContent_Declare(
    jansson
    GIT_REPOSITORY "https://github.com/akheron/jansson.git"
    GIT_TAG "v2.15.0"
    UPDATE_DISCONNECTED TRUE
    EXCLUDE_FROM_ALL
    OVERRIDE_FIND_PACKAGE
  )
  set(JANSSON_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(JANSSON_EXAMPLES OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(jansson)
  
  if(NOT TARGET Jansson::Jansson)
    add_library(Jansson::Jansson ALIAS jansson)
  endif()

  FetchContent_Declare(
    orcania
    GIT_REPOSITORY "https://github.com/babelouest/orcania.git"
    GIT_TAG "v2.3.3"
    UPDATE_DISCONNECTED TRUE
    EXCLUDE_FROM_ALL
    OVERRIDE_FIND_PACKAGE
  )
  FetchContent_MakeAvailable(orcania)
  set(ORCANIA_VERSION_STRING "2.3.3")
  target_compile_options(orcania PRIVATE -Wno-error=discarded-qualifiers)

  FetchContent_Declare(
    yder
    GIT_REPOSITORY "https://github.com/babelouest/yder.git"
    GIT_TAG "v1.4.20"
    UPDATE_DISCONNECTED TRUE
    EXCLUDE_FROM_ALL
    OVERRIDE_FIND_PACKAGE
  )
  if(NOT UNIX)
    set(WITH_JOURNALD OFF CACHE BOOL "" FORCE)
  endif()
  FetchContent_MakeAvailable(yder)
  set(YDER_VERSION_STRING "1.4.20")

  FetchContent_Declare(
    ulfius
    GIT_REPOSITORY "https://github.com/babelouest/ulfius.git"
    GIT_TAG "v2.7.15"
    UPDATE_DISCONNECTED TRUE
    EXCLUDE_FROM_ALL
  )
  FetchContent_MakeAvailable(ulfius)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

add_executable(${PROJECT_NAME} 
              src/main.c
              src/core/database.c
              src/encryption/encryption.c
              src/response_structs/base_response.c
              src/callback_logic/start.c
              src/callback_logic/login.c
              src/callback_logic/debug.c
)

if(APPLE)
  set(MHD_INCLUDE_DIR /opt/homebrew/include)
  set(MHD_LIBRARY /opt/homebrew/lib/libmicrohttpd.dylib)
  target_link_directories(${PROJECT_NAME} PUBLIC /opt/homebrew/lib)
  target_include_directories(${PROJECT_NAME} PUBLIC /opt/homebrew/include)
else()
  set(MHD_INCLUDE_DIR /usr/include)
  set(MHD_LIBRARY /usr/lib/libmicrohttpd.so)
  target_link_directories(${PROJECT_NAME} PUBLIC /usr/local/lib /usr/lib)
  target_include_directories(${PROJECT_NAME} PUBLIC /usr/local/include /usr/include)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${MARIADB_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
                      ulfius
                      jansson
                      orcania
                      ${MARIADB_LIBRARIES}
                      OpenSSL::Crypto
                      m)
