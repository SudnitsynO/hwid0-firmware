#pragma once

#include "gpio_interface.h"
#include "stm32f0xx_hal.h"


/*
enum PortIndex
{
    PortAIndex = (size_t)(GPIOA),
    PortBIndex = (size_t)(GPIOB),
    PortCIndex = (size_t)(GPIOC),
    PortDIndex = (size_t)(GPIOD),
    PortEIndex = (size_t)(GPIOE),
    PortFIndex = (size_t)(GPIOF),
    PortGIndex = (size_t)(GPIOG),
    PortHIndex = (size_t)(GPIOH)
};
*/
extern ExtiIsr _delegates[16];

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
    static ExtiIsr &GetIstDelegate(uint16_t PinNumber);

  private:
    static constexpr GPIO_TypeDef *GetPortBaseAddress();
    static void EnableEextiIntBit(uint16_t PinNumber, InterruptMode mode);
    static constexpr uint32_t EXTIx_bits();
};

template <size_t GPIO_BASE> ExtiIsr &GpioPort<GPIO_BASE>::GetIstDelegate(uint16_t PinNumber)
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
#ifdef GPIOE
    case GPIOE_BASE:
        return 4;
        break;
#endif
    case GPIOF_BASE:
        return 5;
        break;
#ifdef GPIOG
    case GPIOG_BASE:
        return 6;
        break;
#endif
#ifdef GPIOH
    case GPIOH_BASE:
        return 7;
        break;
#endif
    default:
        return 15;
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

#ifdef GPIOE
template <> inline void GpioPort<GPIOE_BASE>::Init()
{
    __HAL_RCC_GPIOE_CLK_ENABLE();
}
#endif

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

#ifdef GPIOE
template <> inline void GpioPort<GPIOE_BASE>::DeInit()
{
    GpioPort<PortEIndex>::SetMode(Mode::Input);
    __HAL_RCC_GPIOE_CLK_DISABLE();
}
#endif

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
    uint32_t MODER_bits = 0;
    uint32_t OTYPER_bits = 0;
    uint32_t OSPEEDR_bits = 3;
    uint32_t PUPDR_bits = 0;
    switch (mode)
    {
    case Mode::Input:
        break;
    case Mode::InputPullUp:
        PUPDR_bits = 1;
        break;
    case Mode::InputPullDown:
        PUPDR_bits = 2;
        break;
    case Mode::Output:
        MODER_bits = 1;
        break;
    case Mode::OutputOpenDrain:
        OTYPER_bits = 1;
        break;
    case Mode::Analog:
        MODER_bits = 3;
        break;
    }

    uint32_t MODER = 0;
    uint32_t OTYPER = 0;
    uint32_t mask32 = 0;
    uint32_t OSPEEDR = 0;
    uint32_t PUPDR = 0;
    for (int i = 0; i < 16; i++)
    {
        if (mask & (1 << i))
        {
            MODER |= MODER_bits << (i * 2);
            mask32 |= 3 << (i * 2);
            OTYPER |= OTYPER_bits << i;
            OSPEEDR |= OSPEEDR_bits << (i * 2);
            PUPDR |= PUPDR_bits << (i * 2);
        }
    }

    GetPortBaseAddress()->MODER = (GetPortBaseAddress()->MODER & ~mask32) | MODER;
    GetPortBaseAddress()->OTYPER = (GetPortBaseAddress()->OTYPER & ~mask) | OTYPER;
    GetPortBaseAddress()->OSPEEDR = (GetPortBaseAddress()->OSPEEDR & ~mask32) | OSPEEDR;
    GetPortBaseAddress()->PUPDR = (GetPortBaseAddress()->PUPDR & ~mask32) | PUPDR;

    // GetPortBaseAddress()->ODR = (GetPortBaseAddress()->ODR & ~mask) | (init_value & mask);
}

template <size_t GPIO_BASE> void GpioPort<GPIO_BASE>::DisableInterrupt(uint16_t PinNumber)
{
    if (PinNumber > 15)
        return;
    EXTI->IMR &= ~(1 << PinNumber); //Маскировка прерывания
    _delegates[PinNumber] = ExtiIsr{};

    /*
    switch (PinNumber)
    {
    case 0:
    case 1:
        if ((EXTI->IMR & (3 << 0)) == 0)
            NVIC_DisableIRQ(EXTI0_1_IRQn);	//TODO: сделать также для F1
        break;
    case 2:
    case 3:
    if ((EXTI->IMR & (3 << 2)) == 0)
            NVIC_DisableIRQ(EXTI2_3_IRQn);
        break;
    case 4:
    NVIC_DisableIRQ(EX)
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
    */
}

template <size_t GPIO_BASE>
void GpioPort<GPIO_BASE>::EnableInterrupt(uint16_t PinNumber, InterruptMode mode, uint32_t interrupt_priority)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    //__HAL_RCC_AFIO_CLK_ENABLE();
    //__HAL_RCC_APB1_RELEASE_RESET();
    //__HAL_RCC_APB2_RELEASE_RESET();
    if (PinNumber > 15)
        return;
    _delegates[PinNumber] = ExtiIsr{};

    auto temp = SYSCFG->EXTICR[PinNumber >> 2u];
    temp &= ~(0x0FuL << (4u * (PinNumber & 0x03u)));
    temp |= (EXTIx_bits() << (4u * (PinNumber & 0x03u)));
    SYSCFG->EXTICR[PinNumber >> 2u] = temp;

    switch (PinNumber)
    {
    case 0:
    case 1:
        NVIC_SetPriority(EXTI0_1_IRQn, interrupt_priority);
        NVIC_EnableIRQ(EXTI0_1_IRQn);
        break;
    case 2:
    case 3:
        NVIC_SetPriority(EXTI2_3_IRQn, interrupt_priority);
        NVIC_EnableIRQ(EXTI2_3_IRQn);
        break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        NVIC_SetPriority(EXTI4_15_IRQn, interrupt_priority);
        NVIC_EnableIRQ(EXTI4_15_IRQn);
        break;
    }
    EnableEextiIntBit(PinNumber, mode);
}

template <size_t GPIO_BASE> void GpioPort<GPIO_BASE>::EnableEextiIntBit(uint16_t PinNumber, InterruptMode mode)
{
    const uint32_t mask = 1 << PinNumber;
    EXTI->IMR |= mask;
    if (mode == InterruptMode::RisingEdge || mode == InterruptMode::ChangeLevel)
        EXTI->RTSR |= mask;
    else
        EXTI->RTSR &= ~mask;
    if (mode == InterruptMode::FallingEdge || mode == InterruptMode::ChangeLevel)
        EXTI->FTSR |= mask;
    else
        EXTI->FTSR &= ~mask;
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
