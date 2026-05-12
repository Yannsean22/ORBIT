#include "globalVar.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"

static const char *TAG = "ORBIT_FETCH";

// ---- structs ----


// ---- buffer ----

static char f1_buf[2048];
static int  f1_buf_len = 0;

static esp_err_t _http_event_handler(esp_http_client_event_t *evt){
    switch(evt->event_id){
        case HTTP_EVENT_ON_CONNECTED:
            f1_buf_len = 0;
            memset(f1_buf, 0, sizeof(f1_buf));
            break;
        case HTTP_EVENT_ON_DATA:
            if(f1_buf_len + evt->data_len < sizeof(f1_buf)){
                memcpy(f1_buf + f1_buf_len, evt->data, evt->data_len);
                f1_buf_len += evt->data_len;
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}

// ---- parse ----

static orbit_err_t f1_parse(orbit_f1_data_t *out){
    cJSON *root = cJSON_Parse(f1_buf);
    if(root == NULL) return ORBIT_ERR;

    cJSON *entry = NULL;
    cJSON_ArrayForEach(entry, root){
        int   driver_number = cJSON_GetObjectItem(entry, "driver_number")->valueint;
        int   position      = cJSON_GetObjectItem(entry, "position_current")->valueint;
        float points        = (float)cJSON_GetObjectItem(entry, "points_current")->valuedouble;

        orbit_driver_t *d = NULL;
        switch(driver_number){
            case  4: d = &out->mclaren.norris;   break;
            case 81: d = &out->mclaren.piastri;  break;
            case 16: d = &out->ferrari.leclerc;  break;
            case 44: d = &out->ferrari.hamilton; break;
            default: continue;
        }

        d->driver_number = driver_number;
        d->position      = position;
        d->points        = points;

        switch(driver_number){
            case  4: snprintf(d->name, sizeof(d->name), "Norris");   break;
            case 81: snprintf(d->name, sizeof(d->name), "Piastri");  break;
            case 16: snprintf(d->name, sizeof(d->name), "Leclerc");  break;
            case 44: snprintf(d->name, sizeof(d->name), "Hamilton"); break;
        }
    }

    cJSON_Delete(root);
    return ORBIT_OK;
}

// ---- fetch ----

orbit_err_t f1_fetch(orbit_f1_data_t *out){
    memset(out, 0, sizeof(orbit_f1_data_t));

    esp_http_client_config_t config = {
        .url                         = OPENF1_URL,
        .event_handler               = _http_event_handler,
        .crt_bundle_attach           = esp_crt_bundle_attach,
        .skip_cert_common_name_check = true,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);
    esp_http_client_cleanup(client);

    if(err != ESP_OK){
        printf("fetch failed: %s\n", esp_err_to_name(err));
        return ORBIT_ERR;
    }

    return f1_parse(out);
}

// ---- print ----

void f1_print(orbit_f1_data_t *data){
    printf("\n===== McLaren =====\n");
    printf("P%d | %-10s | %.0f pts\n", data->mclaren.norris.position,  data->mclaren.norris.name,  data->mclaren.norris.points);
    printf("P%d | %-10s | %.0f pts\n", data->mclaren.piastri.position, data->mclaren.piastri.name, data->mclaren.piastri.points);

    printf("\n===== Ferrari =====\n");
    printf("P%d | %-10s | %.0f pts\n", data->ferrari.leclerc.position,  data->ferrari.leclerc.name,  data->ferrari.leclerc.points);
    printf("P%d | %-10s | %.0f pts\n", data->ferrari.hamilton.position, data->ferrari.hamilton.name, data->ferrari.hamilton.points);
    printf("===================\n");
}