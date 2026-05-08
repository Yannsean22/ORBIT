/**
 * @brief NVS (flash storage) helpers for ORBIT
 *
 * This file handles reading and writing data to flash using NVS.
 * Basically how the helmet remembers stuff after reboot.
 *
 * Handles:
 *  - Initializing NVS namespaces (user, system, device info)
 *  - Writing strings (like usernames)
 *  - Writing blobs (structs, arrays, settings)
 *  - Reading strings
 *  - Reading blobs
 *
 * Used for things like:
 *  - user name
 *  - system preferences
 *  - button configs
 *  - device info
 *
 * @note
 * Writes are committed immediately after setting values.
 *
 * @warning
 * Make sure read buffers are big enough, or data can get cut off
 * or cause errors.
 *
 * @author Yann Kabambi
 * @project ORBIT SYSTEMS
 */


#include "globalVar.h"


nvs_handle_t usr_handle;
nvs_handle_t sys_handle;
nvs_handle_t info_handle;

orbit_err_t nvs_init(void){
    if(nvs_open(CFG_USR_NAMESPACE_KEY,  NVS_READWRITE, &usr_handle)  != ESP_OK) return ORBIT_ERR;
    if(nvs_open(CFG_SYS_NAMESPACE_KEY,  NVS_READWRITE, &sys_handle)  != ESP_OK) return ORBIT_ERR;
    if(nvs_open(SYS_INFO_NAMESPACE_KEY, NVS_READWRITE, &info_handle) != ESP_OK) return ORBIT_ERR;

    return ORBIT_OK;
}


// usage: nvs_write_char(usr_handle, "username", "john");
orbit_err_t nvs_write_char(nvs_handle_t handle, const char *key, const char *value){
    if(nvs_set_str(handle, key, value)   != ESP_OK) return ORBIT_ERR;
    if(nvs_commit(handle)                != ESP_OK) return ORBIT_ERR;
    return ORBIT_OK;
}

// usage: nvs_write_blob(usr_handle, "btn_order", btn_array, sizeof(btn_array));
orbit_err_t nvs_write_blob(nvs_handle_t handle, const char *key, const void *value, size_t size){
    if(nvs_set_blob(handle, key, value, size) != ESP_OK) return ORBIT_ERR;
    if(nvs_commit(handle)                     != ESP_OK) return ORBIT_ERR;
    return ORBIT_OK;
}

/*
 usage: char username[32];
        nvs_read_char(usr_handle, "username", username, sizeof(username));
*/
orbit_err_t nvs_read_char(nvs_handle_t handle, const char *key, char *out, size_t size){
    if(nvs_get_str(handle, key, out, &size) != ESP_OK) return ORBIT_ERR;
    return ORBIT_OK;
}

/*
 usage: uint8_t btn_order[4];
        nvs_read_blob(usr_handle, "btn_order", btn_order, sizeof(btn_order));
*/
orbit_err_t nvs_read_blob(nvs_handle_t handle, const char *key, void *out, size_t size){
    if(nvs_get_blob(handle, key, out, &size) != ESP_OK) return ORBIT_ERR;
    return ORBIT_OK;
}