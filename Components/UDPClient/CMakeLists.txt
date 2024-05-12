idf_component_register(SRCS "udp_task.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_netif esp_wifi nvs_flash)
