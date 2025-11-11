#pragma once
#include "NullType.h"
#include "Typelist.h"
#include "delegate.h"
#include <cstdint>


/*********************************************
 *  Шаблон класса Port, необходимо реализовать
 *  в драйверах порта
 *  ******************************************
 *
class Port
{
public:
    static void Write(uint16_t value);
    static uint16_t Read();
    static void SetBits(uint16_t mask);
    static void ResetBits(uint16_t mask);
    static void ToggleBits(uint16_t mask);
    static bool ReadBit(int bit_index);
    static void Update();
    static void SetMode(Mode mode, uint16_t mask = 0xFFFF, uint16_t init_value = 0);
    static void Init();
    static void DeInit();
    static void EnableInterrupt(uint16_t PinNumber, InterruptMode mode, ExtiIsr callback, void* context, uint32_t
interrupt_priority); static void DisableInterrupt(uint16_t PinNumber);
};
*/

// typedef void (*ExtiIsr)(void*);
typedef Delegate<void(void)> ExtiIsr;
/****************************************************
 *			Режимы работы порта						*
 * **************************************************/
enum class Mode
{
    Input,
    InputPullUp,
    InputPullDown,
    Output,
    OutputOpenDrain,
    Analog
};

/****************************************************
 *Режимы работы порта*
 ***************************************************/
enum class InterruptMode
{
    RisingEdge,
    FallingEdge,
    HighLevel,
    LowLevel,
    ChangeLevel
};
#ifndef DEFAULT_EXTI_PRIORITY
#define DEFAULT_EXTI_PRIORITY 10
#endif

/****************************************************
 *		Шаблон универсального класса Pin			*
 * **************************************************/
template <class PORT, unsigned long PIN> class Pin
{
  public:
    typedef PORT Port;
    enum
    {
        PinNumber = PIN
    };
    enum
    {
        PinMask = 1 << PIN
    };
    static constexpr bool exist();
    static void Set();
    static void Reset();
    static void Toggle();
    static void Write(bool value);
    static bool Read();
    static void SetMode(Mode mode, bool init_value = false);
    static void Init();
    static void SetInterrupt(InterruptMode interrupt_mode, uint32_t priority = DEFAULT_EXTI_PRIORITY);
    static void DisableInterrupt();
    static ExtiIsr &GetIsrDelegate();
};

template <unsigned long PIN> class Pin<Loki::NullType, PIN>
{
  public:
    // typedef PORT Port;
    enum
    {
        PinNumber = PIN
    };
    enum
    {
        PinMask = 1 << PIN
    };
    static constexpr bool exist();
    static void Set();
    static void Reset();
    static void Toggle();
    static void Write(bool value);
    static bool Read();
    static void SetMode(Mode mode, bool init_value = false);
    static void Init();
    static void SetInterrupt(InterruptMode interrupt_mode, uint32_t priority = DEFAULT_EXTI_PRIORITY);
    static void DisableInterrupt();
    static ExtiIsr &GetIsrDelegate();

  private:
    static ExtiIsr localdelegate;
};

template <class PORT, unsigned long PIN> ExtiIsr &Pin<PORT, PIN>::GetIsrDelegate()
{
    return PORT::GetIstDelegate(PinNumber);
}

template <class PORT, unsigned long PIN> void Pin<PORT, PIN>::Set()
{
    PORT::SetBits(PinMask);
}

template <class PORT, unsigned long PIN> void Pin<PORT, PIN>::Reset()
{
    PORT::ResetBits(PinMask);
}

template <class PORT, unsigned long PIN> void Pin<PORT, PIN>::Toggle()
{
    PORT::ToggleBits(PinMask);
}

template <class PORT, unsigned long PIN> void Pin<PORT, PIN>::Write(bool value)
{
    if (value)
        Set();
    else
        Reset();
}

template <class PORT, unsigned long PIN> bool Pin<PORT, PIN>::Read()
{
    return PORT::ReadBit(PinNumber);
}

template <class PORT, unsigned long PIN> void Pin<PORT, PIN>::SetMode(Mode mode, bool init_value)
{
    PORT::SetMode(mode, PinMask);
    if (init_value)
        Set();
    else
        Reset();
}

template <class PORT, unsigned long PIN> void Pin<PORT, PIN>::Init()
{
    PORT::Init();
}

template <class PORT, unsigned long PIN>
void Pin<PORT, PIN>::SetInterrupt(InterruptMode interrupt_mode, uint32_t priority)
{
    PORT::EnableInterrupt(PinNumber, interrupt_mode, priority);
}

template <class PORT, unsigned long PIN> void Pin<PORT, PIN>::DisableInterrupt()
{
    PORT::DisableInterrupt(PinNumber);
}

template <class PORT, unsigned long PIN> constexpr bool Pin<PORT, PIN>::exist()
{
    return true;
}

// NULL PORT

template <unsigned long PIN> ExtiIsr &Pin<Loki::NullType, PIN>::GetIsrDelegate()
{
    static_assert(true, "This is NullPort");
    return localdelegate;
}

template <unsigned long PIN> void Pin<Loki::NullType, PIN>::Set()
{
    static_assert(true, "This is NullPort");
}

template <unsigned long PIN> void Pin<Loki::NullType, PIN>::Reset()
{
    static_assert(true, "This is NullPort");
}

template <unsigned long PIN> void Pin<Loki::NullType, PIN>::Toggle()
{
    static_assert(true, "This is NullPort");
}

template <unsigned long PIN> void Pin<Loki::NullType, PIN>::Write(bool value)
{
    static_assert(true, "This is NullPort");
}

template <unsigned long PIN> bool Pin<Loki::NullType, PIN>::Read()
{
    static_assert(true, "This is NullPort");
    return false;
}

template <unsigned long PIN> void Pin<Loki::NullType, PIN>::SetMode(Mode mode, bool init_value)
{
    static_assert(true, "This is NullPort");
}

template <unsigned long PIN> void Pin<Loki::NullType, PIN>::Init()
{
    static_assert(true, "This is NullPort");
}

template <unsigned long PIN>
void Pin<Loki::NullType, PIN>::SetInterrupt(InterruptMode interrupt_mode, uint32_t priority)
{
    static_assert(true, "This is NullPort");
}

template <unsigned long PIN> void Pin<Loki::NullType, PIN>::DisableInterrupt()
{
    static_assert(true, "This is NullPort");
}

template <unsigned long PIN> constexpr bool Pin<Loki::NullType, PIN>::exist()
{
    return false;
}

/****************************************************
 *			Список пинов на базе списка типов Loki  *
 * **************************************************/
/*CREATE PIN LIST*/
template <typename T1 = Loki::NullType, typename T2 = Loki::NullType, typename T3 = Loki::NullType,
          typename T4 = Loki::NullType, typename T5 = Loki::NullType, typename T6 = Loki::NullType,
          typename T7 = Loki::NullType, typename T8 = Loki::NullType, typename T9 = Loki::NullType,
          typename T10 = Loki::NullType, typename T11 = Loki::NullType, typename T12 = Loki::NullType,
          typename T13 = Loki::NullType, typename T14 = Loki::NullType, typename T15 = Loki::NullType,
          typename T16 = Loki::NullType, typename T17 = Loki::NullType, typename T18 = Loki::NullType>
struct CreatePinList
{
  private:
    typedef typename CreatePinList<T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18>::Result
        TailResult;

  public:
    typedef Loki::Typelist<T1, TailResult> Result;
};

template <> struct CreatePinList<>
{
    typedef Loki::NullType Result;
};

/******************************************************/
/*             Шаблон GetPorts                        */
/*             Формирует список портов, которые	      */
/*             используются в указанном списке выводов*/
/******************************************************/

//шаблон принимает список линий в качестве параметра
template <class TList> struct GetPorts;
// для пустого списка результат – пустой тип
template <> struct GetPorts<Loki::NullType>
{
    typedef Loki::NullType AllPorts;
};

// для непустого списка
// конкретизируем, что это должен быть список типов
// содержащий голову Head и хвост Tail
template <class Head, class Tail> struct GetPorts<Loki::Typelist<Head, Tail>>
{
  private:
    // класс TPin помнит свой порт
    // запоминаем этот тип порта
    typedef typename Head::Port Port;
    //рекурсивно генерируем хвост
    typedef typename GetPorts<Tail>::AllPorts L1;

  public:
    // определяем список портов из текущего порта (Port) и хвоста (L1)
    typedef Loki::Typelist<Port, L1> AllPorts;
    // конвертируем список линий в соответствующий список портов
    // typedef typename GetPorts<TList>::Result PinsToPorts;
    // генерируем список портов без дудликатов
    typedef typename Loki::TL::NoDuplicates<AllPorts>::Result Ports;
};

/****************************************************
 *			Шаблон класса  GetPortMask				*
 *	Генерирует битовую маску для списка пинов       *
 * **************************************************/

template <class PinList> struct GetPortMask;

template <> struct GetPortMask<Loki::NullType>
{
    enum
    {
        value = 0
    };
};

template <class Head, class Tail> struct GetPortMask<Loki::Typelist<Head, Tail>>
{
    enum
    {
        value = Head::PinMask | GetPortMask<Tail>::value
    };
};

/****************************************************
 *     Шаблон      GetPinsWithPort     		        *
 *     Генерирует список пинов, принадлежащих ука-   *
 *     занному порту									*
 * **************************************************/
template <class PinList, class Port> struct GetPinsWithPort;

template <class Port> struct GetPinsWithPort<Loki::NullType, Port>
{
    typedef Loki::NullType Result;
};

template <class Port, class Tail, unsigned long PIN> struct GetPinsWithPort<Loki::Typelist<Pin<Port, PIN>, Tail>, Port>
{
    typedef Loki::Typelist<Pin<Port, PIN>, typename GetPinsWithPort<Tail, Port>::Result> Result;
};

template <class Head, class Tail, class Port> struct GetPinsWithPort<Loki::Typelist<Head, Tail>, Port>
{
    typedef typename GetPinsWithPort<Tail, Port>::Result Result;
};

/****************************************************************
 *     Шаблон      ShiftedValue       		                    *
 *     Сдвигает биты PinsToWrite виртуального порта  AllPins		*
 *     таким образом, чтобы они отображались                     *
 *     на биты физического порта, в котором они                  *
 *     находятся                                                 *
 * ***************************************************************/
template <class PinsToWrite, class AllPins> class ShiftedValue;

template <class PinsToWrite, class AllPins> class ShiftedValue
{
    typedef typename PinsToWrite::Head head;
    typedef typename PinsToWrite::Tail tail;

  public:
    static uint16_t Result(uint16_t value)
    {
        return ((1 << Loki::TL::IndexOf<AllPins, head>::value) & value ? head::PinMask : 0) |
               ShiftedValue<tail, AllPins>::Result(value);
    }
    static uint16_t ReverseResult(uint16_t value)
    {
        return (head::PinMask & value ? 1 << Loki::TL::IndexOf<AllPins, head>::value : 0) |
               ShiftedValue<tail, AllPins>::ReverseResult(value);
    }
};

template <class AllPins> class ShiftedValue<Loki::NullType, AllPins>
{
  public:
    static uint16_t Result(uint16_t value)
    {
        return 0;
    }
    static uint16_t ReverseResult(uint16_t value)
    {
        return 0;
    }
};

/****************************************************************
 *     Шаблон      PortIterator       		                    *
 *     Выполняет операцию с виртуальным портом                   *
 *     последовательно для каждого физического порта				*
 * ***************************************************************/
template <class PortList, class PinList> struct PortIterator;
template <class PinList> struct PortIterator<Loki::NullType, PinList>
{
    static void Write(uint16_t value)
    { /*ничего не делаем тут*/
    }
    static void SetBits(uint16_t mask)
    {
    }
    static uint16_t Read()
    {
        return 0;
    }
    static void SetMode(Mode mode, uint16_t mask)
    {
    }
};

template <class PortList, class PinList> struct PortIterator
{
    typedef typename PortList::Head Port;                         //текущий порт
    typedef typename GetPinsWithPort<PinList, Port>::Result Pins; //пины для изменения в данном порту
    static void Write(uint16_t value)
    {
        Port::Write((Port::Read() & ~GetPortMask<Pins>::value) |
                    ShiftedValue<Pins, PinList>::Result(value));      //для текущего
        PortIterator<typename PortList::Tail, PinList>::Write(value); //для следующего
    };
    static uint16_t Read()
    {
        return ShiftedValue<Pins, PinList>::ReverseResult(Port::Read()) |
               PortIterator<typename PortList::Tail, PinList>::Read();
    }
    static void SetBits(uint16_t mask)
    {
        Port::SetBits(ShiftedValue<Pins, PinList>::Result(mask));
        PortIterator<typename PortList::Tail, PinList>::SetBits(mask);
    }
    static void SetMode(Mode mode, uint16_t mask)
    {
        Port::SetMode(mode, GetPortMask<Pins>::value & mask);                //для текущего
        PortIterator<typename PortList::Tail, PinList>::SetMode(mode, mask); //для следующего
    }
    // static void ResetBits(uint16_t mask);
    // static void ToggleBits(uint16_t mask);
    // static bool ReadBit(int bit_index);
};

/********************************************************
 * Шаблон класса виртуального порта VirtualPort			*
 * Виртуальный порт состоит из линий PinList			*
 ********************************************************/
template <class PinList> class VirtualPort
{
  public:
    static void Write(uint16_t value)
    {
        PortIterator<PortsToWrite, PinList>::Write(value);
    }
    static uint16_t Read()
    {
        return PortIterator<PortsToWrite, PinList>::Read();
    }
    static void SetBits(uint16_t mask)
    {
        PortIterator<PortsToWrite, PinList>::SetBits(mask);
    }
    static void SetMode(Mode mode, uint16_t mask = 0xFFFF)
    {
        PortIterator<PortsToWrite, PinList>::SetMode(mode, mask);
    }
    // static void ResetBits(uint16_t mask);
    // static void ToggleBits(uint16_t mask);
    // static bool ReadBit(int bit_index);
  private:
    // static GPIO_TypeDef * GetPortBaseAddress();
    typedef typename GetPorts<PinList>::Ports PortsToWrite; //получаем список портов
};

typedef Pin<Loki::NullType, 0> NullPin;