/**
 * @brief Settings portal handler logic
 *
 * @details
 * Processes incoming settings updates from the web portal.
 * The HTTP handler parses raw request data into a codex key/value pair,
 * then routes it to the appropriate update function defined here.
 *
 * Expected format:
 *      "&<KEY>&<VALUE>&"
 *
 * Example:
 *      "&0x0A&1&"        -> FIRST_BOOT = 1
 *      "&0x0B&Yann&"     -> USER_NAME = "Yann"
 *
 * Each function updates a specific setting and persists it to NVS.
 *
 * Storage Model:
 *      - handle = namespace (usr / sys / info)
 *      - key    = specific stored object
 *      - struct = memory representation
 *
 * @author Yann Kabambi
 * @name   Orbit
 */

#include "globalVar.h"

//
// ======================== SYSTEM INFO ========================
//

orbit_err_t settings_update_system_info(const device_info_t *info){

    if (nvs_write_blob(
            sys_handle,
            NVS_KEY_DEVICE_INFO,
            info,                          
            sizeof(device_info_t)) != ORBIT_OK)
        return ORBIT_ERR;

    return ORBIT_OK;
}

const char *settings_get_manufacturer(void){
    if(nvs_read_blob(info_handle,
        NVS_KEY_DEVICE_INFO,
        &g_device_info,
        sizeof(g_device_info)) != ORBIT_OK) return NULL;

    return g_device_info.manufacturer;
}

const char *settings_get_model_name(void){
    if(nvs_read_blob(info_handle,
        NVS_KEY_DEVICE_INFO,
        &g_device_info,
        sizeof(g_device_info)) != ORBIT_OK) return NULL;

    return g_device_info.model_name;
}

const char *settings_get_firmware_version(void){
    
    if(nvs_read_blob(info_handle,
        NVS_KEY_DEVICE_INFO,
        &g_device_info,
        sizeof(g_device_info)) != ORBIT_OK) return NULL;

    return g_device_info.firmware_version;

}

const char *settings_get_build_date(void){
    if(nvs_read_blob(info_handle,
        NVS_KEY_DEVICE_INFO,
        &g_device_info,
        sizeof(g_device_info)) != ORBIT_OK) return NULL;

    return g_device_info.build_date;

}
const char *settings_get_device_id(void){
    if(nvs_read_blob(info_handle,
        NVS_KEY_DEVICE_INFO,
        &g_device_info,
        sizeof(g_device_info)) != ORBIT_OK) return NULL;
    return g_device_info.device_id;
}

const char *settings_get_hardware_rev(void){
    if(nvs_read_blob(info_handle,
        NVS_KEY_DEVICE_INFO,
        &g_device_info,
        sizeof(g_device_info)) != ORBIT_OK) return NULL;
    return g_device_info.hardware_revision;
}


//
// ======================== SYSTEM SECURE ========================
//

/**
 * @brief Updates first_boot system flag.
 *
 * 1 = onboarding complete
 * 0 = onboarding required
 */
orbit_err_t settings_update_first_boot(const char *first_boot_values){

    g_sys_secure_info.first_boot = (first_boot_values[0] == '1') ? 1 : 0;

    if (nvs_write_blob(sys_handle,
        NVS_KEY_SYSTEM_SECURE,
        &g_sys_secure_info,
        sizeof(g_sys_secure_info)) != ORBIT_OK) return ORBIT_ERR;

    return ORBIT_OK;
}

orbit_err_t settings_get_first_boot(){
    
    if (nvs_read_blob(sys_handle,
        NVS_KEY_SYSTEM_SECURE,
        &g_sys_secure_info,
        sizeof(g_sys_secure_info)) != ORBIT_OK) return ORBIT_ERR;

    
    return g_sys_secure_info.first_boot;

}

//
// ======================== USER PREFS ========================
//

/**
 * @brief Updates user name in NVS.
 */
orbit_err_t settings_update_user_name(const char *user_name){

    strncpy(g_user_prefs.user_name, user_name, sizeof(g_user_prefs.user_name) - 1);
    g_user_prefs.user_name[sizeof(g_user_prefs.user_name) - 1] = '\0';

    if (nvs_write_blob(usr_handle,
        NVS_KEY_USER_PREFS,
        &g_user_prefs,
        sizeof(g_user_prefs)) != ORBIT_OK) return ORBIT_ERR;

    vTaskDelay(pdMS_TO_TICKS(100));
    nvs_read_blob(usr_handle,NVS_KEY_USER_PREFS, &g_user_prefs, sizeof(g_user_prefs));

    printf("in name: %s\n", g_user_prefs.user_name);

    return ORBIT_OK;
}


const char *settings_get_user_name(){

    nvs_read_blob(
        usr_handle,
        NVS_KEY_USER_PREFS,
        &g_user_prefs,
        sizeof(g_user_prefs));

    printf("name: %s\n", g_user_prefs.user_name);
    printf("pressed\n");

    return g_user_prefs.user_name;
}


/**
 * @brief Updates device name in NVS.
 */
orbit_err_t settings_update_device_name(const char *device_name){

    strncpy(g_user_prefs.device_name, device_name, sizeof(g_user_prefs.device_name) - 1);
    g_user_prefs.device_name[sizeof(g_user_prefs.device_name) - 1] = '\0';

    if (nvs_write_blob(usr_handle,
        NVS_KEY_USER_PREFS,
        &g_user_prefs,
        sizeof(g_user_prefs)) != ORBIT_OK) return ORBIT_ERR;

    return ORBIT_OK;
}
const char *settings_get_device_name(){
    return g_user_prefs.device_name;
}


//
// ======================== SYSTEM PREFS ========================
//

/**
 * @brief Updates device language in NVS.
 */
orbit_err_t settings_update_device_lang(const char *device_lang){

    strncpy(g_system_prefs.device_lang, device_lang, sizeof(g_system_prefs.device_lang) - 1);
    g_system_prefs.device_lang[sizeof(g_system_prefs.device_lang) - 1] = '\0';

    if (nvs_write_blob(sys_handle,
        NVS_KEY_SYSTEM_PREFS,
        &g_system_prefs,
        sizeof(g_system_prefs)) != ORBIT_OK) return ORBIT_ERR;

    return ORBIT_OK;
}

const char *settings_get_device_lang(){

    if (nvs_read_blob(sys_handle,
        NVS_KEY_SYSTEM_PREFS,
        &g_system_prefs,
        sizeof(g_system_prefs)) != ORBIT_OK) return NULL;

    return g_system_prefs.device_lang;
}

/**
 * @brief Updates device units in NVS.
 */
orbit_err_t settings_update_device_units(const char *device_units){

    g_system_prefs.units = (device_units[0] == '1') ? 1 : 0;
    if (nvs_write_blob(sys_handle,
        NVS_KEY_SYSTEM_PREFS,
        &g_system_prefs,
        sizeof(g_system_prefs)) != ORBIT_OK) return ORBIT_ERR;

    return ORBIT_OK;
}

uint8_t settings_get_device_units(){

    if (nvs_read_blob(sys_handle,
        NVS_KEY_SYSTEM_PREFS,
        &g_system_prefs,
        sizeof(g_system_prefs)) != ORBIT_OK) return ORBIT_ERR;

    return g_system_prefs.units;
}

/**
 * @brief Updates device F1 in NVS.
 */
orbit_err_t settings_update_device_f1(const char *device_f1){

    g_system_prefs.f1 = (device_f1[0] == '1') ? 1 : 0;
    if (nvs_write_blob(sys_handle,
        NVS_KEY_SYSTEM_PREFS,
        &g_system_prefs,
        sizeof(g_system_prefs)) != ORBIT_OK) return ORBIT_ERR;

    return ORBIT_OK;
}

uint8_t settings_get_device_f1(){

    if (nvs_read_blob(sys_handle,
        NVS_KEY_SYSTEM_PREFS,
        &g_system_prefs,
        sizeof(g_system_prefs)) != ORBIT_OK) return ORBIT_ERR;

    return g_system_prefs.f1;
}


/**
 * @brief Updates device theme in NVS.
 */
orbit_err_t settings_update_device_theme(const char *device_theme){

    g_system_prefs.theme = (device_theme[0] == '1')? 1 : 0;

    if (nvs_write_blob(sys_handle,
        NVS_KEY_SYSTEM_PREFS,
        &g_system_prefs,
        sizeof(g_system_prefs)) != ORBIT_OK) return ORBIT_ERR;

    return ORBIT_OK;
}



uint8_t settings_get_device_theme(){

    if (nvs_read_blob(sys_handle,
        NVS_KEY_SYSTEM_PREFS,
        &g_system_prefs,
        sizeof(g_system_prefs)) != ORBIT_OK) return ORBIT_ERR;

    return g_system_prefs.theme;
}



// ========================= FAKE SECURITY TBH =========================


orbit_err_t settings_update_passcode(const char *passcode){


    strncpy(g_usr_secure_info.passcode, passcode, sizeof(g_usr_secure_info.passcode) - 1);
    g_usr_secure_info.passcode[sizeof(g_usr_secure_info.passcode) - 1] = '\0';

    if (nvs_write_blob(sys_handle,
        NVS_KEY_USER_SECURE,
        &g_usr_secure_info,
        sizeof(g_usr_secure_info)) != ORBIT_OK) return ORBIT_ERR;

    return ORBIT_OK;
}

orbit_err_t settings_check_passcode(const char *input_passcode){

    if (nvs_read_blob(sys_handle,
        NVS_KEY_USER_SECURE,
        &g_usr_secure_info,
        sizeof(g_usr_secure_info)) != ORBIT_OK) return ORBIT_ERR;

        printf("Checking passcode: input '%s' vs stored '%s'\n", input_passcode, g_usr_secure_info.passcode); // Debug print to verify values

    return (strcmp(input_passcode, g_usr_secure_info.passcode) == 0) ? ORBIT_OK : ORBIT_ERR;
}

orbit_err_t settings_update_recovery_code(const char *recovery_code){

    strncpy(g_usr_secure_info.recovery_code, recovery_code, sizeof(g_usr_secure_info.recovery_code) - 1);
    g_usr_secure_info.recovery_code[sizeof(g_usr_secure_info.recovery_code) - 1] = '\0';

    if (nvs_write_blob(sys_handle,
        NVS_KEY_USER_SECURE,
        &g_usr_secure_info,
        sizeof(g_usr_secure_info)) != ORBIT_OK) return ORBIT_ERR;

    return ORBIT_OK;
}