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
  int ret = read_http_body(req, __inbuff, sizeof __inbuff);
  if (ret <= 0)
  {  /* 0 return value indicates connection closed */
    /* Check if timeout occurred */
    if (ret == HTTPD_SOCK_ERR_TIMEOUT)
    {
      /* In case of timeout one can choose to retry calling                                                      
        * httpd_req_recv(), but to keep it simple, here we                                                       
        * respond with an HTTP 408 (Request Timeout) error */
      httpd_resp_send_408(req);
    }
    /* In case of error, returning ESP_FAIL will                                                                 
      * ensure that the underlying socket is closed */
    return ESP_FAIL;
  }
  __inbuff[ret] = 0;
  ESP_LOGI(TAG, "%s", __inbuff);

  json_t const *doc = json_create(__inbuff, __jsonpool, MAX_JSON_FIELDS);
  if ( doc == NULL )
  {
    ESP_LOGE(TAG, "Unable to parse JSON");
    return ESP_FAIL;
  }

  ais_station_t station;
  json_t const *mmsi = json_getProperty(doc, "mmsi");
  station.mmsi = json_getInteger(mmsi);

  const char *name = json_getPropertyValue(doc, "name");
  strncpy(station.name, name, sizeof station.name);

  const char *callsign = json_getPropertyValue(doc, "callsign");
  strncpy(station.callsign, callsign, sizeof station.callsign);

  json_t const *type = json_getProperty(doc, "type");
  station.type = json_getInteger(type);

  json_t const *len = json_getProperty(doc, "len");
  station.len = json_getInteger(len);

  json_t const *beam = json_getProperty(doc, "beam");
  station.beam = json_getInteger(beam);
  
  json_t const *portOffset = json_getProperty(doc, "portOffset");
  station.port_offs = json_getInteger(portOffset);
  
  json_t const *bowOffset = json_getProperty(doc, "bowOffset");
  station.bow_offs = json_getInteger(bowOffset);

  if ( ais_config_write(&station) )
  {
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
  }
  else
  {
    httpd_resp_set_status(req, "500");
    httpd_resp_send(req, NULL, 0);
    return ESP_FAIL;
  }

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
