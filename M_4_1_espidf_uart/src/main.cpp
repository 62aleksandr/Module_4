#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "uart.h"

static constexpr uart_port_t UART1_PORT = UART_NUM_1;
static constexpr gpio_num_t UART1_TX_PIN = GPIO_NUM_17;
static constexpr gpio_num_t UART1_RX_PIN = GPIO_NUM_18;
static constexpr uint32_t UART1_BAUD_RATE = 9600;
static constexpr uart_word_length_t UART1_DATA_BITS = UART_DATA_8_BITS;
static constexpr uart_parity_t UART1_PARITY_MODE = UART_PARITY_DISABLE;
static constexpr uart_stop_bits_t UART1_STOP_BITS_MODE = UART_STOP_BITS_1;
static constexpr uart_hw_flowcontrol_t UART1_FLOW_CONTROL = UART_HW_FLOWCTRL_DISABLE;
static constexpr int UART1_RX_BUFFER_SIZE = 1024;
static constexpr int UART1_TX_BUFFER_SIZE = 256;

static constexpr gpio_num_t LED_GPIO = GPIO_NUM_15;

static const char *TAG = "UART";

extern "C" void app_main(void)
{
    //------------ UART CONTEXT Init ---------------

    uart_context uart1_ctx = {};

    uart1_ctx.uart_num = UART1_PORT;
    uart1_ctx.tx_pin = UART1_TX_PIN;
    uart1_ctx.rx_pin = UART1_RX_PIN;
    uart1_ctx.baud_rate = UART1_BAUD_RATE;
    uart1_ctx.data_bits = UART1_DATA_BITS;
    uart1_ctx.parity = UART1_PARITY_MODE;
    uart1_ctx.stop_bits = UART1_STOP_BITS_MODE;
    uart1_ctx.flow_ctrl = UART1_FLOW_CONTROL;
    uart1_ctx.rx_buffer_size = UART1_RX_BUFFER_SIZE;
    uart1_ctx.tx_buffer_size = UART1_TX_BUFFER_SIZE;
    uart1_ctx.initialized = false;

    ESP_ERROR_CHECK(uart_init(&uart1_ctx));

    //-------------Ініціалізація GPIO LED-------------
    gpio_config_t led_conf = {};
    led_conf.pin_bit_mask = (1ULL << LED_GPIO);
    led_conf.mode = GPIO_MODE_OUTPUT;
    led_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    led_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    led_conf.intr_type = GPIO_INTR_DISABLE;

    ESP_ERROR_CHECK(gpio_config(&led_conf));

    static bool led_state = false; // Змінна стану
    esp_err_t err = ESP_OK;

    while (1)
    {
        // Перевірка наявності данних UART1
        size_t length = 0;
        err = uart_available(&uart1_ctx, &length);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "uart_available: %s",
                     esp_err_to_name(err));
        }
        else if (length > 0)
        {
            uint8_t rx_data;
            size_t received = 0;

            err = uart_receive(&uart1_ctx,
                               &rx_data,
                               1,
                               pdMS_TO_TICKS(1),
                               &received);

            if (err == ESP_OK && received == 1)
            {
                ESP_LOGI(TAG,
                         "Received UART1: 0x%02X",
                         rx_data);

                led_state = !led_state;
                gpio_set_level(LED_GPIO, led_state);
            }
            else
            {
                ESP_LOGE(TAG,
                         "uart_receive: %s",
                         esp_err_to_name(err));
            }
        }

        // Приймання команди з Serial Monitor
        int cmd = getchar();

        if (cmd != EOF)
        {
            uint8_t tx_data = (uint8_t)cmd;
            ESP_LOGI(TAG, "Received from Serial Monitor: 0x%02X", tx_data);

            // Передача команди в UART1
            err = uart_send(&uart1_ctx, &tx_data, 1);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, " uart_send: %s", esp_err_to_name(err));
            }
        }

        // Затримка цикла
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
