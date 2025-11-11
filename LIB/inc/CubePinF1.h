#pragma once
#include "main.h"
#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "gpio_interface.h"
#include "gpio_stm32f1.h"

/*
#define CUBE_PIN(Port,Pin,Name) \
	struct Name\
    {\
	static void Toggle()\
	{\
		HAL_GPIO_TogglePin(Port, Pin);\
	}\
	static void Set()\
	{\
		HAL_GPIO_WritePin(Port, Pin,GPIO_PIN_SET);\
	}\
	static void Reset()\
	{\
		HAL_GPIO_WritePin(Port, Pin,GPIO_PIN_RESET);\
	}\
    }

	*/

#define CUBE_PORT(PIN_NAME) PIN_NAME##_GPIO_Port

constexpr int cube_pin_number(uint16_t cube_pin)
{
    switch (cube_pin) {
        case GPIO_PIN_0 : return 0;
        case GPIO_PIN_1 : return 1;
        case GPIO_PIN_2 : return 2;
        case GPIO_PIN_3 : return 3;
        case GPIO_PIN_4 : return 4;
        case GPIO_PIN_5 : return 5;
        case GPIO_PIN_6 : return 6;
        case GPIO_PIN_7 : return 7;
        case GPIO_PIN_8 : return 8;
        case GPIO_PIN_9 : return 9;
        case GPIO_PIN_10 : return 10;
        case GPIO_PIN_11 : return 11;
        case GPIO_PIN_12 : return 12;
        case GPIO_PIN_13 : return 13;
        case GPIO_PIN_14 : return 14;
        case GPIO_PIN_15 : return 15;
    }
    return 0;
}

#define CUBE_PIN(PIN_NAME)\
typedef GpioPort<PIN_NAME##_GPIO_Port> PIN_NAME##_PORT;\
typedef Pin<PIN_NAME##_PORT,cube_pin_number(PIN_NAME##_Pin)> PIN_NAME

#pragma push_macro("GPIOA")
#pragma push_macro("GPIOB")
#pragma push_macro("GPIOC")
#pragma push_macro("GPIOD")
#pragma push_macro("GPIOE")
#pragma push_macro("GPIOF")
#pragma push_macro("GPIOG")
#pragma push_macro("GPIOH")

#undef GPIOA
#define GPIOA GPIOA_BASE
#undef GPIOB
#define GPIOB GPIOB_BASE
#undef GPIOC
#define GPIOC GPIOC_BASE
#undef GPIOD
#define GPIOD GPIOD_BASE
#undef GPIOE
#define GPIOE GPIOE_BASE
#undef GPIOF
#define GPIOF GPIOF_BASE
#undef GPIOG
#define GPIOG GPIOG_BASE
#undef GPIOH
#define GPIOH GPIOH_BASE

#include "CubePinList.h"

#pragma pop_macro("GPIOA")
#pragma pop_macro("GPIOB")
#pragma pop_macro("GPIOC")
#pragma pop_macro("GPIOD")
#pragma pop_macro("GPIOE")
#pragma pop_macro("GPIOF")
#pragma pop_macro("GPIOG")
#pragma pop_macro("GPIOH")