set (SOURCES bsp_1_0.c)
idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS .
                       REQUIRES nvs_flash)
                       