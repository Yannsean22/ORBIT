#include "globalVar.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"

orbit_packers_data_t g_packers_data;



static const char *TAG = "ORBIT_PACKERS";

static char packers_buf[8192];
static int  packers_buf_len = 0;

static esp_err_t _packers_http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ON_CONNECTED:
            packers_buf_len = 0;
            memset(packers_buf, 0, sizeof(packers_buf));
            break;

        case HTTP_EVENT_ON_DATA:
            if(packers_buf_len + evt->data_len < sizeof(packers_buf)) {
                memcpy(packers_buf + packers_buf_len, evt->data, evt->data_len);
                packers_buf_len += evt->data_len;
            }
            break;

        default:
            break;
    }

    return ESP_OK;
}

static orbit_err_t packers_parse(orbit_packers_data_t *out)
{
    cJSON *root = cJSON_Parse(packers_buf);
    if(root == NULL) {
        out->valid = 0;
        return ORBIT_ERR;
    }

    cJSON *count = cJSON_GetObjectItem(root, "count");
    cJSON *items = cJSON_GetObjectItem(root, "items");

    if(!count || !items || !cJSON_IsArray(items)) {
        cJSON_Delete(root);
        out->valid = 0;
        return ORBIT_ERR;
    }

    int game_count = count->valueint;

    snprintf(out->opponent, sizeof(out->opponent), "TBD");
    snprintf(out->date, sizeof(out->date), "%d games", game_count);
    snprintf(out->status, sizeof(out->status), "2026 sched");

    out->packers_score = 0;
    out->opponent_score = 0;
    out->is_final = 0;
    out->valid = 1;

    cJSON_Delete(root);
    return ORBIT_OK;
}



orbit_err_t packers_fetch(orbit_packers_data_t *out)
{
    memset(out, 0, sizeof(orbit_packers_data_t));

    esp_http_client_config_t config = {
        .url                         = PACKERS_URL,
        .event_handler               = _packers_http_event_handler,
        .crt_bundle_attach           = esp_crt_bundle_attach,
        .skip_cert_common_name_check = true,
        .timeout_ms                  = 8000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);
    esp_http_client_cleanup(client);

    if(err != ESP_OK) {
        printf("packers fetch failed: %s\n", esp_err_to_name(err));
        out->valid = 0;
        return ORBIT_ERR;
    }

    return packers_parse(out);
}

void packers_print(orbit_packers_data_t *data)
{
    printf("\n===== PACKERS =====\n");

    if(!data->valid) {
        printf("No Packers data\n");
        return;
    }

    printf("GB vs %s\n", data->opponent);
    printf("Date: %s\n", data->date);
    printf("Status: %s\n", data->status);
    printf("Score: GB %d - %s %d\n",
           data->packers_score,
           data->opponent,
           data->opponent_score);

    printf("===================\n");
}