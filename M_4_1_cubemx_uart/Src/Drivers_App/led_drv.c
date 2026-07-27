#include "led_drv.h"
#include "stm32f4xx_hal.h"

void led_toggle(void)
{
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}