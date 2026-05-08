#include "globalVar.h"



bool update_time_flag = 0;
bool update_fun_fact_flag = 0;
bool update_full_recycle_flag = 0;

void _clock_timer_task(void *vpParam){

    while (1)
    {
        if(update_time_flag == 0){
             
            update_time_flag = 1; //alert _draw_page1_task to update the clock on screen
        }

        vTaskDelay(pdMS_TO_TICKS(1000 * 60)); // wait 1 minute before allowing next update
    }
    

}

void _fun_fact_timer_task(void *vpParam){

     while (1)
    {
        if(update_fun_fact_flag == 0){
             
            update_fun_fact_flag = 1; //alert _draw_page1_task to update the fun fact on screen
        }
        vTaskDelay(pdMS_TO_TICKS(1000 * 60 )); // wait minutes before allowing next update
    }

}

void _full_recycle_timer_task(void *vpParam){
 
        while (1)
        {
            if(update_full_recycle_flag == 0){
                
                update_full_recycle_flag = 1; //alert _draw_page1_task to update the full recycle on screen
            }
            vTaskDelay(pdMS_TO_TICKS(1000 * 60 * 60)); // wait 1 hour before allowing next update
    }

}