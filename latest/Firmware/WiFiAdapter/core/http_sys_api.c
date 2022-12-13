#include <stdlib.h>
#include <esp_http_server.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include <time.h>
#include "configuration.h"
#include "ais_config.h"
#include "json-maker.h"
#include "tiny-json.h"
#include "http_utils.h"
#include "types.h"

static const char *TAG = "sys";

static char __outbuff[128];

////////////////////////////////////////////////////////////////////////////////////////
// Handlers
////////////////////////////////////////////////////////////////////////////////////////
esp_err_t http_sys_get_handler(httpd_req_t *req)
{
  ais_system_t data;
  if ( !ais_config_read_sys(&data) )
  {
    httpd_resp_set_status(req, "500");
    httpd_resp_send(req, NULL, 0);
    return ESP_FAIL;
  }

  char *p = __outbuff;
  size_t rem = sizeof __outbuff;

  p = json_objOpen(p, NULL, &rem);
  p = json_str(p, "hw", data.hw, &rem);
  p = json_str(p, "fw", data.fw, &rem);
  p = json_objClose(p, &rem);
  p = json_end(p, &rem);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, __outbuff, HTTPD_RESP_USE_STRLEN);
  ESP_LOGI(TAG, "GET %s %d 200", req->uri, strlen(__outbuff));

  return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// URIs
////////////////////////////////////////////////////////////////////////////////////////
httpd_uri_t uri_sys_get = 
{
  .uri = "/api/sys",
  .method = HTTP_GET,
  .handler = http_sys_get_handler,
  .user_ctx = NULL
};

////////////////////////////////////////////////////////////////////////////////////////
// Registration
////////////////////////////////////////////////////////////////////////////////////////
void register_sys_api_handlers(httpd_handle_t handle)
{
  httpd_register_uri_handler(handle, &uri_sys_get);
}
