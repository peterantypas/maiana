#include <stdlib.h>
#include <esp_http_server.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "configuration.h"
#include "json-maker.h"
#include "tiny-json.h"
#include "http_utils.h"


static const char *TAG = "httpd";
static char __outbuff[256];
static char __inbuff[256];

#define MAX_JSON_FIELDS 20
static json_t __jsonpool[MAX_JSON_FIELDS];


////////////////////////////////////////////////////////////////////////////////////////
// Handlers
////////////////////////////////////////////////////////////////////////////////////////

esp_err_t http_nmea_get_handler(httpd_req_t *req)
{
  nmea_gateway_mode_t mode = config_get_nmea_gateway_mode();
  const char *ip = config_get_nmea_gateway_ip();
  uint16_t port = config_get_nmea_gateway_port();

    char *p = __outbuff;
  size_t rem = sizeof __outbuff;

  p = json_objOpen(p, NULL, &rem);
  p = json_int(p, "mode", mode, &rem); 
  p = json_str(p, "ip", ip, &rem);
  p = json_int(p, "port", port, &rem);
  p = json_objClose(p, &rem);
  p = json_end(p, &rem);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, __outbuff, HTTPD_RESP_USE_STRLEN);
  ESP_LOGI(TAG, "GET %s %d 200", req->uri, strlen(__outbuff));

  return ESP_OK;
}

esp_err_t http_nmea_post_handler(httpd_req_t *req)
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

  nmea_gateway_mode_t mode = NMEA_TCP_LISTENER;
  json_t const *modeprop = json_getProperty(doc, "mode");
  if ( modeprop && modeprop->type != JSON_NULL )
    mode = (nmea_gateway_mode_t)json_getInteger(modeprop);

  const char *ip = json_getPropertyValue(doc, "ip");
  json_t const *portprop = json_getProperty(doc, "port");
  uint16_t port = json_getInteger(portprop);

  config_nmea_gateway(mode, ip, port);
  httpd_resp_send(req, NULL, 0);

  return ESP_OK;
}


////////////////////////////////////////////////////////////////////////////////////////
// URIs
////////////////////////////////////////////////////////////////////////////////////////
httpd_uri_t uri_nmea_get = 
{
  .uri = "/api/nmea",
  .method = HTTP_GET,
  .handler = http_nmea_get_handler,
  .user_ctx = NULL
};

httpd_uri_t uri_nmea_post = 
{
  .uri = "/api/nmea",
  .method = HTTP_POST,
  .handler = http_nmea_post_handler,
  .user_ctx = NULL
};


////////////////////////////////////////////////////////////////////////////////////////
// Registration
////////////////////////////////////////////////////////////////////////////////////////

void register_nmea_api_handlers(httpd_handle_t handle)
{
  httpd_register_uri_handler(handle, &uri_nmea_get);
  httpd_register_uri_handler(handle, &uri_nmea_post);

}

