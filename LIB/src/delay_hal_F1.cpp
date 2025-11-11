#include "delay.h"
#include "stm32f1xx_hal.h"

uint32_t freq_MZ = 0;

void Delay_us(uint32_t time)
{
	const uint32_t stop_value = DWT->CYCCNT + time * freq_MZ - 30;
	while ((DWT->CYCCNT - stop_value ) & 0x80000000);
}

void delay_system_init()
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // разрешаем использовать счётчик
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;   // запускаем счётчик
	freq_MZ = SystemCoreClock / 1000000U;
}
