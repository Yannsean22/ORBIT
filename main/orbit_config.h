/**
 * @brief Main header for ORBIT
 *
 * This is the main "contract" for the whole project.
 * It exposes all the core definitions, configs, structs,
 * and function prototypes used across the project.
 *
 * Includes:
 *  - project init + core functions
 *  - Network functions
 *  - DNS / captive portal functions
 *  - NVS read/write helpers
 *  - Global project structs (prefs, state, device info)
 *
 *
 * Basically: if something is shared across files, it’s probably here.
 *
 * @note
 * Changing anything here can affect the entire project, so be careful.
 *
 * @warning
 * Keep this file organized -> it can get messy fast since everything depends on it.
 *
 * @author Yann Kabambi
 * @name   ORBIT
 */

#ifndef ORBIT_CONFIG_H
#define ORBIT_CONFIG_H


#include "globalVar.h"
#include "orbit_portal_settings_codex.h"
#include "orbit_settings_codex.h"

//initialize error types
typedef enum{
    ORBIT_OK = 0,
    ORBIT_ERR = !ORBIT_OK
} orbit_err_t;


// ======================
// 1. USER SETTINGS (stored in NVS, survives reboot)
// ======================
typedef struct {
    char user_name[64];      // "Alex", "ORBIT Rider", etc.
    char device_name[64];    // "ORBIT-VISION-001", "SnowBeast"
} user_prefs_t;

// ======================
// 2. SYSTEM SETTINGS (user preferences, stored in NVS)
// ======================
typedef struct {
    char device_lang[8];            // "en", "es", "fr", etc.
    uint8_t units;                    // 0 = metric (km/h, m) C* , 1 = imperial (mph, ft) F*
    uint8_t theme;                   // 0 = light, 1 = dark
    uint8_t f1;                      // 0 = mclaren, 1 = ferrari
} system_prefs_t;

// ======================
// 3 - DEVICE ENCODED INFO
// ======================
typedef struct {
    uint8_t first_boot;              // 0 = new device, 1 = already accepted
    char wifi_ssid[64];              // Last connected WiFi SSID
    char wifi_pass[64];              // Last connected WiFi password (encrypted in real implementation
    bool wifi_enabled;              // Is WiFi enabled or disabled
} sys_secure_info_t;

// ======================
// 4 - USER ENCODED SECURE INFO (encrypted in real implementation)
// ======================
typedef struct {
    char passcode[16];               // Onboarding + portal passcode
    char recovery_code[16];          // Code to recover/reset device (shown once on first boot)
} usr_secure_info_t;


// ======================
// 6. DEVICE ENCODED INFO
// ======================

typedef struct {
    /* =========================
       DEVICE IDENTITY
       ========================= */
    char manufacturer[16];        // "ORBIT SYSTEMS"
    char model_name[20];           // "ORBIT-HELMET" - "ORBIT VISION"
    char device_id[24];        // Unique per unit

    /* =========================
       HARDWARE
       ========================= */
    char hardware_revision[16];   // "1.0", "1.1", etc.

    /* =========================
       FIRMWARE
       ========================= */
    char firmware_version[16];     // "1.0.0"

    /* =========================
       BUILD INFO
       ========================= */
    char build_date[16];          // "2024-06-01"
    char build_type[16];          // "debug" or "release" or "fun"

} device_info_t;


// ======================
// Global instances
// ======================
extern user_prefs_t    g_user_prefs;
extern system_prefs_t  g_system_prefs;
extern device_info_t   g_device_info;
extern sys_secure_info_t g_sys_secure_info;
extern usr_secure_info_t g_usr_secure_info;

orbit_err_t orbit_config_init(void);
orbit_err_t _spiffs_init(void);


//======================
// I2C PINS
//======================
#define I2C_NUM         I2C_NUM_0
#define I2C_SDA_PIN     GPIO_NUM_21
#define I2C_SCL_PIN     GPIO_NUM_22
orbit_err_t _ds3231_init(void);
char* _ds3231_get_time(void);
char* _ds3231_get_date(void);
orbit_err_t _ds3231_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds);
orbit_err_t _ds3231_set_date(uint8_t day, uint8_t month, uint8_t year);
orbit_err_t ds3231_test_write_seconds(void);

//======================
// RBG LIGHT PINS
//======================
#define RED_LED_PIN     GPIO_NUM_4
#define GREEN_LED_PIN   GPIO_NUM_16
#define BLUE_LED_PIN    GPIO_NUM_17


//DNS SERVER - CAPTIVE PORTAL 
extern char dns_server_ssid[128];
extern char dns_server_bssid[128];
extern bool is_settings_portal_on;
#define DNS_PORT                53
#define DNS_TASK_STACK_SIZE     8192
#define DNS_TASK_PRIORITY       3
#define MAX_HTTP_RECV_BUFFER    512

#ifdef __cplusplus
extern "C" {
#endif
void start_dns_server(void);
void stop_dns_server(void);
esp_err_t handler(httpd_req_t *req);
void register_dns_catch_all(httpd_handle_t server);
#ifdef __cplusplus
}
#endif

//NVS
#define CFG_USR_NAMESPACE_KEY   "usr_cfg"   // User preferences (name, device name)
#define CFG_SYS_NAMESPACE_KEY   "sys_cfg"   // System settings (behavior)
#define SYS_INFO_NAMESPACE_KEY  "sys_info"  // System info (device ID, firmware version, hardware data)

#define NVS_KEY_USER_PREFS      "user_prefs"
#define NVS_KEY_SYSTEM_PREFS    "system_prefs"
#define NVS_KEY_USER_SECURE     "user_secure"
#define NVS_KEY_SYSTEM_SECURE   "system_secure"
#define NVS_KEY_DEVICE_INFO     "device_info"


extern nvs_handle_t usr_handle;
extern nvs_handle_t sys_handle;
extern nvs_handle_t info_handle;

orbit_err_t nvs_init(void);
orbit_err_t nvs_write_char(nvs_handle_t handle, const char *key, const char *value);
orbit_err_t nvs_write_blob(nvs_handle_t handle, const char *key, const void *value, size_t size);
orbit_err_t nvs_read_char(nvs_handle_t handle, const char *key, char *out, size_t size);
orbit_err_t nvs_read_blob(nvs_handle_t handle, const char *key, void *out, size_t size);

//portal settings

extern TaskHandle_t dns_task_handle;
extern httpd_handle_t web_server;
httpd_handle_t start_webserver(void);
esp_err_t main_page_get_handler(httpd_req_t *req);

orbit_err_t settings_update_system_info(const device_info_t *info);
orbit_err_t settings_update_first_boot(const char *first_boot_values);
orbit_err_t settings_update_user_name(const char *user_name);
orbit_err_t settings_update_device_name(const char *device_name);
orbit_err_t settings_update_device_lang(const char *device_lang);
orbit_err_t settings_update_device_units(const char *device_units);
orbit_err_t settings_update_device_theme(const char *device_theme);
orbit_err_t settings_update_device_f1(const char *device_f1);
orbit_err_t settings_update_passcode(const char *passcode);
orbit_err_t settings_update_recovery_code(const char *recovery_code);
orbit_err_t settings_update_wifi_security(const char *wifi_ssid, const char *wifi_pass);



orbit_err_t settings_get_first_boot(void);
const char *settings_get_user_name(void);
const char *settings_get_device_name(void);
const char *settings_get_device_lang(void);
uint8_t settings_get_device_units(void);
uint8_t settings_get_device_theme(void);
uint8_t settings_get_device_f1();

const char *settings_get_manufacturer(void);
const char *settings_get_model_name(void);
const char *settings_get_firmware_version(void);
const char *settings_get_build_date(void);
const char *settings_get_device_id(void);
const char *settings_get_hardware_rev(void);

orbit_err_t settings_check_passcode(const char *input_passcode);
orbit_err_t settings_get_wifi_security();


//NETWORK
#define NETWORK_MAX_CONN            9
#define NETWORK_SETTINGS_SSID       "ORBIT VIEW PORTAL"
extern char availableNetworks[5120];
extern bool wifi_connected;
orbit_err_t _network_init(void);
orbit_err_t _network_settings_mode();
orbit_err_t _network_connect_mode();
orbit_err_t _network_connect_to_wifi(const char* ssid, const char* password);



//=============== DISPLAY ====================

//DISLAY POSITIONS
#define DISPLAY_POSITION_TOP_LEFT_X_LV0                     LCD_WIDTH / 1.25
#define DISPLAY_POSITION_TOP_LEFT_X_LV1                     LCD_WIDTH / 1.35
#define DISPLAY_POSITION_TOP_LEFT_Y_LV0                     8
#define DISPLAY_POSITION_TOP_LEFT_Y_LV1                     24
#define DISPLAY_POSITION_TOP_MIDDLE_X_LV0                   LCD_WIDTH / 2
#define DISPLAY_POSITION_TOP_MIDDLE_Y_LV0                   8
#define DISPLAY_POSITION_TOP_MIDDLE_X_LV1                   LCD_WIDTH / 2
#define DISPLAY_POSITION_TOP_MIDDLE_Y_LV1                   39
#define DISPLAY_POSITION_TOP_RIGHT_X
#define DISPLAY_POSITION_TOP_RIGHT_Y

#define DISPLAY_POSITION_MIDDLE_LEFT_X                      230
#define DISPLAY_POSITION_MIDDLE_LEFT_Y                      56
#define DISPLAY_POSITION_MIDDLE_CENTER_X                    120
#define DISPLAY_POSITION_MIDDLE_CENTER_Y                    56
#define DISPLAY_POSITION_MIDDLE_RIGHT_X                     10
#define DISPLAY_POSITION_MIDDLE_RIGHT_Y                     56

#define DISPLAY_POSITION_SINGLE_POINT_X                     116 
#define DISPLAY_POSITION_SINGLE_POINT_Y                     156

#define DISPLAY_POSITION_SINGLE_POINT_PT2_X                 116 
#define DISPLAY_POSITION_SINGLE_POINT_PT2_Y                 LCD_HEIGHT / 2.5


#define DISPLAY_POSITION_BOTTOM_LEFT_X
#define DISPLAY_POSITION_BOTTOM_LEFT_Y                      188
#define DISPLAY_POSITION_BOTTOM_MIDDLE_X                    LCD_WIDTH / 2.5
#define DISPLAY_POSITION_BOTTOM_MIDDLE_Y                    195
#define DISPLAY_POSITION_BOTTOM_RIGHT_X                     
#define DISPLAY_POSITION_BOTTOM_RIGHT_Y                     188

//PIN DEF
#define PIN_NUM_MOSI    GPIO_NUM_13
#define PIN_NUM_MISO    GPIO_NUM_12
#define PIN_NUM_CLK     GPIO_NUM_14
#define PIN_NUM_CS      GPIO_NUM_15
#define PIN_NUM_DC      GPIO_NUM_2
#define PIN_NUM_RST     GPIO_NUM_4
#define PIN_NUM_BL      GPIO_NUM_21
#define TOUCH_CS        GPIO_NUM_33
#define TOUCH_IRQ       GPIO_NUM_36
#define TOUCH_MOSI      GPIO_NUM_32
#define TOUCH_MISO      GPIO_NUM_39
#define TOUCH_CLK       GPIO_NUM_25

//WxH
#define LCD_WIDTH       320
#define LCD_HEIGHT      240

//=========== Color Palette ===========
#define UI_BG               0x0000
#define UI_WHITE            0xFFFF
#define UI_GRAY             0x8408
#define UI_ACCENT           0x06FF
#define UI_WARN             0xFC60
#define UI_GREEN            0x000F
#define UI_RED              0x1000
#define UI_YELLOW           0x1001
#define UI_GOLD             0xF5A0 
#define UI_PACKER_GREEN     0x0007
#define UI_ORANGE           0x1702 
orbit_err_t _display_init(void);
void _display_clear(uint16_t color);
void _display_hello_world(void);
void _display_main_UI(void);


//=============== F1 ===================
#define OPENF1_URL "https://api.openf1.org/v1/championship_drivers?session_key=latest&driver_number=4&driver_number=81&driver_number=16&driver_number=44"

typedef struct {
    int   driver_number;
    char  name[16];
    int   position;
    float points;
} orbit_driver_t;

typedef struct {
    orbit_driver_t norris;    // #4
    orbit_driver_t piastri;   // #81
} orbit_mclaren_t;

typedef struct {
    orbit_driver_t leclerc;   // #16
    orbit_driver_t hamilton;  // #44
} orbit_ferrari_t;

typedef struct {
    orbit_mclaren_t mclaren;
    orbit_ferrari_t ferrari;
} orbit_f1_data_t;

extern orbit_f1_data_t g_f1_data;

void f1_print(orbit_f1_data_t *data); //for testing
orbit_err_t f1_fetch(orbit_f1_data_t *out);



//========= WEATHER ==================
#define WEATHER_URL "https://api.open-meteo.com/v1/forecast?latitude=43.0117&longitude=-88.2315&current=temperature_2m,apparent_temperature,weather_code&daily=temperature_2m_max&timezone=auto"
typedef struct {
    char day[4];      // MON, TUE, WED
    int temp_max;
} orbit_weather_day_t;

typedef struct {
    char city[32];

    int temp_c;
    int feels_c;
    int weather_code;

    char condition[24];

    orbit_weather_day_t forecast[3];

    int valid;
} orbit_weather_data_t;

extern orbit_weather_data_t g_weather_data;
orbit_err_t weather_fetch(orbit_weather_data_t *out);


//========= PACKERS ===================
#define PACKERS_URL \
"https://sports.core.api.espn.com/v2/sports/football/leagues/nfl/seasons/2026/teams/9/events?lang=en&region=us"


typedef struct {
    char opponent[16];
    char date[16];
    char status[16];

    int packers_score;
    int opponent_score;

    int is_final;
    int valid;
} orbit_packers_data_t;

extern orbit_packers_data_t g_packers_data;
orbit_err_t packers_fetch(orbit_packers_data_t *out);

#endif