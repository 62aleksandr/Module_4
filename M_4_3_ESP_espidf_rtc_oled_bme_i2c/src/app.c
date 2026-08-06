#include "i2cdev.h"
#include "ssd1306.h"
#include "ds1307.h"
#include "bmp280.h"
#include "app.h"

static ssd1306_t oled_dev;
static i2c_dev_t rtc_dev;
static bmp280_t bmp_dev;

// Конфігурація I2C ESP32-S3
static const gpio_num_t I2C_PORT = I2C_NUM_0;
static const gpio_num_t I2C_SDA_GPIO = GPIO_NUM_16;
static const gpio_num_t I2C_SCL_GPIO = GPIO_NUM_15;

static app_data_t app_data;

// Функция возвращает адрес (указатель) на эту переменную
app_data_t *get_app_data(void)
{
	return &app_data;
}

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

// Iніціалізацію бібліотеки i2cdev
esp_err_t i2c_init(void)
{
	// ----- Init i2cdev -----
	esp_err_t err = i2cdev_init();
	if (err != ESP_OK)
	{
		return err;
	}

	return ESP_OK;
}

// Ініціалізація OLED-дисплея
esp_err_t oled_dev_init()

{
	memset(&oled_dev, 0, sizeof(ssd1306_t));

	esp_err_t first_err = ESP_OK;

	esp_err_t err = ssd1306_init_desc(&oled_dev,
									  I2C_PORT,
									  I2C_SDA_GPIO,
									  I2C_SCL_GPIO);
	if (first_err == ESP_OK && err != ESP_OK)
	{
		first_err = err;
	}

	err = ssd1306_init_display(&oled_dev);
	if (first_err == ESP_OK && err != ESP_OK)
	{
		first_err = err;
	}

	ssd1306_clear(&oled_dev);

	return first_err;
}

// Ініціалізація RTC
esp_err_t rtc_dev_init(app_data_t *app_data)
{
	esp_err_t first_err = ESP_OK;

	memset(&rtc_dev, 0, sizeof(i2c_dev_t));

	esp_err_t err = ds1307_init_desc(&rtc_dev,
									 I2C_PORT,
									 I2C_SDA_GPIO,
									 I2C_SCL_GPIO);
	if (first_err == ESP_OK && err != ESP_OK)
	{
		first_err = err;
	}

	rtc_set_time(&app_data->time);

	err = ds1307_set_time(&rtc_dev, &app_data->time);
	if (first_err == ESP_OK && err != ESP_OK)
	{
		first_err = err;
	}

	return first_err;
}

// Ініціалізація BMP280
esp_err_t bmp280_dev_init()
{
	esp_err_t first_err = ESP_OK;

	memset(&bmp_dev, 0, sizeof(bmp280_t));

	esp_err_t err = bmp280_init_desc(&bmp_dev,
									 BMP280_I2C_ADDRESS_0,
									 I2C_PORT,
									 I2C_SDA_GPIO,
									 I2C_SCL_GPIO);

	if (first_err == ESP_OK && err != ESP_OK)
	{
		first_err = err;
	}

	bmp280_params_t bmp_params;
	bmp280_init_default_params(&bmp_params);

	err = bmp280_init(&bmp_dev, &bmp_params);
	if (first_err == ESP_OK && err != ESP_OK)
	{
		first_err = err;
	}

	return first_err;
}

// Читання часу з RTC
esp_err_t rtc_read(app_data_t *app_data)
{
	esp_err_t err = ds1307_get_time(&rtc_dev, &app_data->time);
	if (err != ESP_OK)
	{
		return err;
	}

	return ESP_OK;
}
// Читання даних з BME280
esp_err_t bme280_read(app_data_t *app_data)
{
	esp_err_t err = bmp280_read_float(&bmp_dev, &app_data->bme280_data.temperature, &app_data->bme280_data.pressure, &app_data->bme280_data.humidity);
	if (err != ESP_OK)
	{
		return err;
	}

	return ESP_OK;
}
// Оновлення OLED-дисплея
void oled_update(app_data_t *app_data)
{
	char text_buf[32] = {0};

	ssd1306_draw_string(&oled_dev, 5, 1, text_buf);

	// Дата
	snprintf(text_buf, sizeof(text_buf),
			 "%02d.%02d.%04d",
			 app_data->time.tm_mday,
			 app_data->time.tm_mon + 1,
			 app_data->time.tm_year + 1900);

	ssd1306_draw_string(&oled_dev, 5, 1, text_buf);

	// Час
	snprintf(text_buf, sizeof(text_buf),
			 "%02d:%02d:%02d",
			 app_data->time.tm_hour,
			 app_data->time.tm_min,
			 app_data->time.tm_sec);

	ssd1306_draw_string(&oled_dev, 5, 2, text_buf);

	// Температура
	snprintf(text_buf, sizeof(text_buf),
			 "T: %.1f C",
			 app_data->bme280_data.temperature);

	ssd1306_draw_string(&oled_dev, 5, 4, text_buf);

	// Вологість
	snprintf(text_buf, sizeof(text_buf),
			 "H: %.1f %%",
			 app_data->bme280_data.humidity);

	ssd1306_draw_string(&oled_dev, 5, 5, text_buf);

	// Тиск
	snprintf(text_buf, sizeof(text_buf),
			 "P: %.1f hPa",
			 app_data->bme280_data.pressure);

	ssd1306_draw_string(&oled_dev, 5, 6, text_buf);
}
