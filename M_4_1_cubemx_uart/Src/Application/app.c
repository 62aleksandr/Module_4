#include "app.h"

uint8_t msg[MESSAGE_SIZE] = {0};
uint8_t rxData[UART_BUFFER_SIZE];

void app_init(uart_context *uart_ctx, UART_HandleTypeDef *huart)
{
	uart_init(uart_ctx, huart);
}

void app_run(uart_context *uart_ctx)
{
	if (uart_receive(uart_ctx,
					 rxData,
					 UART_BUFFER_SIZE,
					 0) == HAL_OK)
	{
		uart_send(uart_ctx,
				  rxData,
				  UART_BUFFER_SIZE);

		led_toggle();
	}
}