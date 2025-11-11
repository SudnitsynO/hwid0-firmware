#include "gpio_stm32f0.h"

ExtiIsr _delegates[16];

extern "C" void EXTI0_1_IRQHandler()
{
    if (EXTI->PR & EXTI_PR_PR0)
    {
        EXTI->PR = EXTI_PR_PR0;
        _delegates[0]();
    }
    if (EXTI->PR & EXTI_PR_PR1)
    {
        EXTI->PR = EXTI_PR_PR1;
        _delegates[1]();
    }
}

extern "C" void EXTI2_3_IRQHandler()
{
    if (EXTI->PR & EXTI_PR_PR2)
    {
        EXTI->PR = EXTI_PR_PR2;
        _delegates[2]();
    }
    if (EXTI->PR & EXTI_PR_PR3)
    {
        EXTI->PR = EXTI_PR_PR3;
        _delegates[3]();
    }
}

extern "C" void EXTI4_15_IRQHandler()
{
    if (EXTI->PR & EXTI_PR_PR4)
    {
        EXTI->PR = EXTI_PR_PR4;
        _delegates[4]();
    }
    if (EXTI->PR & EXTI_PR_PR5)
    {
        EXTI->PR = EXTI_PR_PR5;
        _delegates[5]();
    }
    if (EXTI->PR & EXTI_PR_PR6)
    {
        EXTI->PR = EXTI_PR_PR6;
        _delegates[6]();
    }
    if (EXTI->PR & EXTI_PR_PR7)
    {
        EXTI->PR = EXTI_PR_PR7;
        _delegates[7]();
    }
    if (EXTI->PR & EXTI_PR_PR8)
    {
        EXTI->PR = EXTI_PR_PR8;
        _delegates[8]();
    }
    if (EXTI->PR & EXTI_PR_PR9)
    {
        EXTI->PR = EXTI_PR_PR9;
        _delegates[9]();
    }
    if (EXTI->PR & EXTI_PR_PR10)
    {
        EXTI->PR = EXTI_PR_PR10;
        _delegates[10]();
    }
    if (EXTI->PR & EXTI_PR_PR11)
    {
        EXTI->PR = EXTI_PR_PR11;
        _delegates[11]();
    }
    if (EXTI->PR & EXTI_PR_PR12)
    {
        EXTI->PR = EXTI_PR_PR12;
        _delegates[12]();
    }
    if (EXTI->PR & EXTI_PR_PR13)
    {
        EXTI->PR = EXTI_PR_PR13;
        _delegates[13]();
    }
    if (EXTI->PR & EXTI_PR_PR14)
    {
        EXTI->PR = EXTI_PR_PR14;
        _delegates[14]();
    }
    if (EXTI->PR & EXTI_PR_PR15)
    {
        EXTI->PR = EXTI_PR_PR15;
        _delegates[15]();
    }
}