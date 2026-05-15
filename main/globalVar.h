#ifndef GLOBAL_VAR_H
#define GLOBAL_VAR_H



#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>

#include <nvs.h>
#include <nvs_flash.h>

#include <sys/unistd.h>
#include <sys/stat.h>

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <freertos/task.h>
#include <freertos/ringbuf.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>

#include <esp_log.h>
#include <esp_flash.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <esp_timer.h>
#include <esp_now.h>
#include <esp_mac.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_spiffs.h>
#include <esp_event.h>
#include <esp_vfs.h>
#include <esp_vfs_fat.h>
#include <esp_http_server.h>
#include <esp_netif_ip_addr.h>
#include <esp_system.h>
#include <esp_http_client.h>

#include <lwip/sockets.h>
#include <lwip/inet.h>

#include <driver/gpio.h>
#include <driver/i2c.h>
#include <driver/uart.h>
#include <driver/spi_master.h>
#include <driver/ledc.h>

#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_ili9341.h"

#include "esp_sntp.h"
#include "time.h"

#include "orbit_config.h"
#include "cJSON.h"

//BUZZER
#define BUZZER_PIN          GPIO_NUM_26
#define BUZZER_CHANNEL      LEDC_CHANNEL_0
#define BUZZER_TIMER        LEDC_TIMER_0
#define BUZZER_SPEED_MODE   LEDC_LOW_SPEED_MODE
orbit_err_t buzzer_init(void);
void init_beep(void);
void shutdown_beep(void);
void single_beep(void);
void double_beep(void);
void error_beep(void);


extern bool update_time_flag; // if true, clock will update time then turn off runs every second
extern bool update_fun_fact_flag; // if true, fun fact will update then turn off runs every 3 minute
extern bool update_full_recycle_flag; // if true, full recycle will update then turn off runs every hour
extern bool update_weather_flag; // if true, weather will update then turn off runs every 5 minutes
extern bool update_f1_flag; // if true, F1 will update then turn off runs every 5 minutes
extern bool update_packers_flag; // if true, packers will update then turn off runs every 5 minutes
void _draw_page1_task(void *vpParam);
void _time_widget_task(void *vpParam);
void _date_widget_task(void *vpParam);
void _weather_widget_task(void *vpParam);
void _f1_widget_task(void *vpParam);
void _packers_widget_task(void *vpParam);
void _fun_fact_timer_task(void *vpParam);
void _rst_whole_sys_task(void *vpParam);

#endif