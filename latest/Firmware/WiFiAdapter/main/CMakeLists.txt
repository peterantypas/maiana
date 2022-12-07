idf_component_register(SRCS "MaianaWiFiAdapter.c"
                    INCLUDE_DIRS 
                      .
                      ../bsp
                      ../core
                    REQUIRES 
                      app_update
                      nvs_flash
                      esp_http_server                      
                    )
