#include "globalVar.h"



bool update_time_flag = 0;
bool update_fun_fact_flag = 0;
bool update_full_recycle_flag = 0;
bool update_weather_flag = 0;
bool update_f1_flag = 0;
bool update_packers_flag = 0;



void _rst_whole_sys_task(void *vpParam){

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000 * 60 * 15)); //restart system every 15 minutes if fetcing f1, weater, packers fails
        if(update_full_recycle_flag == 1)esp_restart();
    }
    

}