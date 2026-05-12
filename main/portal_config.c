/**
 * @brief Web server and UI handler module
 *
 * This file runs the ESP32 web server and basically connects the web portal (UI).
 *
 * Handles:
 *  - Serving the UI (HTML, JS, CSS, media from SPIFFS)
 *  - Receiving settings updates from the portal
 *  - Sending device data back to the UI on load
 *
 * Settings use a custom format:
 *      "&<KEY>&<VALUE>&"
 * Example:
 *      "&10&Yann&"
 *
 * It also exposes endpoints for:
 *  - Dashboard pages
 *  - Media (photos)
 *  - Device/system info
 *
 * @note
 * If you change system structs (system_prefs_t, device_state, etc.),
 * make sure this file and the UI stay in sync.
 *
 * @warning
 * This assumes the input format is correct — bad data can break parsing.
 *
 * @author Yann Kabambi
 */

#include "globalVar.h"


static const char *SET_TAG = "settings";

// Exported so dns_server.c can reference it
httpd_handle_t web_server = NULL;

/* ===== Index File Handlers ===== */
static esp_err_t settings_set_settings_data_handler(httpd_req_t *req)
{
    // Buffer to hold entire incoming POST body
    char val[512] = {0};

    // Buffer to hold extracted VALUE (after parsing)
    char value[64] = {0};

    // Will store the codex key (e.g., 0x0A, 0x0B, etc.)
    uint8_t key = 0;

    // Return value from httpd_req_recv
    int ret;

    // ===================== STEP 1: READ REQUEST BODY =====================
    // Read incoming data into 'val'
    ret = httpd_req_recv(req, val, MIN(req->content_len, sizeof(val) - 1));

    // If nothing received or error -> return HTTP 400
    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to read body");
        return ESP_FAIL;
    }

    // Null-terminate string so it can be safely used as C string
    val[ret] = '\0';

    printf("Received body: %s\n", val); // Debug print to verify received data

    // ===================== STEP 2: SPLIT STRING =====================
    // We expect format: "&KEY&VALUE&"
    // Example: "&10&Yann&"

    char *parts[16];   // Array to hold split tokens
    int count = 0;     // Number of tokens found

    // Split string using '&' as delimiter
    char *token = strtok(val, "&");

    while (token != NULL && count < 16) {
        parts[count++] = token;   // Store token pointer
        token = strtok(NULL, "&"); // Continue splitting
    }

    // Example result for "&10&Yann&":
    // parts[0] = "10"
    // parts[1] = "Yann"
    // count = 2

    // ===================== STEP 3: VALIDATE =====================
    // We need at least KEY and VALUE
    if (count < 2) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Malformed body");
        return ESP_FAIL;
    }

    // ===================== STEP 4: EXTRACT KEY + VALUE =====================

    // Convert KEY from string to integer -- IMPORTANT: HAS TO STAY AT INDEX 1
    key = (uint8_t)atoi(parts[1]);

    // Copy VALUE safely into buffer -- IMPORTANT: HAS TO STAY AT INDEX 2
    strncpy(value, parts[2], sizeof(value) - 1);

    // Ensure null-termination (important for safety)
    value[sizeof(value) - 1] = '\0';

    // Debug print (optional but very useful)

    //return 
    orbit_err_t change = 0;

    vTaskDelay(pdMS_TO_TICKS(10)); //small delay never hurts in these situations, especially when dealing with async web requests and potential NVS writes

    // ===================== STEP 5: HANDLE SETTING =====================
    switch (key)
    {
        case FIRST_BOOT_CODEX:
            change = settings_update_first_boot(value);
            break;

        case USER_NAME_CODEX:
            change = settings_update_user_name(value);
            break;

        case DEVICE_NAME_CODEX:
            change = settings_update_device_name(value);
            break;

        case DEVICE_LANG_CODEX:
            change = settings_update_device_lang(value);
            break;

        case DEVICE_UNITS_CODEX:
            change = settings_update_device_units(value);
            break;

        case DEVICE_THEME_CODEX:
            change = settings_update_device_theme(value);
            break;

        case DEVICE_F1_CODEX:
            change = settings_update_device_f1(value);
            break;

        case PASSCODE_CODEX:
            change = settings_update_passcode(value);
            break;

        case RECOVERY_CODE_CODEX:
            change = settings_update_recovery_code(value);
            break;

        case CON_WIFI_CODEX:
            orbit_err_t e = _network_connect_to_wifi(parts[2], parts[3]);

            if(e == ORBIT_OK){
                if(settings_update_wifi_security(parts[2], parts[3]) == ORBIT_OK){
                    change = ORBIT_OK;
                } else {
                    change = ORBIT_ERR;
                }
                
            } else {
                change = ORBIT_ERR;
            }

            break;

        default:
            printf("Unknown codex: %u\n", key);
            break;
    }

    // ===================== STEP 6: SEND RESPONSE =====================
    char r[32];
    

    if(change == ORBIT_OK)sniprintf(r, sizeof(r), "ORBIT_OK");
    if(change == ORBIT_ERR)sniprintf(r, sizeof(r), "ORBIT_ERR");


    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, r, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static esp_err_t settings_get_settings_data_handler(httpd_req_t *req) // Handle init data fetching from settings page
{
   
    // Buffer to hold entire incoming POST body
    char val[512] = {0};

    // Buffer to hold extracted VALUE (after parsing)
    char value[64] = {0};
    char pass[64] = {0};
    // Will store the codex key (e.g., 0x0A, 0x0B, etc.)
    uint8_t key = 0;

    // Return value from httpd_req_recv
    int ret;

    // ===================== STEP 1: READ REQUEST BODY =====================
    // Read incoming data into 'val'
    ret = httpd_req_recv(req, val, MIN(req->content_len, sizeof(val) - 1));

    // If nothing received or error -> return HTTP 400
    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to read body");
        return ESP_FAIL;
    }

    // Null-terminate string so it can be safely used as C string
    val[ret] = '\0';

    printf("Received body: %s\n", val); // Debug print to verify received data

    // ===================== STEP 2: SPLIT STRING =====================
    // We expect format: "&KEY&VALUE&"
    // Example: "&10&Yann&"

    char *parts[16];   // Array to hold split tokens
    int count = 0;     // Number of tokens found

    // Split string using '&' as delimiter
    char *token = strtok(val, "&");

    while (token != NULL && count < 16) {
        parts[count++] = token;   // Store token pointer
        token = strtok(NULL, "&"); // Continue splitting
    }

    // Convert KEY from string to integer -- IMPORTANT: HAS TO STAY AT INDEX 1
    key = (uint8_t)atoi(parts[1]);

    // Copy VALUE safely into buffer -- IMPORTANT: HAS TO STAY AT INDEX 2
    strncpy(pass, parts[2], sizeof(pass) - 1);

    pass[sizeof(pass) - 1] = '\0';
    
    // ===================== STEP 5: HANDLE SETTING =====================
    switch (key)
    {

        case FIRST_BOOT_CODEX:
            snprintf(value, sizeof(value), "%d",settings_get_first_boot());
            break;

        case USER_NAME_CODEX:
            snprintf(value, sizeof(value), "%s",settings_get_user_name());
            break;

        case DEVICE_NAME_CODEX:
            snprintf(value, sizeof(value), "%s",settings_get_device_name());
            break;

        case DEVICE_LANG_CODEX:
            snprintf(value, sizeof(value), "%s",settings_get_device_lang());
            break;

        case DEVICE_UNITS_CODEX:
            snprintf(value, sizeof(value), "%d",settings_get_device_units());
            break;

        case DEVICE_THEME_CODEX:
            snprintf(value, sizeof(value), "%d",settings_get_device_theme());
            break;

        case DEVICE_F1_CODEX:
            snprintf(value, sizeof(value), "%d",settings_get_device_f1());
            break;
         
        case PASSCODE_CODEX:
        
            snprintf(value, sizeof(value), "%s",(settings_check_passcode(pass) == ORBIT_OK ? "ORBIT_OK" : "ORBIT_ERR")); //never send passcode back to UI
           
            break;

        case ADV_WIFIS_CODEX:

            printf("Fetching available WiFi SSIDs...\n");
            orbit_err_t e = _network_connect_mode();
            if(e == ORBIT_ERR)return ESP_FAIL;

            vTaskDelay(pdMS_TO_TICKS(2000)); //wait for wifi scan to populate availableNetworks

            wifi_scan_config_t scan_cfg = {
                .ssid = NULL,
                .bssid = NULL,
                .channel = 0,
                .show_hidden = true
            };

            while(1){
                ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_cfg, true)); // true = blocking

                uint16_t ap_count = 0;
                esp_wifi_scan_get_ap_num(&ap_count);

                wifi_ap_record_t ap_list[ap_count];
                esp_wifi_scan_get_ap_records(&ap_count, ap_list);

                uint8_t first_index = 1; //used to avoid leading comma in string

                for(int i = 0; i < ap_count; i++){
                   
                    if(strcmp((char*)ap_list[i].ssid, "") != 0){ // Filter out empty SSIDs
                        if(first_index == 0)strcat(availableNetworks, ",");
                        strcat(availableNetworks, (char*)ap_list[i].ssid);
                        first_index = 0;
                    }
                 
                }

                break;
            }

            printf("Available Networks: %s\n", availableNetworks); // Debug print

            httpd_resp_set_type(req, "text/plain");
            httpd_resp_send(req, availableNetworks, strlen(availableNetworks));
            vTaskDelay(pdMS_TO_TICKS(200)); //small delay to ensure response is sent before next scan starts
            
            memset(availableNetworks, 0, sizeof(availableNetworks)); // Clear buffer for next scan results
            return ESP_OK; //prevent unwanted calls

            break;

        default:
            printf("Unknown codexSS: %u\n", key);
            break;
    }

  

    // ===================== STEP 6: SEND RESPONSE =====================
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, value, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static esp_err_t settings_leave_portal_handler(httpd_req_t *req){
    // This endpoint is called when the user clicks "Leave Portal" on the UI. It can be used to trigger any cleanup or state changes needed when exiting the portal.

    // For now, we just send a simple response back to the client.
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "ORBIT_OK", HTTPD_RESP_USE_STRLEN);

    stop_dns_server(); // Stop DNS server to allow normal network operation to resume

    return ESP_OK;
}

esp_err_t main_page_get_handler(httpd_req_t *req)
{

    FILE *index = fopen("/spiffs/index.html", "r");
    if (!index) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    char line[256];
    httpd_resp_set_type(req, "text/html");
    while (fgets(line, sizeof(line), index)) {
        httpd_resp_sendstr_chunk(req, line);
    }

    fclose(index);
    httpd_resp_sendstr_chunk(req, NULL); // Signal end of chunked response
    return ESP_OK;
}

// Optional: avoid 404 spam from favicon requests
static esp_err_t favicon_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "image/x-icon");
    // Tiny 1×1 empty response
    return httpd_resp_send(req, "", 0);
}

esp_err_t js_handler(httpd_req_t *req){
    FILE *f = fopen("/spiffs/script.js", "r");
    if (!f) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    char line[256];
    httpd_resp_set_type(req, "application/javascript");
    while (fgets(line, sizeof(line), f)) {
        httpd_resp_sendstr_chunk(req, line);
    }

    fclose(f);
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}

esp_err_t globalData_handler(httpd_req_t *req){
    FILE *f = fopen("/spiffs/globalData.js", "r");
    if (!f) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    char line[256];
    httpd_resp_set_type(req, "application/javascript");
    while (fgets(line, sizeof(line), f)) {
        httpd_resp_sendstr_chunk(req, line);
    }

    fclose(f);
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}

esp_err_t style_handler(httpd_req_t *req){
    FILE *f = fopen("/spiffs/style.css", "r");
    if (!f) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    char line[256];
    httpd_resp_set_type(req, "text/css");
    while (fgets(line, sizeof(line), f)) {
        httpd_resp_sendstr_chunk(req, line);
    }

    fclose(f);
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}

esp_err_t logo_handler(httpd_req_t *req)
{
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "/spiffs/photo_%d.jpg",1); 
    const char *path = "/spiffs/media/photos/pic_2.jpg";

    FILE *f = fopen(path, "rb");
    if (!f) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/png");
    httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=86400"); // optional

    char buf[1024];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        if (httpd_resp_send_chunk(req, buf, n) != ESP_OK) {
            fclose(f);
            httpd_resp_sendstr_chunk(req, NULL); // end chunked transfer
            return ESP_FAIL;
        }
    }
    fclose(f);
    httpd_resp_sendstr_chunk(req, NULL); // end chunked transfer
    return ESP_OK;
}

/* ====== DASHBOARD FILES Handlers ===== */

esp_err_t dashboard_screen_page_get_handler(httpd_req_t *req)
{

    FILE *index = fopen("/spiffs/www/homeScreen.html", "r");
    if (!index) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    char line[256];
    httpd_resp_set_type(req, "text/html");
    while (fgets(line, sizeof(line), index)) {
        httpd_resp_sendstr_chunk(req, line);
    }

    fclose(index);
    httpd_resp_sendstr_chunk(req, NULL); // Signal end of chunked response
    return ESP_OK;
}

esp_err_t dashboard_js_handler(httpd_req_t *req){
    FILE *f = fopen("/spiffs/www/data.js", "r");
    if (!f) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    char line[256];
    httpd_resp_set_type(req, "application/javascript");
    while (fgets(line, sizeof(line), f)) {
        httpd_resp_sendstr_chunk(req, line);
    }

    fclose(f);
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}

esp_err_t dashboard_style_handler(httpd_req_t *req){
    FILE *f = fopen("/spiffs/www/dashStyle.css", "r");
    if (!f) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    char line[256];
    httpd_resp_set_type(req, "text/css");
    while (fgets(line, sizeof(line), f)) {
        httpd_resp_sendstr_chunk(req, line);
    }

    fclose(f);
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}


esp_err_t httpd_serve_static_files(httpd_req_t *req) {

    char filepath[128];
    const char *uri_path = req->uri + strlen("/media/photos/");

    // prevent directory traversal
    if (strstr(uri_path, "..") || uri_path[0] == '/') {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    // build full SPIFFS path
    snprintf(filepath, sizeof(filepath), "/spiffs/media/photos/%s", uri_path);

    // open file
    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    // get file size
    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // content type
    const char *ext = strrchr(uri_path, '.');
    if (ext) {
        if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
            httpd_resp_set_type(req, "image/jpeg");
        } else if (strcmp(ext, ".png") == 0) {
            httpd_resp_set_type(req, "image/png");
        }
    }

    // content length
    char size_str[16];
    snprintf(size_str, sizeof(size_str), "%ld", st.st_size);
    httpd_resp_set_hdr(req, "Content-Length", size_str);

    // send in chunks
    char buf[1024];
    ssize_t read_bytes;
    esp_err_t err = ESP_OK;

    while ((read_bytes = read(fd, buf, sizeof(buf))) > 0) {
        if (httpd_resp_send_chunk(req, buf, read_bytes) != ESP_OK) {
            err = ESP_FAIL;
            break;
        }
    }
    close(fd);

    if (read_bytes < 0) err = ESP_FAIL;

    if (err == ESP_OK) {
        httpd_resp_send_chunk(req, NULL, 0);
    }

    return err;
}


/* ===== MAIN URI table ===== */

httpd_uri_t main_page = { .uri = "/", .method = HTTP_GET, .handler = main_page_get_handler, .user_ctx = NULL };
httpd_uri_t favicon_uri = {.uri = "/favicon.ico", .method = HTTP_GET, .handler = favicon_handler, .user_ctx = NULL};
httpd_uri_t script_handler = {.uri = "/script.js", .method = HTTP_GET,.handler = js_handler,.user_ctx = NULL};
httpd_uri_t css_handler = {.uri = "/style.css", .method = HTTP_GET,.handler = style_handler,.user_ctx = NULL};
httpd_uri_t logo_uri = { .uri = "/media/photos/pic_2.jpg", .method = HTTP_GET, .handler = logo_handler, .user_ctx = NULL};
httpd_uri_t globalData_js = { .uri = "/globalData.js", .method = HTTP_GET, .handler = globalData_handler, .user_ctx = NULL};

/* ===== DASHBOARD URI table ===== */
httpd_uri_t dashboard_screen_page = { .uri = "/www/homeScreen.html", .method = HTTP_GET, .handler = dashboard_screen_page_get_handler, .user_ctx = NULL };
httpd_uri_t dashboard_js = {.uri = "/www/data.js", .method = HTTP_GET,.handler = dashboard_js_handler,.user_ctx = NULL};
httpd_uri_t dashboard_css = {.uri = "/www/dashStyle.css", .method = HTTP_GET,.handler = dashboard_style_handler,.user_ctx = NULL};

/* ===== PHOTO URI table ===== */
httpd_uri_t photos_uri = {
    .uri      = "/media/photos/*",
    .method   = HTTP_GET,
    .handler  = httpd_serve_static_files,
    .user_ctx = NULL
};

/* ===== SEVER CONFIGS ===== */
httpd_uri_t settings_fetch_onLoad_data = { .uri = "/settings_get_settings_data", .method = HTTP_POST, .handler = settings_get_settings_data_handler, .user_ctx = NULL};
httpd_uri_t settings_update = { .uri = "/settings_set_settings_data", .method = HTTP_POST, .handler = settings_set_settings_data_handler, .user_ctx = NULL};
httpd_uri_t settings_leave_portal = { .uri = "/settings_leave_portal", .method = HTTP_POST, .handler = settings_leave_portal_handler, .user_ctx = NULL};

/* ===== Server start ===== */

httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 16;
    config.lru_purge_enable = true;
    config.uri_match_fn = httpd_uri_match_wildcard;   // needed**

    if (httpd_start(&web_server, &config) == ESP_OK) {
        httpd_register_uri_handler(web_server, &main_page);
        httpd_register_uri_handler(web_server, &favicon_uri);
        httpd_register_uri_handler(web_server, &script_handler);
        httpd_register_uri_handler(web_server, &css_handler);
        httpd_register_uri_handler(web_server, &logo_uri);
        httpd_register_uri_handler(web_server, &settings_fetch_onLoad_data);
        httpd_register_uri_handler(web_server, &settings_update);
        httpd_register_uri_handler(web_server, &globalData_js);
        httpd_register_uri_handler(web_server, &dashboard_screen_page);
        httpd_register_uri_handler(web_server, &dashboard_js);
        httpd_register_uri_handler(web_server, &dashboard_css);
        httpd_register_uri_handler(web_server, &photos_uri);
        httpd_register_uri_handler(web_server, &settings_leave_portal);

       
      
        ESP_LOGI(SET_TAG, "Web server started (serving embedded Hello World)");
        return web_server;
    }

    ESP_LOGE(SET_TAG, "Web server start failed");
    return NULL;
}