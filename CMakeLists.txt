cmake_minimum_required(VERSION 3.27)

project(
  alive-serv
  LANGUAGES C)

find_package(Ulfius REQUIRED)
find_package(OpenSSL REQUIRED)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O2")

add_executable(${PROJECT_NAME} 
              src/main.c
              src/core/static_file_callback.c
              src/encryption/encryption.c
              src/response_structs/base_response.c
              src/callback_logic/start.c
)

target_link_libraries(${PROJECT_NAME} 
                      Ulfius::Ulfius 
                      OpenSSL::Crypto)
