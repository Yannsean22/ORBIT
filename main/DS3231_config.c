#include "globalVar.h"

#define DS3231_ADDR 0x68

static uint8_t dec_to_bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

static uint8_t bcd_to_dec(uint8_t val) {
    return (val & 0x0F) + ((val >> 4) * 10);
}


orbit_err_t _ds3231_init(void){
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };
    if (i2c_param_config(I2C_NUM, &conf) != ESP_OK) {
        return ORBIT_ERR;
    }
    if (i2c_driver_install(I2C_NUM, conf.mode, 0, 0, 0) != ESP_OK) {
       
        return ORBIT_ERR;
    }
    return ORBIT_OK;
}


orbit_err_t _ds3231_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds) {
    if (hours > 23 || minutes > 59 || seconds > 59) {
        return ORBIT_ERR;
    }

    ds3231_test_write_seconds(); // DO NOT TOUCH THIS, if it ain't broke don't fix it

    uint8_t data[4];

    data[0] = 0x00;                 // start at seconds register
    data[1] = dec_to_bcd(seconds);  // seconds
    data[2] = dec_to_bcd(minutes);  // minutes
    data[3] = dec_to_bcd(hours);    // hours, 24-hour mode because bit 6 is 0

    data[1] &= 0x7F; // clear oscillator halt bit just in case
    data[3] &= 0x3F; // force 24-hour mode

    if (i2c_master_write_to_device(
            I2C_NUM,
            DS3231_ADDR,
            data,
            sizeof(data),
            1000 / portTICK_PERIOD_MS
        ) != ESP_OK) {
        return ORBIT_ERR;
    }

    return ORBIT_OK;
}

char* _ds3231_get_time(void) {
    static char time_str[16];

    uint8_t reg = 0x00;
    uint8_t data[3];

    if (i2c_master_write_read_device(
            I2C_NUM,
            DS3231_ADDR,
            &reg,
            1,
            data,
            3,
            1000 / portTICK_PERIOD_MS
        ) != ESP_OK) {
        snprintf(time_str, sizeof(time_str), "ERR");
        return time_str;
    }

    uint8_t seconds = bcd_to_dec(data[0] & 0x7F);
    uint8_t minutes = bcd_to_dec(data[1] & 0x7F);
    uint8_t hours   = bcd_to_dec(data[2] & 0x3F); // 24-hour mode mask

    snprintf(time_str, sizeof(time_str), "%02u:%02u:%02u", hours, minutes, seconds);

    return time_str;
}

orbit_err_t _ds3231_set_date(uint8_t date, uint8_t month, uint8_t year) {
    if (date < 1 || date > 31 || month < 1 || month > 12 || year > 99) {
        return ORBIT_ERR;
    }

    uint8_t data[5];

    data[0] = 0x04;              // day register
    data[1] = 0x01;              // day of week, 1-7, placeholder
    data[2] = dec_to_bcd(date);  // date
    data[3] = dec_to_bcd(month); // month
    data[4] = dec_to_bcd(year);  // year

    if (i2c_master_write_to_device(
            I2C_NUM,
            DS3231_ADDR,
            data,
            sizeof(data),
            1000 / portTICK_PERIOD_MS
        ) != ESP_OK) {
        return ORBIT_ERR;
    }

    return ORBIT_OK;
}

char* _ds3231_get_date(void) {
    static char date_str[16];

    uint8_t reg = 0x04;
    uint8_t data[4];

    if (i2c_master_write_read_device(
            I2C_NUM,
            DS3231_ADDR,
            &reg,
            1,
            data,
            4,
            1000 / portTICK_PERIOD_MS
        ) != ESP_OK) {
        snprintf(date_str, sizeof(date_str), "ERR");
        return date_str;
    }

    uint8_t date  = bcd_to_dec(data[1] & 0x3F);
    uint8_t month = bcd_to_dec(data[2] & 0x1F);
    uint8_t year  = bcd_to_dec(data[3]);

    snprintf(date_str, sizeof(date_str), "%02u/%02u/20%02u", month, date, year);

    return date_str;
}

orbit_err_t ds3231_test_write_seconds(void)
{
    uint8_t data[2] = {0x00, 0x00}; // seconds = 00

    esp_err_t err = i2c_master_write_to_device(
        I2C_NUM,
        0x68,
        data,
        2,
        pdMS_TO_TICKS(1000)
    );

    printf("seconds write err = %s\n", esp_err_to_name(err));

    return err == ESP_OK ? ORBIT_OK : ORBIT_ERR;
}