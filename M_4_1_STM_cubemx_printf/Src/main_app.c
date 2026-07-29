#include <string.h>
#include <stdio.h>
#include "main.h"

extern UART_HandleTypeDef huart2;

int _write(int file, char *ptr, int len)
{
	HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);
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
