#include <jansson.h>
#include <ulfius.h>

struct base_response
{
  json_t *master_json;
  json_t *data_json;
};

struct base_response *base_response_new(struct _u_response *response);
