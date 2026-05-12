/**
 *
 * @author Yann Kabambi
 * @name   ORBIT
 * 
// THIS WAS A SEMI COPY OF 'ZAIRE SYSTEM' ANOTHER PROJECT I WORKED ON(MUCH MORE ADVANCED AND BETTER IN MY OPINION). 
// I COPIED THIS FILE AS A STARTING POINT FOR THE SETTINGS STRUCTURE AND FUNCTIONS, 
// AND THEN MODIFIED IT FOR THE ORBIT SYSTEM. 
// THERE MAY BE SOME UNUSED OR OUTDATED CODE IN THIS FILE, 
// BUT I LEFT IT IN FOR NOW AS A REFERENCE FOR ANY FUTURE SETTINGS I MAY WANT TO ADD.
 */

#include "globalVar.h"


orbit_f1_data_t g_f1_data = {0};

void app_main(void)
{
    
    orbit_err_t ret;
    // Initialize NVS -- ALWAYS FIRST
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if(ret != ORBIT_OK)esp_restart();


    if(orbit_config_init() != ORBIT_OK){printf("INIT Failed!\n");vTaskDelay(4000 / portTICK_PERIOD_MS);esp_restart();}


    xTaskCreate(_draw_page1_task, "_draw_page1_task",  10240, NULL, 7, NULL);
    xTaskCreate(_time_widget_task, "_time_widget_task", 4096, NULL, 7, NULL);
    xTaskCreate(_date_widget_task, "_date_widget_task", 4096, NULL, 7, NULL);
    xTaskCreate(_weather_widget_task, "_weather_widget_task", 4096, NULL, 7, NULL);
    xTaskCreate(_f1_widget_task, "_f1_widget_task", 4096, NULL, 7, NULL);
    xTaskCreate(_packers_widget_task, "_packers_widget_task", 4096, NULL, 7, NULL);
    xTaskCreate(_fun_fact_timer_task, "_fun_fact_timer_task", 4096, NULL, 7, NULL);


    // orbit_f1_data_t f1;
    // f1_fetch(&f1);
    
    _display_main_UI();

}





