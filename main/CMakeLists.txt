idf_component_register(SRCS 


                "packers_config.c"
                "weather_config.c"
                "DS3231_config.c"
                "f1_config.c"
                "orbit_date_time_config.c"
                "task_config.c"
                "display_config.c"
                "portal_config.c"
                "dns_server.c"
                "network_config.c"
                "settings_config.c"
                "buzzer_config.c"            
                "nvs_config.c" 
                "orbit_config.c" 
                "main.c"
                INCLUDE_DIRS "")
spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)
