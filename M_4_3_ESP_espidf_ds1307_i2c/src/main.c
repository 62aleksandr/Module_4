// https://components.espressif.com/components/esp-idf-lib/ds1307/versions/1.0.7/readme?language=en

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ds1307.h>
#include <string.h>
#include <esp_log.h>

#define I2C_PORT I2C_NUM_0
#define I2C_MASTER_SDA_IO 16
#define I2C_MASTER_SCL_IO 15

static const char *TAG = "MAIN_RTC";

// Ініціалізація поточеого часу
static void rtc_set_time(struct tm *time)
{

    static const char *months[] =
        {
            "Jan", "Feb", "Mar", "Apr",
            "May", "Jun", "Jul", "Aug",
            "Sep", "Oct", "Nov", "Dec"};

    char month[4];
    int year;
    memset(time, 0, sizeof(struct tm));

    sscanf(__DATE__, "%3s %d %d",
           month,
           &time->tm_mday,
           &year);

    time->tm_year = year - 1900;

    for (int i = 0; i < 12; i++)
    {
        if (strcmp(month, months[i]) == 0)
        {
            time->tm_mon = i;
            break;
        }
    }

    sscanf(__TIME__, "%d:%d:%d",
           &time->tm_hour,
           &time->tm_min,
           &time->tm_sec);
}

void app_main()
{
    i2c_dev_t dev;
    memset(&dev, 0, sizeof(i2c_dev_t));
    struct tm time;
    esp_err_t err;

    // Ініціалізація I2C
    err = i2cdev_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "i2cdev_init() failed: %s", esp_err_to_name(err));
    }

    // Створення та ініціалізація дескриптора DS1307
    err = ds1307_init_desc(&dev, I2C_PORT, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "ds1307_init_desc: %s", esp_err_to_name(err));
        return;
    }

    // Установка времени на RTC
    rtc_set_time(&time);
    err = ds1307_set_time(&dev, &time);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "ds1307_set_time: %s", esp_err_to_name(err));
        return;
    }

    while (1)
    {
        // Читання часу
        err = ds1307_get_time(&dev, &time);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "ds1307_get_time: %s", esp_err_to_name(err));
            return;
        }

        ESP_LOGI(TAG,
                 "%04d-%02d-%02d %02d:%02d:%02d\n",
                 time.tm_year + 1900 /*Add 1900 for better readability*/,
                 time.tm_mon + 1,
                 time.tm_mday,
                 time.tm_hour,
                 time.tm_min,
                 time.tm_sec);

        // Затримка на 500 мс
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
