#pragma once

#include "uart_drv.h"

#define MESSAGE_SIZE 48
#define UART_BUFFER_SIZE 1 // Розмір буфера для прийому даних з UART

void app_init(uart_context *uart_ctx, UART_HandleTypeDef *huart);
void app_run(uart_context *uart_ctx);
