/**
 * @brief Handles all Wi-Fi stuff for orbit
 *
 *
 * Modes:
 *  - Settings mode → creates a hotspot so you can connect and change settings
 *  - Standard mode → normal connects to home wifi
 *
 */


#include "globalVar.h"

char availableNetworks[5120]; // Buffer to hold scanned WiFi SSIDs for portal display
bool wifi_connected = 0;

 orbit_err_t _network_init(void) {
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
    assert(ap_netif);
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));



    return ORBIT_OK;

}


orbit_err_t _network_settings_mode(){

    wifi_config_t default_cfg = { 
        .ap = {
            .ssid = NETWORK_SETTINGS_SSID,
            .password = "",
            .channel = 3,
            .ssid_len = strlen(NETWORK_SETTINGS_SSID),
            .authmode = WIFI_AUTH_OPEN,
            .max_connection = NETWORK_MAX_CONN,
        },
    };

    // Copy SSID string from system settings
    // strncpy((char *)default_cfg.ap.ssid, system_settings.WIFI_SSID, sizeof(default_cfg.ap.ssid) - 1);
    // default_cfg.ap.ssid_len = strlen(system_settings.WIFI_SSID);

    // Set Wi-Fi to Access Point mode
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &default_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    vTaskDelay(pdMS_TO_TICKS(200));

    return ORBIT_OK;
}

orbit_err_t _network_connect_mode(){
    
    wifi_config_t master_cfg = {
        .sta = {
            .ssid = {0},
            .password = {0},
            .channel = 3
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &master_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    return ORBIT_OK;
}

orbit_err_t _network_connect_to_wifi(const char* ssid, const char* password){

    wifi_config_t master_cfg = {
        .sta = {
            .ssid = {0},
            .password = {0},
            .channel = 3
        }
    };

    strncpy((char *)master_cfg.sta.ssid, ssid, sizeof(master_cfg.sta.ssid) - 1);
    strncpy((char *)master_cfg.sta.password, password, sizeof(master_cfg.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &master_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_err_t e = esp_wifi_connect();
    if(e == ESP_OK)wifi_connected = 1;

    vTaskDelay(pdMS_TO_TICKS(1000)); // wait for connection to establish (or fail)

    return ORBIT_OK;
}
