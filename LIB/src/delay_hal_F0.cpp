#include "delay.h"
#include "stm32f0xx_hal.h"
//#include <stdint.h>

uint32_t freq_MZ = 0;

void Delay_us(uint32_t time)
{
    int32_t old_value = SysTick->VAL;
    // const uint32_t stop_value = SysTick->VAL + time * freq_MZ - 30;
	const int32_t stop = time * freq_MZ - 80;
    int32_t cnt = 0;
    while (cnt < stop)
    {
        int32_t diff = old_value - (int32_t)(SysTick->VAL);
        old_value -= diff;
        if (diff < 0)
            diff += SysTick->LOAD;
        cnt += diff;
    }
}

void delay_system_init()
{
    freq_MZ = SystemCoreClock / 1000000U;
}

__weak void Delay_ms(uint32_t time)
{
    HAL_Delay(time);
}
