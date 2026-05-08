/**
 * @brief Buzzer / sound system;
 *
 * This file controls all sound effects on the helmet using the buzzer.
 * It sets up PWM (LEDC) and plays different tones for feedback and UI events.
 *
 * Handles:
 *  - Buzzer initialization (PWM setup)
 *  - Basic beep function (_beep)
 *  - Startup / shutdown sounds
 *  - UI feedback sounds (single beep, double beep, etc.)
 *
 * Different sound patterns are used to give the helmet personality
 * and make interactions feel more alive (startup chime, pairing sounds, etc.)
 *
 * @note
 * All sounds are built using frequency + duration combos with delays.
 *
 * @warning
 * Blocking delays (vTaskDelay) are used, so avoid calling these in
 * time-critical or high-priority tasks.
 *
 * @author Yann Kabambi
 * @project ORBIT
 */

#include "globalVar.h"

orbit_err_t buzzer_init(void) {
    ledc_timer_config_t timer = {
        .speed_mode      = BUZZER_SPEED_MODE,
        .timer_num       = BUZZER_TIMER,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz         = 2000,
        .clk_cfg         = LEDC_AUTO_CLK
    };
    if (ledc_timer_config(&timer) != ESP_OK) return ORBIT_ERR;

    ledc_channel_config_t channel = {
        .speed_mode = BUZZER_SPEED_MODE,
        .channel    = BUZZER_CHANNEL,
        .timer_sel  = BUZZER_TIMER,
        .gpio_num   = BUZZER_PIN,
        .duty       = 0,
        .hpoint     = 0
    };
    if (ledc_channel_config(&channel) != ESP_OK) return ORBIT_ERR;

    return ORBIT_OK;
}


static void _beep(uint32_t frqz, uint32_t time){
    ledc_set_freq(BUZZER_SPEED_MODE, BUZZER_TIMER, frqz);
    ledc_set_duty(BUZZER_SPEED_MODE, BUZZER_CHANNEL, 512); // 50% duty
    ledc_update_duty(BUZZER_SPEED_MODE, BUZZER_CHANNEL);
    vTaskDelay(pdMS_TO_TICKS(time));
    ledc_set_duty(BUZZER_SPEED_MODE, BUZZER_CHANNEL, 0);   // off
    ledc_update_duty(BUZZER_SPEED_MODE, BUZZER_CHANNEL);
}

void error_beep(){
    _beep(900, 180);    // soft start
    vTaskDelay(pdMS_TO_TICKS(80));

    _beep(1300, 180);   // step up
    vTaskDelay(pdMS_TO_TICKS(80));

    _beep(700, 260);    // boop
    vTaskDelay(pdMS_TO_TICKS(80));

    _beep(2000, 500);   // final confirmation tone
}



static void play_startup_sound(void) {
    _beep(1319, 80);   // E6
    vTaskDelay(pdMS_TO_TICKS(20));
    
    _beep(1568, 80);   // G6
    vTaskDelay(pdMS_TO_TICKS(20));
    
    _beep(2093, 200);  // C7 high & held
    // Total ≈ 0.7 s on + gaps = 1.0–1.2 s (short & addictive)
}

static void startup_chime(void){
   // up
    _beep(600, 150);
    vTaskDelay(pdMS_TO_TICKS(30));
    _beep(900, 150);
    vTaskDelay(pdMS_TO_TICKS(30));
    // down
    _beep(750, 150);
    vTaskDelay(pdMS_TO_TICKS(30));
    _beep(500, 250);
}

static void shutdown_chime(void){

    _beep(800, 100);
    vTaskDelay(pdMS_TO_TICKS(20));

    _beep(650, 120);
    vTaskDelay(pdMS_TO_TICKS(20));

    _beep(500, 140);
    vTaskDelay(pdMS_TO_TICKS(20));

    _beep(300, 300);
}

void init_beep(void){
    startup_chime();
}

void shutdown_beep(void){
    shutdown_chime();
}

void single_beep(void){
    _beep(5000, 250);
  vTaskDelay(pdMS_TO_TICKS(50));
}

void double_beep(void){
     _beep(3000, 250);
  vTaskDelay(pdMS_TO_TICKS(50));
  _beep(5000, 250);
  vTaskDelay(pdMS_TO_TICKS(50));
}