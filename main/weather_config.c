#include "globalVar.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"

orbit_weather_data_t g_weather_data;


static const char *TAG = "ORBIT_WEATHER";

static char weather_buf[4096];
static int  weather_buf_len = 0;

static esp_err_t _weather_http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ON_CONNECTED:
            weather_buf_len = 0;
            memset(weather_buf, 0, sizeof(weather_buf));
            break;

        case HTTP_EVENT_ON_DATA:
            if(weather_buf_len + evt->data_len < sizeof(weather_buf)) {
                memcpy(weather_buf + weather_buf_len, evt->data, evt->data_len);
                weather_buf_len += evt->data_len;
            }
            break;

        default:
            break;
    }

    return ESP_OK;
}

static const char* _weather_code_to_text(int code)
{
    switch(code) {
        case 0:  return "Clear";
        case 1:
        case 2:
        case 3:  return "Cloudy";

        case 45:
        case 48: return "Fog";

        case 51:
        case 53:
        case 55: return "Drizzle";

        case 61:
        case 63:
        case 65: return "Rain";

        case 71:
        case 73:
        case 75: return "Snow";

        case 80:
        case 81:
        case 82: return "Showers";

        case 95:
        case 96:
        case 99: return "Storm";

        default: return "Weather";
    }
}

static const char* _date_to_day_short(const char *date)
{
    // Lazy/simple placeholder.
    // Open-Meteo returns YYYY-MM-DD.
    // For now just rotate labels if you do not want full date math.
    return "DAY";
}

static orbit_err_t weather_parse(orbit_weather_data_t *out)
{
    cJSON *root = cJSON_Parse(weather_buf);
    if(root == NULL) {
        out->valid = 0;
        return ORBIT_ERR;
    }

    cJSON *current = cJSON_GetObjectItem(root, "current");
    cJSON *daily   = cJSON_GetObjectItem(root, "daily");

    if(!current || !daily) {
        cJSON_Delete(root);
        out->valid = 0;
        return ORBIT_ERR;
    }

    cJSON *temp_item  = cJSON_GetObjectItem(current, "temperature_2m");
    cJSON *feels_item = cJSON_GetObjectItem(current, "apparent_temperature");
    cJSON *code_item  = cJSON_GetObjectItem(current, "weather_code");

    if(!temp_item || !feels_item || !code_item) {
        cJSON_Delete(root);
        out->valid = 0;
        return ORBIT_ERR;
    }

    out->temp_c       = (int)temp_item->valuedouble;
    out->feels_c      = (int)feels_item->valuedouble;
    out->weather_code = code_item->valueint;

    snprintf(out->condition, sizeof(out->condition), "%s",
             _weather_code_to_text(out->weather_code));

    cJSON *daily_temps = cJSON_GetObjectItem(daily, "temperature_2m_max");

    if(daily_temps && cJSON_IsArray(daily_temps)) {
        const char *labels[3] = {"TOD", "TMR", "DAY"};

        for(int i = 0; i < 3; i++) {
            cJSON *temp = cJSON_GetArrayItem(daily_temps, i);

            snprintf(out->forecast[i].day,
                     sizeof(out->forecast[i].day),
                     "%s",
                     labels[i]);

            if(temp) {
                out->forecast[i].temp_max = (int)temp->valuedouble;
            }
        }
    }

    out->valid = 1;

    cJSON_Delete(root);
    return ORBIT_OK;
}

orbit_err_t weather_fetch(orbit_weather_data_t *out)
{
    memset(out, 0, sizeof(orbit_weather_data_t));

    snprintf(out->city, sizeof(out->city), "Waukesha");

    esp_http_client_config_t config = {
        .url                         = WEATHER_URL,
        .event_handler               = _weather_http_event_handler,
        .crt_bundle_attach           = esp_crt_bundle_attach,
        .skip_cert_common_name_check = true,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);
    esp_http_client_cleanup(client);

    if(err != ESP_OK) {
        printf("weather fetch failed: %s\n", esp_err_to_name(err));
        out->valid = 0;
        return ORBIT_ERR;
    }

    return weather_parse(out);
}

void weather_print(orbit_weather_data_t *data)
{
    printf("\n===== WEATHER =====\n");

    if(!data->valid) {
        printf("No weather data\n");
        return;
    }

    printf("City: %s\n", data->city);
    printf("Temp: %d C\n", data->temp_c);
    printf("Feels: %d C\n", data->feels_c);
    printf("Condition: %s\n", data->condition);

    for(int i = 0; i < 3; i++) {
        printf("%s: %d C\n", data->forecast[i].day, data->forecast[i].temp_max);
    }

    printf("===================\n");
}