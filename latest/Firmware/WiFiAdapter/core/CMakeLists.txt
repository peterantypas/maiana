set (SOURCES 
  wifi.c
  configuration.c
  http_server.c
  http_web_handlers.c
  http_wifi_api.c
  http_ais_api.c
  http_nmea_api.c
  tiny-json.c
  json-maker.c
  http_utils.c
  button.c
  nmea_gateway.c
  types.c
  ais_config.c
  utils.c
  http_sys_api.c
  )
idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS .
                       REQUIRES 
                        nvs_flash
                        esp_http_server
                      )
                       