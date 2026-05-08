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
    xTaskCreate(_clock_timer_task, "_clock_timer_task",  2048, NULL, 4, NULL);
    xTaskCreate(_fun_fact_timer_task, "_fun_fact_timer_task",  2048, NULL, 4, NULL);
    xTaskCreate(_full_recycle_timer_task, "_full_recycle_timer_task",  2048, NULL, 4, NULL);


    _display_main_UI();
    

}





