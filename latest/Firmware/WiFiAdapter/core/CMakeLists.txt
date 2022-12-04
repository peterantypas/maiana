set (SOURCES 
  wifi.c
  configuration.c
  http_server.c
  http_handlers.c
  )
idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS .
                       REQUIRES 
                        nvs_flash
                        esp_http_server
                      )
                       