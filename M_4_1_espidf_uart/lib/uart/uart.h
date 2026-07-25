#pragma once

#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_err.h"

typedef struct
{
	uart_port_t uart_num;			 // Номер UART_NUM_0 (1, 2)
	gpio_num_t tx_pin;				 // Номер GPIO TX (Transmit)
	gpio_num_t rx_pin;				 // Номер GPIO RX (Receive)
	uint32_t baud_rate;				 // Швидкість (9600, 115200)
	uart_word_length_t data_bits;	 // UART_DATA_5_BITS (6,7,8)
	uart_parity_t parity;			 // UART_PARITY_DISABLE (EVEN, ODD)
	uart_stop_bits_t stop_bits;		 // UART_STOP_BITS_1 (1_5, 2)
	uart_hw_flowcontrol_t flow_ctrl; // UART_HW_FLOWCTRL_DISABLE (RTS, CTS, CTS_RTS)
	int rx_buffer_size;				 // Розмір RX-буфера, байт
	int tx_buffer_size;				 // Розмір TX-буфера, байт
	bool initialized;				 // stan ініціалізації UART

} uart_context;

// API
esp_err_t uart_init(uart_context *ctx);

esp_err_t uart_receive(uart_context *ctx,
					   uint8_t *buffer,
					   size_t length,
					   TickType_t timeout,
					   size_t *received);

esp_err_t uart_send(uart_context *ctx,
					const uint8_t *data,
					size_t length);

esp_err_t uart_available(uart_context *ctx,
						 size_t *length);

esp_err_t uart_deinit(uart_context *ctx);
