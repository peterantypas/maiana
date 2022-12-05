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
extern const char wifi_html[] asm("wifi_html");
extern const char ais_html[] asm("ais_html");
extern const char nmea_html[] asm("nmea_html");

static const char *TAG = "httpd";

/* Forward declarations */
esp_err_t http_img_handler(httpd_req_t *req);

void send_std_headers(httpd_req_t *req)
{
  httpd_resp_set_hdr(req, "Connection", "Close");
  httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
}

esp_err_t http_html_handler(httpd_req_t *req, const char *html)
{
  send_std_headers(req);
  esp_err_t err = httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
  ESP_LOGI(TAG, "GET %s %d %d", req->uri, strlen(html), err);
  return err;
}


esp_err_t http_root_handler(httpd_req_t *req)
{
  //ESP_LOGI(TAG, "Request: %s", req->uri);
  if ( strcmp(req->uri, "/") == 0 || strstr(req->uri, "index.html") )
  {
    return http_html_handler(req, index_html);
  }
  else 
  {
    httpd_resp_send_404(req);
  }
  return ESP_OK;
}

esp_err_t http_wifi_handler(httpd_req_t *req)
{
  return http_html_handler(req, wifi_html);
}

esp_err_t http_ais_handler(httpd_req_t *req)
{
  return http_html_handler(req, ais_html);
}

esp_err_t http_nmea_handler(httpd_req_t *req)
{
  return http_html_handler(req, nmea_html);
}

esp_err_t http_image_handler(httpd_req_t *req)
{
  //ESP_LOGI(TAG, "Request: %s", req->uri);
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




////////////////////////////////////////////////////////////////////////////////////////
// URIs
////////////////////////////////////////////////////////////////////////////////////////


httpd_uri_t uri_root1 = 
{
  .uri = "/",
  .method = HTTP_GET,
  .handler = http_root_handler,
  .user_ctx = NULL
};

httpd_uri_t uri_root2 = 
{
  .uri = "/index.html",
  .method = HTTP_GET,
  .handler = http_root_handler,
  .user_ctx = NULL
};

httpd_uri_t uri_images = 
{
  .uri = "/images/*",
  .method = HTTP_GET,
  .handler = http_image_handler,
  .user_ctx = NULL
};

httpd_uri_t uri_wifi = 
{
  .uri = "/wifi.html",
  .method = HTTP_GET,
  .handler = http_wifi_handler,
  .user_ctx = NULL
};

httpd_uri_t uri_ais = 
{
  .uri = "/ais.html",
  .method = HTTP_GET,
  .handler = http_ais_handler,
  .user_ctx = NULL
};

httpd_uri_t uri_nmea = 
{
  .uri = "/nmea.html",
  .method = HTTP_GET,
  .handler = http_nmea_handler,
  .user_ctx = NULL
};


////////////////////////////////////////////////////////////////////////////////////////
// Registration
////////////////////////////////////////////////////////////////////////////////////////

void register_http_web_handlers(httpd_handle_t handle)
{
  httpd_register_uri_handler(handle, &uri_root1);
  httpd_register_uri_handler(handle, &uri_root2);
  httpd_register_uri_handler(handle, &uri_images);
  httpd_register_uri_handler(handle, &uri_wifi);
  httpd_register_uri_handler(handle, &uri_ais);
  httpd_register_uri_handler(handle, &uri_nmea);
}
