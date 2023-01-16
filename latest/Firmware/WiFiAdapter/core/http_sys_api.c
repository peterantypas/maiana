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
#include "../bsp/bsp.h"
#include "esp_ota_ops.h"

static const char *TAG = "sys";

static char __outbuff[128];
static char __inbuff[1024];

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

/**
 * @brief This came from https://gitlab.com/kevinwolfe/esp32_template/-/blob/main/main/main.c
 * 
 * @param req 
 * @return esp_err_t 
 */
static esp_err_t http_sys_ota_handler( httpd_req_t *req )
{
  httpd_resp_set_status( req, HTTPD_500 );    // Assume failure
  
  int ret, remaining = req->content_len;
  //printf( "Receiving\n" );
  
  esp_ota_handle_t update_handle = 0 ;
  const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
  //const esp_partition_t *running          = esp_ota_get_running_partition();
  
  if ( update_partition == NULL )
  {
    //printf( "Uh oh, bad things\n" );
    goto return_failure;
  }

  //printf( "Writing partition: type %d, subtype %d, offset 0x%08x\n", update_partition-> type, update_partition->subtype, update_partition->address);
  //printf( "Running partition: type %d, subtype %d, offset 0x%08x\n", running->type,           running->subtype,          running->address);
  esp_err_t err = ESP_OK;
  err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
  if (err != ESP_OK)
  {
    //printf( "esp_ota_begin failed (%s)", esp_err_to_name(err));
    goto return_failure;
  }

  while ( remaining > 0 )
  {
    // Read the data for the request
    if ( ( ret = httpd_req_recv( req, __inbuff, MIN( remaining, sizeof( __inbuff ) ) ) ) <= 0 )
    {
      if ( ret == HTTPD_SOCK_ERR_TIMEOUT )
      {
        // Retry receiving if timeout occurred
        continue;
      }

      goto return_failure;
    }
    
    size_t bytes_read = ret;
    
    remaining -= bytes_read;
    err = esp_ota_write(update_handle, __inbuff, bytes_read);
    if (err != ESP_OK)
    {
      goto return_failure;
    }
  }

  //printf( "Receiving done\n" );

  // End response
  if ( esp_ota_end(update_handle) == ESP_OK && esp_ota_set_boot_partition(update_partition) == ESP_OK )
  {
    //printf( "OTA Success?!\n Rebooting\n" );
    //fflush( stdout );

    httpd_resp_set_status( req, HTTPD_200 );
    httpd_resp_send( req, NULL, 0 );
    
    vTaskDelay( 2000 / portTICK_RATE_MS);
    bsp_reboot();    
    return ESP_OK;
  }
  //printf( "OTA End failed (%s)!\n", esp_err_to_name(err));

return_failure:
  if ( update_handle )
  {
    esp_ota_abort(update_handle);
  }

  httpd_resp_set_status( req, HTTPD_500 );    // Assume failure
  httpd_resp_send( req, NULL, 0 );
  return ESP_FAIL;
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

httpd_uri_t uri_sys_ota = 
{
  .uri = "/api/ota",
  .method = HTTP_POST,
  .handler = http_sys_ota_handler,
  .user_ctx = NULL
};

////////////////////////////////////////////////////////////////////////////////////////
// Registration
////////////////////////////////////////////////////////////////////////////////////////
void register_sys_api_handlers(httpd_handle_t handle)
{
  httpd_register_uri_handler(handle, &uri_sys_get);
  httpd_register_uri_handler(handle, &uri_sys_ota);
}
