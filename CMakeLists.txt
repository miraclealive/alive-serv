cmake_minimum_required(VERSION 3.27)
project(alive-serv)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O2")

add_executable(alive-serv 
              src/main.c
              src/core/static_file_callback.c
              src/encryption/encryption.c
              src/callback_logic/start.c
)

target_include_directories(alive-serv PUBLIC /usr/local/include /opt/homebrew/include)
target_link_directories(alive-serv PUBLIC /usr/local/lib /opt/homebrew/lib)

target_link_libraries(alive-serv ulfius jansson orcania yder crypto)
