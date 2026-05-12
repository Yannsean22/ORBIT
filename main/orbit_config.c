/**
 * @brief Core project init + global state for Orbit
 *
 * This file is basically the main setup / brain of the project.
 * It initializes everything needed for the helmet to run and
 * holds global state used across the project.
 *
 * Handles:
 *  - First boot setup (default settings)
 *  - NVS init + loading/saving project preferences
 *  - SPIFFS init (for web files, storage, etc.)
 *  - Network init
 *  - Buzzer init
 *
 * Also defines global project structs like:
 *  - user prefs
 *  - project prefs
 *  - device state
 *  - device info
 *
 * Basically: this is where everything gets brought together on startup.
 *
 * @note
 * If you change system_prefs_t, make sure to update the first boot
 * function so all fields are initialized properly.
 *
 * @warning
 * Missing init steps here can break the whole project, so keep this
 * clean and in order.
 *
 * @author Yann Kabambi
 * @name   Orbit
 */

#include "globalVar.h"
#include "orbit_config.h"


user_prefs_t    g_user_prefs;
system_prefs_t  g_system_prefs;
device_info_t   g_device_info;
sys_secure_info_t g_sys_secure_info;
usr_secure_info_t g_usr_secure_info;

/**
 * @brief Default Settings Bootstrap: Initializes system settings if NVS is uninitialized.
 *        Detects erased flash state (0xFF), applies default values, and restarts device.
 *
 * @return ORBIT_OK if bootstrap succeeds or not required.
 * @return ORBIT_ERR if default settings fail to write.
 */
static orbit_err_t _default_settings_bootstrap(void){


    if(settings_update_first_boot("0") != ORBIT_OK)return ORBIT_ERR;
    if(settings_update_user_name("USER") != ORBIT_OK)return ORBIT_ERR;
    if(settings_update_device_name("ORBIT") != ORBIT_OK)return ORBIT_ERR;
    if(settings_update_device_theme("1") != ORBIT_OK)return ORBIT_ERR;
    if(settings_update_device_units("1") != ORBIT_OK)return ORBIT_ERR;
    if(settings_update_device_f1("1") != ORBIT_OK)return ORBIT_ERR;
    if(settings_update_passcode("123456") != ORBIT_OK)return ORBIT_ERR;
    if(settings_update_recovery_code("0") != ORBIT_OK)return ORBIT_ERR;

    strlcpy(g_device_info.manufacturer, "lol's BEDROOM", sizeof(g_device_info.manufacturer));
    strlcpy(g_device_info.model_name, "IDK Y THIS IS HERE", sizeof(g_device_info.model_name));
    strlcpy(g_device_info.firmware_version, "1.0.0", sizeof("1.0.0"));
    strlcpy(g_device_info.build_date, "2026-5-4", sizeof("2026-5-4"));
    strlcpy(g_device_info.build_type, "release", sizeof("release"));
    
    if(_network_init() != ORBIT_OK)return ORBIT_ERR;
    vTaskDelay(pdMS_TO_TICKS(100));
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(g_device_info.device_id,sizeof(g_device_info.device_id),"ORBIT-%02X%02X%02X",mac[3], mac[4], mac[5]); //device ID

    if(settings_update_system_info(&g_device_info) != ORBIT_OK) return ORBIT_ERR;
    return ORBIT_OK;
}

static orbit_err_t _nvs_load_settings(void){

    if (nvs_read_blob(sys_handle,
        NVS_KEY_SYSTEM_PREFS,
        &g_system_prefs,
        sizeof(g_system_prefs)) != ORBIT_OK) return ORBIT_ERR;

    if (nvs_read_blob(usr_handle,
        NVS_KEY_USER_PREFS,
        &g_user_prefs,
        sizeof(g_user_prefs)) != ORBIT_OK) return ORBIT_ERR;

    if (nvs_read_blob(sys_handle,
        NVS_KEY_SYSTEM_SECURE,
        &g_sys_secure_info,
        sizeof(g_sys_secure_info)) != ORBIT_OK) return ORBIT_ERR;

    return ORBIT_OK;
}

orbit_err_t orbit_config_init(void){

    if(nvs_init() != ORBIT_OK)return ORBIT_ERR;
    orbit_err_t err = nvs_read_blob(sys_handle, NVS_KEY_SYSTEM_SECURE, &g_sys_secure_info, sizeof(g_sys_secure_info));
    if (err != ORBIT_OK){
        printf("No system config found - running default settings bootstrap...\n");
        vTaskDelay(pdMS_TO_TICKS(1500));

        if (_default_settings_bootstrap() != ORBIT_OK) {
            printf("Error during default settings bootstrap!\n");

            while (1) {
                printf("FATAL: default settings bootstrap failed\n");
                error_beep();
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }

        printf("Bootstrap complete - restarting device...\n");
        vTaskDelay(pdMS_TO_TICKS(500));
        esp_restart();

        // Catch if restart returns unexpectedly
        while (1) {
            printf("FATAL: esp_restart() returned unexpectedly\n");
            error_beep();
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    if(_nvs_load_settings() != ORBIT_OK)return ORBIT_ERR;
    if(_spiffs_init() != ORBIT_OK)return ORBIT_ERR;
    if(_network_init() != ORBIT_OK)return ORBIT_ERR;
    vTaskDelay(pdMS_TO_TICKS(500)); // prevent display issues
    if(g_sys_secure_info.wifi_enabled)_network_connect_to_wifi(g_sys_secure_info.wifi_ssid, g_sys_secure_info.wifi_pass);// do no care if it fails, user can fix in settings, plus what if the network is off? lol
    vTaskDelay(pdMS_TO_TICKS(500));//prevent display issues
    if(_display_init() != ORBIT_OK)return ORBIT_ERR;
    vTaskDelay(pdMS_TO_TICKS(500));//prevent display issues
    _display_clear(UI_BG);
    return ORBIT_OK;
}

orbit_err_t _spiffs_init(void){


    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "storage",
        .max_files = 20,                    // Increase if you have many files
        .format_if_mount_failed = true      // Keep true during development
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE("SPIFFS", "Partition 'storage' not found in partition table!");
        } else if (ret == ESP_ERR_INVALID_STATE) {
            ESP_LOGE("SPIFFS", "SPIFFS mount failed - possibly corrupted");
        } else {
            ESP_LOGE("SPIFFS", "Failed to register SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ORBIT_ERR;
    }


    // List root directory for debugging
    DIR *d = opendir("/spiffs");
    if (d) {
        struct dirent *e;
        printf("Files in /spiffs:\n");
        while ((e = readdir(d)) != NULL) {
            printf("  - %s\n", e->d_name);
        }
        closedir(d);
    } else {
        printf("Warning: Could not open /spiffs directory\n");
    }


    return ORBIT_OK;
}


