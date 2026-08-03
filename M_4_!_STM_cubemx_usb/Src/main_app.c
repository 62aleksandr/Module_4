#include <string.h>
#include <stdio.h>
#include "usbd_cdc_if.h"
#include "main.h"

// extern UART_HandleTypeDef huart2;

int _write(int file, char *ptr, int len)
{
	while (CDC_Transmit_FS((uint8_t *)ptr, len) == USBD_BUSY)
	{
		HAL_Delay(1);
	}
	return len;
}

void main_app()
{

	while (1)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
		printf("LED ON\r\n");

		HAL_Delay(1000);

		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		printf("LED OFF\r\n");

		HAL_Delay(1000);
	}
}
