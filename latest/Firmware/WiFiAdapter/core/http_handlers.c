#include <stdlib.h>
#include <esp_http_server.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "configuration.h"

extern const char index_html[] asm("index_html");
extern const uint8_t maiana_logo_jpg[] asm("maiana_logo_jpg");
extern const uint32_t maiana_logo_jpg_len asm("maiana_logo_jpg_length");

static const char *TAG = "httpd";

/* Forward declarations */
esp_err_t http_img_handler(httpd_req_t *req);

void send_std_headers(httpd_req_t *req)
{
  httpd_resp_set_hdr(req, "Connection", "Close");
  httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
}

esp_err_t http_root_handler(httpd_req_t *req)
{
  ESP_LOGI(TAG, "Request: %s", req->uri);
  if ( strstr(req->uri, "/images") )
  {
    return http_img_handler(req);
  }
  else if ( strcmp(req->uri, "/") == 0 || strcmp(req->uri, "/index.html") == 0 )
  {
    send_std_headers(req);
    esp_err_t err = httpd_resp_send(req, index_html, HTTPD_RESP_USE_STRLEN);
    ESP_LOGI(TAG, "GET %s %d %d", req->uri, strlen(index_html), err);
  }
  else if ( strcmp(req->uri, "/favicon.ico") == 0 )
  {
    httpd_resp_send_404(req);
  }
  return ESP_OK;
}

esp_err_t http_img_handler(httpd_req_t *req)
{
  ESP_LOGI(TAG, "Request: %s", req->uri);
  if ( strstr(req->uri, "maiana-logo.jpg") )
  {
    send_std_headers(req);
    httpd_resp_set_hdr(req, "Content-Type", "image/jpeg");
    esp_err_t err = httpd_resp_send(req, (const char *)maiana_logo_jpg, maiana_logo_jpg_len);
    ESP_LOGI(TAG, "GET %s %d %d", req->uri, maiana_logo_jpg_len, err);
  }
  // TODO: Add more image handlers here
  else 
  {
    httpd_resp_send_404(req);
  }
  return ESP_OK;
}


/** URI definitions */

httpd_uri_t uri_root = 
{
  .uri = "*",
  .method = HTTP_GET,
  .handler = http_root_handler,
  .user_ctx = NULL
};


/** Registration */
void register_http_handlers(httpd_handle_t handle)
{
  httpd_register_uri_handler(handle, &uri_root);
}
