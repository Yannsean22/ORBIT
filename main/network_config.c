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