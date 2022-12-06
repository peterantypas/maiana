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
static char __outbuff[2048];
static char __inbuff[2048];

#define MAX_JSON_FIELDS 20
static json_t __jsonpool[MAX_JSON_FIELDS];

////////////////////////////////////////////////////////////////////////////////////////
// Handlers
////////////////////////////////////////////////////////////////////////////////////////
esp_err_t http_wifi_get_handler(httpd_req_t *req)
{
  wifi_operation_mode_t mode = config_get_wifi_operation_mode();
  const char *ssid = config_get_ssid();
  const char *password = config_get_password();

  char *p = __outbuff;
  size_t rem = sizeof __outbuff;

  p = json_objOpen(p, NULL, &rem);
  p = json_int(p, "mode", mode, &rem); 
  p = json_str(p, "ssid", ssid, &rem);
  if ( password )
    p = json_str(p, "password", password, &rem);
  else
    p = json_null(p, "password", &rem);

  p = json_objClose(p, &rem);
  p = json_end(p, &rem);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, __outbuff, HTTPD_RESP_USE_STRLEN);
  ESP_LOGI(TAG, "GET %s %d 200", req->uri, strlen(__outbuff));

  return ESP_OK;
}

esp_err_t http_wifi_post_handler(httpd_req_t *req)
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

  int mode = -1;
  const char *ssid = NULL;
  const char *pwd = NULL;

  json_t const *modeprop = json_getProperty(doc, "mode");
  if ( modeprop && modeprop->type != JSON_NULL )
    mode = json_getInteger(modeprop);

  ssid = json_getPropertyValue(doc, "ssid");
  
  json_t const *pwdprop = json_getProperty(doc, "password");
  if ( pwdprop->type != JSON_NULL )
    pwd = json_getValue(pwdprop);

  wifi_operation_mode_t wifi_mode = (wifi_operation_mode_t)mode;
  if ( mode < 0 || ssid == NULL )
  {
    httpd_resp_set_status(req, "400");
    httpd_resp_send(req, NULL, 0);
    return ESP_FAIL;
  }

  if ( wifi_mode == WIFI_SECURE_AP && pwd == NULL )
  {
    httpd_resp_set_status(req, "400");
    httpd_resp_send(req, NULL, 0);
    return ESP_FAIL;
  }

  config_wifi(wifi_mode, ssid, pwd);
  httpd_resp_send(req, NULL, 0);

  return ESP_OK;
}


////////////////////////////////////////////////////////////////////////////////////////
// URIs
////////////////////////////////////////////////////////////////////////////////////////
httpd_uri_t uri_wifi_get = 
{
  .uri = "/api/wifi",
  .method = HTTP_GET,
  .handler = http_wifi_get_handler,
  .user_ctx = NULL
};

httpd_uri_t uri_wifi_post = 
{
  .uri = "/api/wifi",
  .method = HTTP_POST,
  .handler = http_wifi_post_handler,
  .user_ctx = NULL
};


////////////////////////////////////////////////////////////////////////////////////////
// Registration
////////////////////////////////////////////////////////////////////////////////////////

void register_wifi_api_handlers(httpd_handle_t handle)
{
  httpd_register_uri_handler(handle, &uri_wifi_get);
  httpd_register_uri_handler(handle, &uri_wifi_post);
}
