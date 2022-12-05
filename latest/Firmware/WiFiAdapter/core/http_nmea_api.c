#include <stdlib.h>
#include <esp_http_server.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "configuration.h"

static const char *TAG = "httpd";

////////////////////////////////////////////////////////////////////////////////////////
// Handlers
////////////////////////////////////////////////////////////////////////////////////////

esp_err_t http_nmea_get_handler(httpd_req_t *req)
{
  return ESP_OK;
}

esp_err_t http_nmea_post_handler(httpd_req_t *req)
{
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

