#pragma once

#include "stm32f1xx_hal.h"
#include <stdint.h>
//#include "stm32f1xx_hal.h"
#include "gpio_interface.h"
#include <functional>
/*
enum PortIndex
{
    PortAIndex,
    PortBIndex,
    PortCIndex,
    PortDIndex,
    PortEIndex,
    PortFIndex,
    PortGIndex,
    PortHIndex
};
*/
static ExtiIsr _delegates[16];

template <size_t GPIO_BASE> class GpioPort
{
  public:
    static void Write(uint16_t value);
    static uint16_t Read();
    static void SetBits(uint16_t mask);
    static void ResetBits(uint16_t mask);
    static void ToggleBits(uint16_t mask);
    static bool ReadBit(int bit_index);
    // static void Update();		//не реализовано аппаратно
    static void SetMode(Mode mode, uint16_t mask = 0xFFFF, uint16_t init_value = 0);
    static void Init();
    static void DeInit();
    static void EnableInterrupt(uint16_t PinNumber, InterruptMode mode, uint32_t interrupt_priority);
    static void DisableInterrupt(uint16_t PinNumber);
	static ExtiIsr & GetIstDelegate(uint16_t PinNumber) ;

  private:
    static constexpr GPIO_TypeDef *GetPortBaseAddress();
    static void EnableEextiIntBit(uint16_t PinNumber, InterruptMode mode);
    static constexpr uint32_t EXTIx_bits();
	
};

template <size_t GPIO_BASE> 
ExtiIsr & GpioPort<GPIO_BASE>::GetIstDelegate(uint16_t PinNumber)
{
	return _delegates[PinNumber];
}

template <size_t GPIO_BASE> constexpr uint32_t GpioPort<GPIO_BASE>::EXTIx_bits()
{
    switch (GPIO_BASE)
    {
    case GPIOA_BASE:
        return 0;
        break;
    case GPIOB_BASE:
        return 1;
        break;
    case GPIOC_BASE:
        return 2;
        break;
    case GPIOD_BASE:
        return 3;
        break;
    case GPIOE_BASE:
        return 4;
        break;
    case GPIOF_BASE:
        return 5;
        break;
    case GPIOG_BASE:
        return 6;
        break;
#ifdef GPIOH
    case GPIOH_BASE:
        return 7;
        break;
#endif
    default:
        return 15;
    }
}

extern "C" void EXTI0_IRQHandler()
{
    if (EXTI->PR & EXTI_PR_PR0)
    {
        EXTI->PR = EXTI_PR_PR0;
        _delegates[0]();
        // if(_callback_functions[0])
        //_callback_functions[0](contexties[0]);
    }
}

extern "C" void EXTI1_IRQHandler()
{
    if (EXTI->PR & EXTI_PR_PR1)
    {
        EXTI->PR = EXTI_PR_PR1;
        _delegates[1]();
        // if (_callback_functions[1])
        //_callback_functions[1](contexties[1]);
    }
}

extern "C" void EXTI2_IRQHandler()
{
    if (EXTI->PR & EXTI_PR_PR2)
    {
        EXTI->PR = EXTI_PR_PR2;
        _delegates[2]();
        // if (_callback_functions[2])
        //_callback_functions[2](contexties[2]);
    }
}
extern "C" void EXTI3_IRQHandler()
{
    if (EXTI->PR & EXTI_PR_PR3)
    {
        EXTI->PR = EXTI_PR_PR3;
        _delegates[3]();
        // if (_callback_functions[3])
        //	_callback_functions[3](contexties[3]);
    }
}
extern "C" void EXTI4_IRQHandler()
{
    if (EXTI->PR & EXTI_PR_PR4)
    {
        EXTI->PR = EXTI_PR_PR4;
        _delegates[4]();
        // if (_callback_functions[4])
        //	_callback_functions[4](contexties[4]);
    }
}
extern "C" void EXTI9_5_IRQHandler()
{
    if (EXTI->PR & EXTI_PR_PR5)
    {
        EXTI->PR = EXTI_PR_PR5;
        _delegates[5]();
        // if (_callback_functions[5])
        //	_callback_functions[5](contexties[5]);
    }
    if (EXTI->PR & EXTI_PR_PR6)
    {
        EXTI->PR = EXTI_PR_PR6;
        _delegates[6]();
        // if (_callback_functions[6])
        //    _callback_functions[6](contexties[6]);
    }
    if (EXTI->PR & EXTI_PR_PR7)
    {
        EXTI->PR = EXTI_PR_PR7;
        _delegates[7]();
        // if (_callback_functions[7])
        //    _callback_functions[7](contexties[7]);
    }
    if (EXTI->PR & EXTI_PR_PR8)
    {
        EXTI->PR = EXTI_PR_PR8;
        _delegates[8]();
        // if (_callback_functions[8])
        //    _callback_functions[8](contexties[8]);
    }
    if (EXTI->PR & EXTI_PR_PR9)
    {
        EXTI->PR = EXTI_PR_PR9;
        _delegates[9]();
        // if (_callback_functions[9])
        //    _callback_functions[9](contexties[9]);
    }
}
extern "C" void EXTI15_10_IRQHandler()
{
    if (EXTI->PR & EXTI_PR_PR10)
    {
        EXTI->PR = EXTI_PR_PR10;
        _delegates[10]();
        // if (_callback_functions[10])
        //    _callback_functions[10](contexties[10]);
    }
    if (EXTI->PR & EXTI_PR_PR11)
    {
        EXTI->PR = EXTI_PR_PR11;
        _delegates[11]();
        // if (_callback_functions[11])
        //    _callback_functions[11](contexties[11]);
    }
    if (EXTI->PR & EXTI_PR_PR12)
    {
        EXTI->PR = EXTI_PR_PR12;
        _delegates[12]();
        // if (_callback_functions[12])
        //_callback_functions[12](contexties[12]);
    }
    if (EXTI->PR & EXTI_PR_PR13)
    {
        EXTI->PR = EXTI_PR_PR13;
        _delegates[13]();

        // if (_callback_functions[13])
        //   _callback_functions[13](contexties[13]);
    }
    if (EXTI->PR & EXTI_PR_PR14)
    {
        EXTI->PR = EXTI_PR_PR14;
        _delegates[14]();
        // if (_callback_functions[14])
        //    _callback_functions[14](contexties[14]);
    }
    if (EXTI->PR & EXTI_PR_PR15)
    {
        EXTI->PR = EXTI_PR_PR15;
        _delegates[15]();
        // if (_callback_functions[15])
        //    _callback_functions[15](contexties[15]);
    }
}

template <> inline void GpioPort<GPIOA_BASE>::Init()
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

template <> inline void GpioPort<GPIOB_BASE>::Init()
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
}

template <> inline void GpioPort<GPIOC_BASE>::Init()
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
}

template <> inline void GpioPort<GPIOD_BASE>::Init()
{
    __HAL_RCC_GPIOD_CLK_ENABLE();
}

template <> inline void GpioPort<GPIOE_BASE>::Init()
{
    __HAL_RCC_GPIOE_CLK_ENABLE();
}
#ifdef GPIOF
template <> inline void GpioPort<GPIOF_BASE>::Init()
{
    __HAL_RCC_GPIOF_CLK_ENABLE();
}
#endif

#ifdef GPIOG
template <> inline void GpioPort<GPIOG_BASE>::Init()
{
    __HAL_RCC_GPIOG_CLK_ENABLE();
}
#endif

template <> inline void GpioPort<GPIOA_BASE>::DeInit()
{
    GpioPort<GPIOA_BASE>::SetMode(Mode::Input);
    __HAL_RCC_GPIOA_CLK_DISABLE();
}

template <> inline void GpioPort<GPIOB_BASE>::DeInit()
{
    GpioPort<GPIOB_BASE>::SetMode(Mode::Input);
    __HAL_RCC_GPIOB_CLK_DISABLE();
}

template <> inline void GpioPort<GPIOC_BASE>::DeInit()
{
    GpioPort<GPIOC_BASE>::SetMode(Mode::Input);
    __HAL_RCC_GPIOC_CLK_DISABLE();
}

template <> inline void GpioPort<GPIOD_BASE>::DeInit()
{
    GpioPort<GPIOD_BASE>::SetMode(Mode::Input);
    __HAL_RCC_GPIOD_CLK_DISABLE();
}

template <> inline void GpioPort<GPIOE_BASE>::DeInit()
{
    GpioPort<GPIOE_BASE>::SetMode(Mode::Input);
    __HAL_RCC_GPIOE_CLK_DISABLE();
}

#ifdef GPIOF
template <> inline void GpioPort<GPIOF_BASE>::DeInit()
{
    GpioPort<GPIOF_BASE>::SetMode(Mode::Input);
    __HAL_RCC_GPIOF_CLK_DISABLE();
}
#endif

#ifdef GPIOG
template <> inline void GpioPort<GPIOG_BASE>::DeInit()
{
    GpioPort<GPIOG_BASE>::SetMode(Mode::Input);
    __HAL_RCC_GPIOG_CLK_DISABLE();
}
#endif

template <size_t GPIO_BASE> void GpioPort<GPIO_BASE>::Write(uint16_t value)
{
    GetPortBaseAddress()->ODR = value;
}

template <size_t GPIO_BASE> uint16_t GpioPort<GPIO_BASE>::Read()
{
    return GetPortBaseAddress()->IDR;
}

template <size_t GPIO_BASE> void GpioPort<GPIO_BASE>::SetBits(uint16_t mask)
{
    GetPortBaseAddress()->BSRR = mask;
}

template <size_t GPIO_BASE> void GpioPort<GPIO_BASE>::ResetBits(uint16_t mask)
{
    GetPortBaseAddress()->BSRR = mask << 16;
}

template <size_t GPIO_BASE> void GpioPort<GPIO_BASE>::ToggleBits(uint16_t mask)
{
    GetPortBaseAddress()->ODR ^= mask;
}

template <size_t GPIO_BASE> bool GpioPort<GPIO_BASE>::ReadBit(int bit_index)
{
    return GetPortBaseAddress()->IDR & (1 << bit_index);
}

template <size_t GPIO_BASE> void GpioPort<GPIO_BASE>::SetMode(Mode mode, uint16_t mask, uint16_t init_value)
{
    //настройка режима порта
    uint8_t setup_bits = 0;
    switch (mode)
    {
    case Mode::Input:
        setup_bits = 0x4;
        break;
    case Mode::InputPullUp:
        setup_bits = 0x8;
        break;
    case Mode::InputPullDown:
        setup_bits = 0x8;
        break;
    case Mode::Output:
        setup_bits = 0x3;
        break;
    case Mode::OutputOpenDrain:
        setup_bits = 0x7;
        break;
    case Mode::Analog:
		setup_bits = 0x0;
        break;
    }

    uint32_t mask_l = 0;
    uint32_t mask_h = 0;
    uint32_t setup = 0;

    for (size_t i = 0; i < 8; i++)
    {
        setup |= setup_bits << (i * 4);
        mask_l = mask & (1 << i) ? mask_l | (0xF << (i * 4)) : mask_l;
        mask_h = mask & (1 << (i + 8)) ? mask_h | (0xF << (i * 4)) : mask_h;
    }

    GetPortBaseAddress()->CRL = (GetPortBaseAddress()->CRL & ~mask_l) | (mask_l & setup);
    GetPortBaseAddress()->CRH = (GetPortBaseAddress()->CRH & ~mask_h) | (mask_h & setup);

    // pull direction
    if (mode == Mode::InputPullDown)
        ResetBits(mask);
    if (mode == Mode::InputPullUp)
        SetBits(mask);
}

template <size_t GPIO_BASE> void GpioPort<GPIO_BASE>::DisableInterrupt(uint16_t PinNumber)
{
    if (PinNumber > 15)
        return;
    _delegates[PinNumber] = ExtiIsr{};
	EXTI->IMR &= ~(1 << PinNumber);	//Маскировка прерывания
    switch (PinNumber)
    {
    case 0:
        NVIC_DisableIRQ(EXTI0_IRQn);
        break;
    case 1:
        NVIC_DisableIRQ(EXTI1_IRQn);
        break;
    case 2:
        NVIC_DisableIRQ(EXTI2_IRQn);
        break;
    case 3:
        NVIC_DisableIRQ(EXTI3_IRQn);
        break;
    case 4:
        NVIC_DisableIRQ(EXTI4_IRQn);
        break;
    case 5:
    case 6:
    case 7:
    case 8:
    case 9: {
        bool disable_interrupt = true;
        for (int i = 5; i <= 9; i++)
            if (EXTI->IMR & (1 << i))
            {
                disable_interrupt = false;
                break;
            }
        if (disable_interrupt)
            NVIC_DisableIRQ(EXTI9_5_IRQn);
    }
    break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15: {
        bool disable_interrupt = true;
        for (int i = 10; i <= 15; i++)
            if (EXTI->IMR & (1 << i))
            {
                disable_interrupt = false;
                break;
            }
        if (disable_interrupt)
            NVIC_DisableIRQ(EXTI15_10_IRQn);
    }
    break;
    }
}

template <size_t GPIO_BASE>
void GpioPort<GPIO_BASE>::EnableInterrupt(uint16_t PinNumber, InterruptMode mode, uint32_t interrupt_priority)
{
    __HAL_RCC_AFIO_CLK_ENABLE();
    if (PinNumber > 15)
        return;
    _delegates[PinNumber] = ExtiIsr{};
    if (PinNumber == 0)
    {
        NVIC_SetPriority(EXTI0_IRQn, interrupt_priority);
        NVIC_EnableIRQ(EXTI0_IRQn);
    }
    else if (PinNumber == 1)
    {
        NVIC_SetPriority(EXTI1_IRQn, interrupt_priority);
        NVIC_EnableIRQ(EXTI1_IRQn);
    }
    else if (PinNumber == 2)
    {
        NVIC_SetPriority(EXTI2_IRQn, interrupt_priority);
        NVIC_EnableIRQ(EXTI2_IRQn);
    }
    else if (PinNumber == 3)
    {
        NVIC_SetPriority(EXTI3_IRQn, interrupt_priority);
        NVIC_EnableIRQ(EXTI3_IRQn);
    }
    else if (PinNumber == 4)
    {
        NVIC_SetPriority(EXTI4_IRQn, interrupt_priority);
        NVIC_EnableIRQ(EXTI4_IRQn);
    }
    else if (PinNumber <= 9 && PinNumber >= 5)
    {
        NVIC_SetPriority(EXTI9_5_IRQn, interrupt_priority);
        NVIC_EnableIRQ(EXTI9_5_IRQn);
    }
    else if (PinNumber <= 15 && PinNumber >= 10)
    {
        NVIC_SetPriority(EXTI15_10_IRQn, interrupt_priority);
        NVIC_EnableIRQ(EXTI15_10_IRQn);
    }
    EnableEextiIntBit(PinNumber, mode);
}

template <size_t GPIO_BASE> void GpioPort<GPIO_BASE>::EnableEextiIntBit(uint16_t PinNumber, InterruptMode mode)
{
    switch (PinNumber)
    {
    case 0:
        AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI0;
        AFIO->EXTICR[0] |= (EXTIx_bits() << AFIO_EXTICR1_EXTI0_Pos);
        EXTI->IMR |= EXTI_IMR_IM0;
        if (mode == InterruptMode::RisingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->RTSR |= EXTI_RTSR_TR0;
        else
            EXTI->RTSR &= ~EXTI_RTSR_TR0;
        if (mode == InterruptMode::FallingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->FTSR |= EXTI_FTSR_TR0;
        else
            EXTI->FTSR &= ~EXTI_FTSR_TR0;
        break;
    case 1:
        AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI1;
        AFIO->EXTICR[0] |= (EXTIx_bits() << AFIO_EXTICR1_EXTI1_Pos);
        EXTI->IMR |= EXTI_IMR_IM1;
        if (mode == InterruptMode::RisingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->RTSR |= EXTI_RTSR_TR1;
        else
            EXTI->RTSR &= ~EXTI_RTSR_TR1;
        if (mode == InterruptMode::FallingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->FTSR |= EXTI_FTSR_TR1;
        else
            EXTI->FTSR &= ~EXTI_FTSR_TR1;
        break;
    case 2:
        AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI2;
        AFIO->EXTICR[0] |= (EXTIx_bits() << AFIO_EXTICR1_EXTI2_Pos);
        EXTI->IMR |= EXTI_IMR_IM2;
        if (mode == InterruptMode::RisingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->RTSR |= EXTI_RTSR_TR2;
        else
            EXTI->RTSR &= ~EXTI_RTSR_TR2;
        if (mode == InterruptMode::FallingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->FTSR |= EXTI_FTSR_TR2;
        else
            EXTI->FTSR &= ~EXTI_FTSR_TR2;
        break;
    case 3:
        AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI3;
        AFIO->EXTICR[0] |= (EXTIx_bits() << AFIO_EXTICR1_EXTI3_Pos);
        EXTI->IMR |= EXTI_IMR_IM3;
        if (mode == InterruptMode::RisingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->RTSR |= EXTI_RTSR_TR3;
        else
            EXTI->RTSR &= ~EXTI_RTSR_TR3;
        if (mode == InterruptMode::FallingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->FTSR |= EXTI_FTSR_TR3;
        else
            EXTI->FTSR &= ~EXTI_FTSR_TR3;
        break;
    case 4:
        AFIO->EXTICR[1] &= ~AFIO_EXTICR1_EXTI0;
        AFIO->EXTICR[1] |= (EXTIx_bits() << AFIO_EXTICR1_EXTI0_Pos);
        EXTI->IMR |= EXTI_IMR_IM4;
        if (mode == InterruptMode::RisingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->RTSR |= EXTI_RTSR_TR4;
        else
            EXTI->RTSR &= ~EXTI_RTSR_TR4;
        if (mode == InterruptMode::FallingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->FTSR |= EXTI_FTSR_TR4;
        else
            EXTI->FTSR &= ~EXTI_FTSR_TR4;
        break;
    case 5:
        AFIO->EXTICR[1] &= ~AFIO_EXTICR1_EXTI1;
        AFIO->EXTICR[1] |= (EXTIx_bits() << AFIO_EXTICR1_EXTI1_Pos);
        EXTI->IMR |= EXTI_IMR_IM5;
        if (mode == InterruptMode::RisingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->RTSR |= EXTI_RTSR_TR5;
        else
            EXTI->RTSR &= ~EXTI_RTSR_TR5;
        if (mode == InterruptMode::FallingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->FTSR |= EXTI_FTSR_TR5;
        else
            EXTI->FTSR &= ~EXTI_FTSR_TR5;
        break;
    case 6:
        AFIO->EXTICR[1] &= ~AFIO_EXTICR1_EXTI2;
        AFIO->EXTICR[1] |= (EXTIx_bits() << AFIO_EXTICR1_EXTI2_Pos);
        EXTI->IMR |= EXTI_IMR_IM6;
        if (mode == InterruptMode::RisingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->RTSR |= EXTI_RTSR_TR6;
        else
            EXTI->RTSR &= ~EXTI_RTSR_TR6;
        if (mode == InterruptMode::FallingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->FTSR |= EXTI_FTSR_TR6;
        else
            EXTI->FTSR &= ~EXTI_FTSR_TR6;
        break;
    case 7:
        AFIO->EXTICR[1] &= ~AFIO_EXTICR1_EXTI3;
        AFIO->EXTICR[1] |= (EXTIx_bits() << AFIO_EXTICR1_EXTI3_Pos);
        EXTI->IMR |= EXTI_IMR_IM7;
        if (mode == InterruptMode::RisingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->RTSR |= EXTI_RTSR_TR7;
        else
            EXTI->RTSR &= ~EXTI_RTSR_TR7;
        if (mode == InterruptMode::FallingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->FTSR |= EXTI_FTSR_TR7;
        else
            EXTI->FTSR &= ~EXTI_FTSR_TR7;
        break;
    case 8:
        AFIO->EXTICR[2] &= ~AFIO_EXTICR1_EXTI0;
        AFIO->EXTICR[2] |= (EXTIx_bits() << AFIO_EXTICR1_EXTI0_Pos);
        EXTI->IMR |= EXTI_IMR_IM8;
        if (mode == InterruptMode::RisingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->RTSR |= EXTI_RTSR_TR8;
        else
            EXTI->RTSR &= ~EXTI_RTSR_TR8;
        if (mode == InterruptMode::FallingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->FTSR |= EXTI_FTSR_TR8;
        else
            EXTI->FTSR &= ~EXTI_FTSR_TR8;
        break;
    case 9:
        AFIO->EXTICR[2] &= ~AFIO_EXTICR1_EXTI1;
        AFIO->EXTICR[2] |= (EXTIx_bits() << AFIO_EXTICR1_EXTI1_Pos);
        EXTI->IMR |= EXTI_IMR_IM9;
        if (mode == InterruptMode::RisingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->RTSR |= EXTI_RTSR_TR9;
        else
            EXTI->RTSR &= ~EXTI_RTSR_TR9;
        if (mode == InterruptMode::FallingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->FTSR |= EXTI_FTSR_TR9;
        else
            EXTI->FTSR &= ~EXTI_FTSR_TR9;
        break;
    case 10:
        AFIO->EXTICR[2] &= ~AFIO_EXTICR1_EXTI2;
        AFIO->EXTICR[2] |= (EXTIx_bits() << AFIO_EXTICR1_EXTI2_Pos);
        EXTI->IMR |= EXTI_IMR_IM10;
        if (mode == InterruptMode::RisingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->RTSR |= EXTI_RTSR_TR10;
        else
            EXTI->RTSR &= ~EXTI_RTSR_TR10;
        if (mode == InterruptMode::FallingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->FTSR |= EXTI_FTSR_TR10;
        else
            EXTI->FTSR &= ~EXTI_FTSR_TR10;
        break;
    case 11:
        AFIO->EXTICR[2] &= ~AFIO_EXTICR1_EXTI3;
        AFIO->EXTICR[2] |= (EXTIx_bits() << AFIO_EXTICR1_EXTI3_Pos);
        EXTI->IMR |= EXTI_IMR_IM11;
        if (mode == InterruptMode::RisingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->RTSR |= EXTI_RTSR_TR11;
        else
            EXTI->RTSR &= ~EXTI_RTSR_TR11;
        if (mode == InterruptMode::FallingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->FTSR |= EXTI_FTSR_TR11;
        else
            EXTI->FTSR &= ~EXTI_FTSR_TR11;
        break;
    case 12:
        AFIO->EXTICR[3] &= ~AFIO_EXTICR1_EXTI0;
        AFIO->EXTICR[3] |= (EXTIx_bits() << AFIO_EXTICR1_EXTI0_Pos);
        EXTI->IMR |= EXTI_IMR_IM12;
        if (mode == InterruptMode::RisingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->RTSR |= EXTI_RTSR_TR12;
        else
            EXTI->RTSR &= ~EXTI_RTSR_TR12;
        if (mode == InterruptMode::FallingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->FTSR |= EXTI_FTSR_TR12;
        else
            EXTI->FTSR &= ~EXTI_FTSR_TR12;
        break;
    case 13:
        AFIO->EXTICR[3] &= ~AFIO_EXTICR1_EXTI1;
        AFIO->EXTICR[3] |= (EXTIx_bits() << AFIO_EXTICR1_EXTI1_Pos);
        EXTI->IMR |= EXTI_IMR_IM13;
        if (mode == InterruptMode::RisingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->RTSR |= EXTI_RTSR_TR13;
        else
            EXTI->RTSR &= ~EXTI_RTSR_TR13;
        if (mode == InterruptMode::FallingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->FTSR |= EXTI_FTSR_TR13;
        else
            EXTI->FTSR &= ~EXTI_FTSR_TR13;
        break;
    case 14:
        AFIO->EXTICR[3] &= ~AFIO_EXTICR1_EXTI2;
        AFIO->EXTICR[3] |= (EXTIx_bits() << AFIO_EXTICR1_EXTI2_Pos);
        EXTI->IMR |= EXTI_IMR_IM14;
        if (mode == InterruptMode::RisingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->RTSR |= EXTI_RTSR_TR14;
        else
            EXTI->RTSR &= ~EXTI_RTSR_TR14;
        if (mode == InterruptMode::FallingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->FTSR |= EXTI_FTSR_TR14;
        else
            EXTI->FTSR &= ~EXTI_FTSR_TR14;
        break;
    case 15:
        AFIO->EXTICR[3] &= ~AFIO_EXTICR1_EXTI3;
        AFIO->EXTICR[3] |= (EXTIx_bits() << AFIO_EXTICR1_EXTI3_Pos);
        EXTI->IMR |= EXTI_IMR_IM15;
        if (mode == InterruptMode::RisingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->RTSR |= EXTI_RTSR_TR15;
        else
            EXTI->RTSR &= ~EXTI_RTSR_TR15;
        if (mode == InterruptMode::FallingEdge || mode == InterruptMode::ChangeLevel)
            EXTI->FTSR |= EXTI_FTSR_TR15;
        else
            EXTI->FTSR &= ~EXTI_FTSR_TR15;
        break;
    }
}

template <size_t GPIO_BASE> constexpr GPIO_TypeDef *GpioPort<GPIO_BASE>::GetPortBaseAddress()
{
    return (GPIO_TypeDef *)GPIO_BASE;
}

/*
#ifdef GPIOA
template<>
constexpr GPIO_TypeDef* GpioPort<PortAIndex>::GetPortBaseAddress()
{
    return GPIOA;
}
#endif

#ifdef GPIOB
template<>
constexpr GPIO_TypeDef* GpioPort<PortBIndex>::GetPortBaseAddress()
{
    return GPIOB;
}
#endif

#ifdef GPIOC
template<>
constexpr GPIO_TypeDef* GpioPort<PortCIndex>::GetPortBaseAddress()
{
    return GPIOC;
}
#endif

#ifdef GPIOD
template<>
constexpr GPIO_TypeDef* GpioPort<PortDIndex>::GetPortBaseAddress()
{
    return GPIOD;
}

#endif

#ifdef GPIOE

template<>
constexpr GPIO_TypeDef* GpioPort<PortEIndex>::GetPortBaseAddress()
{
    return GPIOE;
}
#endif

#ifdef GPIOF
template<>
constexpr GPIO_TypeDef* GpioPort<PortFIndex>::GetPortBaseAddress()
{
    return GPIOF;
}
#endif

#ifdef GPIOG
template<>
constexpr GPIO_TypeDef* GpioPort<PortGIndex>::GetPortBaseAddress()
{
    return GPIOG;
}
#endif

#ifdef GPIOH
template<>
constexpr GPIO_TypeDef* GpioPort<PortHIndex>::GetPortBaseAddress()
{
    return GPIOH;
}
#endif
*/
#ifdef GPIOA
typedef GpioPort<GPIOA_BASE> porta;
#endif
#ifdef GPIOB
typedef GpioPort<GPIOB_BASE> portb;
#endif
#ifdef GPIOC
typedef GpioPort<GPIOC_BASE> portc;
#endif
#ifdef GPIOD
typedef GpioPort<GPIOD_BASE> portd;
#endif
#ifdef GPIOE
typedef GpioPort<GPIOE_BASE> porte;
#endif
#ifdef GPIOF
typedef GpioPort<GPIOF_BASE> portf;
#endif
#ifdef GPIOG
typedef GpioPort<GPIOG_BASE> portg;
#endif
#ifdef GPIOH
typedef GpioPort<GPIOH_BASE> porth;
#endif

#ifdef GPIOA
typedef Pin<porta, 0> PA0;
typedef Pin<porta, 1> PA1;
typedef Pin<porta, 2> PA2;
typedef Pin<porta, 3> PA3;
typedef Pin<porta, 4> PA4;
typedef Pin<porta, 5> PA5;
typedef Pin<porta, 6> PA6;
typedef Pin<porta, 7> PA7;
typedef Pin<porta, 8> PA8;
typedef Pin<porta, 9> PA9;
typedef Pin<porta, 10> PA10;
typedef Pin<porta, 11> PA11;
typedef Pin<porta, 12> PA12;
typedef Pin<porta, 13> PA13;
typedef Pin<porta, 14> PA14;
typedef Pin<porta, 15> PA15;
#endif

#ifdef GPIOB
typedef Pin<portb, 0> PB0;
typedef Pin<portb, 1> PB1;
typedef Pin<portb, 2> PB2;
typedef Pin<portb, 3> PB3;
typedef Pin<portb, 4> PB4;
typedef Pin<portb, 5> PB5;
typedef Pin<portb, 6> PB6;
typedef Pin<portb, 7> PB7;
typedef Pin<portb, 8> PB8;
typedef Pin<portb, 9> PB9;
typedef Pin<portb, 10> PB10;
typedef Pin<portb, 11> PB11;
typedef Pin<portb, 12> PB12;
typedef Pin<portb, 13> PB13;
typedef Pin<portb, 14> PB14;
typedef Pin<portb, 15> PB15;
#endif

#ifdef GPIOC
typedef Pin<portc, 0> PC0;
typedef Pin<portc, 1> PC1;
typedef Pin<portc, 2> PC2;
typedef Pin<portc, 3> PC3;
typedef Pin<portc, 4> PC4;
typedef Pin<portc, 5> PC5;
typedef Pin<portc, 6> PC6;
typedef Pin<portc, 7> PC7;
typedef Pin<portc, 8> PC8;
typedef Pin<portc, 9> PC9;
typedef Pin<portc, 10> PC10;
typedef Pin<portc, 11> PC11;
typedef Pin<portc, 12> PC12;
typedef Pin<portc, 13> PC13;
typedef Pin<portc, 14> PC14;
typedef Pin<portc, 15> PC15;
#endif

#ifdef GPIOD
typedef Pin<portd, 0> PD0;
typedef Pin<portd, 1> PD1;
typedef Pin<portd, 2> PD2;
typedef Pin<portd, 3> PD3;
typedef Pin<portd, 4> PD4;
typedef Pin<portd, 5> PD5;
typedef Pin<portd, 6> PD6;
typedef Pin<portd, 7> PD7;
typedef Pin<portd, 8> PD8;
typedef Pin<portd, 9> PD9;
typedef Pin<portd, 10> PD10;
typedef Pin<portd, 11> PD11;
typedef Pin<portd, 12> PD12;
typedef Pin<portd, 13> PD13;
typedef Pin<portd, 14> PD14;
typedef Pin<portd, 15> PD15;
#endif

#ifdef GPIOE
typedef Pin<porte, 0> PE0;
typedef Pin<porte, 1> PE1;
typedef Pin<porte, 2> PE2;
typedef Pin<porte, 3> PE3;
typedef Pin<porte, 4> PE4;
typedef Pin<porte, 5> PE5;
typedef Pin<porte, 6> PE6;
typedef Pin<porte, 7> PE7;
typedef Pin<porte, 8> PE8;
typedef Pin<porte, 9> PE9;
typedef Pin<porte, 10> PE10;
typedef Pin<porte, 11> PE11;
typedef Pin<porte, 12> PE12;
typedef Pin<porte, 13> PE13;
typedef Pin<porte, 14> PE14;
typedef Pin<porte, 15> PE15;
#endif

#ifdef GPIOF
typedef Pin<portf, 0> PF0;
typedef Pin<portf, 1> PF1;
typedef Pin<portf, 2> PF2;
typedef Pin<portf, 3> PF3;
typedef Pin<portf, 4> PF4;
typedef Pin<portf, 5> PF5;
typedef Pin<portf, 6> PF6;
typedef Pin<portf, 7> PF7;
typedef Pin<portf, 8> PF8;
typedef Pin<portf, 9> PF9;
typedef Pin<portf, 10> PF10;
typedef Pin<portf, 11> PF11;
typedef Pin<portf, 12> PF12;
typedef Pin<portf, 13> PF13;
typedef Pin<portf, 14> PF14;
typedef Pin<portf, 15> PF15;
#endif

#ifdef GPIOG
typedef Pin<portg, 0> PG0;
typedef Pin<portg, 1> PG1;
typedef Pin<portg, 2> PG2;
typedef Pin<portg, 3> PG3;
typedef Pin<portg, 4> PG4;
typedef Pin<portg, 5> PG5;
typedef Pin<portg, 6> PG6;
typedef Pin<portg, 7> PG7;
typedef Pin<portg, 8> PG8;
typedef Pin<portg, 9> PG9;
typedef Pin<portg, 10> PG10;
typedef Pin<portg, 11> PG11;
typedef Pin<portg, 12> PG12;
typedef Pin<portg, 13> PG13;
typedef Pin<portg, 14> PG14;
typedef Pin<portg, 15> PG15;
#endif
