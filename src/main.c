/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <ulfius.h>
#include <jansson.h>

#include "core/static_file_callback.h"

#include "callback_logic/start.h"

#define PORT 8080 // FIXME: hardcoded

int main() 
{
  struct _u_instance instance;
  struct _static_file_config config;

  // Initialize instance with the port number
  if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
    fprintf(stderr, "Error with ulfius_init_instance, aborting\n");
    return 1;
  }

  // Set up config object for static file serving
  config.files_path = "static";
  config.redirect_on_404 = NULL;
  config.mime_types = NULL;

  // Endpoint list declaration

  // Add static file callback function to maintenance endpoint
  ulfius_add_endpoint_by_val(&instance, "GET", "/maintenance/maintenance.json", 
                             NULL, 0, &callback_static_file, &config);

  // Add callbacks to game endpoints
  ulfius_add_endpoint_by_val(&instance, "POST", "/api/start/assetHash",
                             NULL, 0, &callback_assethash, NULL);

  // Start the framework
  if (ulfius_start_framework(&instance) == U_OK) {
    printf("Starting server on port %d\n", instance.port);

    // Wait for the user to press <enter> on the console to quit the application
    getchar();
  } else {
    fprintf(stderr, "Error starting server\n");
  }
  printf("Ending server\n");

  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);

  return 0;
}
