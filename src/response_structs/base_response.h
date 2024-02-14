/*
 * Copyright (c) Ramen2X
 * SPDX-License-Identifier: MIT
 */

#ifndef BASE_RESPONSE_H
#define BASE_RESPONSE_H

#include <jansson.h>
#include <ulfius.h>

struct base_response
{
  json_t *master_json;
  json_t *data_json;
};

struct base_response *base_response_new(struct _u_response *response);
void return_code(struct _u_response *response, int code);

#endif // BASE_RESPONSE_H
