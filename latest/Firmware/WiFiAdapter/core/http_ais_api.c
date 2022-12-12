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


static const char *TAG = "ais";

#define MAX_JSON_FIELDS 20
static json_t __jsonpool[MAX_JSON_FIELDS];
static char __outbuff[256];
static char __inbuff[256];


////////////////////////////////////////////////////////////////////////////////////////
// Handlers
////////////////////////////////////////////////////////////////////////////////////////
esp_err_t http_ais_get_handler(httpd_req_t *req)
{
  ais_station_t station;
  if ( !ais_config_read(&station) )
  {
    httpd_resp_set_status(req, "500");
    httpd_resp_send(req, NULL, 0);
    return ESP_FAIL;
  }

  char *p = __outbuff;
  size_t rem = sizeof __outbuff;

  p = json_objOpen(p, NULL, &rem);
  p = json_long(p, "mmsi", station.mmsi, &rem);
  p = json_str(p, "name", station.name, &rem);
  p = json_str(p, "callsign", station.callsign, &rem);
  p = json_int(p, "type", station.type, &rem);
  p = json_int(p, "len", station.len, &rem);
  p = json_int(p, "beam", station.beam, &rem);
  p = json_int(p, "portOffset", station.port_offs, &rem);
  p = json_int(p, "bowOffset", station.bow_offs, &rem);
  p = json_objClose(p, &rem);
  p = json_end(p, &rem);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, __outbuff, HTTPD_RESP_USE_STRLEN);
  ESP_LOGI(TAG, "GET %s %d 200", req->uri, strlen(__outbuff));

  return ESP_OK;
}

esp_err_t http_ais_post_handler(httpd_req_t *req)
{
  return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// URIs
////////////////////////////////////////////////////////////////////////////////////////
httpd_uri_t uri_ais_get = 
{
  .uri = "/api/ais",
  .method = HTTP_GET,
  .handler = http_ais_get_handler,
  .user_ctx = NULL
};

httpd_uri_t uri_ais_post = 
{
  .uri = "/api/ais",
  .method = HTTP_POST,
  .handler = http_ais_post_handler,
  .user_ctx = NULL
};


////////////////////////////////////////////////////////////////////////////////////////
// Registration
////////////////////////////////////////////////////////////////////////////////////////


void register_ais_api_handlers(httpd_handle_t handle)
{
  httpd_register_uri_handler(handle, &uri_ais_get);
  httpd_register_uri_handler(handle, &uri_ais_post);

}
