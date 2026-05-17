/**
 * @details Format: "&<codex>&<value>&"
 * @example "&0x0A&1&" -> FIRST_BOOT = 1
 */

#ifndef ORBIT_SETTINGS_CODEX_H
#define ORBIT_SETTINGS_CODEX_H

// ===================== SETTINGS CODEX =====================
#define FIRST_BOOT_CODEX                            0x0A
#define USER_NAME_CODEX                             0x0B
#define DEVICE_NAME_CODEX                           0x0C
#define DEVICE_LANG_CODEX                           0x0D
#define DEVICE_UNITS_CODEX                          0x0E
#define DEVICE_THEME_CODEX                          0x0F
#define DEVICE_F1_CODEX                             0x12
#define PASSCODE_CODEX                              0x10
#define RECOVERY_CODE_CODEX                         0x11


#define FIRMWARE_VERSION_CODEX                      0xA0
#define BUILD_DATE_CODEX                            0xA1
#define DEVICE_ID_CODEX                             0xA2
#define HARDWARE_REV_CODEX                          0xA3
#define BLUETOOTH_VERSION_CODEX                     0xA4

#define MANUFACTURER_CODEX                          0xB5
#define MODEL_NAME_CODEX                            0xB6


#define ADV_WIFIS_CODEX                            0xC0 //used to send available wifi SSIDs to portal for display
#define CON_WIFI_CODEX                             0xC1 //used to send selected wifi SSID and password for connection


#define UPDATE_TIME                                0xF6
#define UPDATE_DATE                                0xF9

#endif // ZAIRE_SYSTEMS_PORTAL_SETTINGS_CODEX_H