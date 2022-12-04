#include "http_server.h"
#include <stdlib.h>
#include <esp_http_server.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "configuration.h"

static httpd_handle_t __handle = NULL;

static const char *TAG = "httpd";

extern void register_http_handlers(httpd_handle_t);

void start_httpd()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.uri_match_fn = httpd_uri_match_wildcard;
  config.max_uri_handlers = 20;
  //config.max_open_sockets = 1;

  if (httpd_start(&__handle, &config) == ESP_OK)
  {
    register_http_handlers(__handle);
    ESP_LOGI(TAG, "Started HTTPD");
  }
 }

void stop_httpd()
{
  if (__handle)
  {
    httpd_stop(__handle);
    __handle = NULL;
    ESP_LOGI(TAG, "Stopped HTTPD");
  }
}

bool is_httpd_running()
{
  return __handle != NULL;
}




