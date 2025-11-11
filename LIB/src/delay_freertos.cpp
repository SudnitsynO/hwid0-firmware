#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"

#if defined (STM32F030x6)
#include "stm32f0xx_hal.h"
#endif

#if defined(STM32F103xB) || defined(STM32F103xE)
#include "stm32f1xx_hal.h"
#endif

#if defined(STM32F407xx)
#include "stm32f4xx_hal.h"
#endif


void Delay_ms(uint32_t time)
{
	vTaskDelay(time);
}
