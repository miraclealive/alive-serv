/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#ifndef RESPONSE_H
#define RESPONSE_H

#include <jansson.h>
#include <ulfius.h>

struct response
{
  json_t *master_json;
  json_t *data_json;
};

struct response *response_new(struct _u_response *response);
int response_send(struct _u_response *response, struct response *br);
int response_error(struct _u_response *response, struct response *br, int code);

void return_code(struct _u_response *response, int code);

#endif // RESPONSE_H
