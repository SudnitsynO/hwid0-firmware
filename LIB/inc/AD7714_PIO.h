#pragma once
#include "gpio_interface.h"
#include "delay.h"
/*
Номера каналов
#     AIN(+)      AIN(–)  Type                  Calibration Register Pair
0     AIN1        AIN6    Pseudo Differential   Register Pair 0
1     AIN2        AIN6    Pseudo Differential   Register Pair 1
2     AIN3        AIN6    Pseudo Differential   Register Pair 2
3     AIN4        AIN6    Pseudo Differential   Register Pair 2
4     AIN1        AIN2    Fully Differential    Register Pair 0
5     AIN3        AIN4    Fully Differential    Register Pair 1
6     AIN5        AIN6    Fully Differential    Register Pair 2
7     AIN6        AIN6    Test Mode             Register Pair 2

Режимы калибровки
0 - Normal Mode; this is the normal mode of operation of the device whereby the device is performing nor-
	mal conversions. This is the default condition of these bits after Power-On or RESET.
1 - Self-Calibration; this activates self-calibration on the channel selected by CH2, CH1 and CH0 of the
	Communications Register. This is a one step calibration sequence and when complete the part returns to
	Normal Mode with MD2, MD1 and MD0 returning to 0, 0, 0. The DRDY output or bit goes high when
	calibration is initiated and returns low when this self-calibration is complete and a new valid word is
	available in the data register. The zero-scale calibration is performed at the selected gain on internally
	shorted (zeroed) inputs and the full-scale calibration is performed at the selected gain on an internally-
	generated VREF/Selected Gain.
2 - Zero-Scale System Calibration; this activates zero scale system calibration on the channel selected by
	CH2, CH1 and CH0 of the Communications Register. Calibration is performed at the selected gain on
	the input voltage provided at the analog input during this calibration sequence. This input voltage should
	remain stable for the duration of the calibration. The DRDY output or bit goes high when calibration is
	initiated and returns low when this zero-scale calibration is complete and a new valid word is available in
	the data register. At the end of the calibration, the part returns to Normal Mode with MD2, MD1 and
	MD0 returning to 0, 0, 0.
3 - Full-Scale System Calibration; this activates full-scale system calibration on the selected input channel.
	Calibration is performed at the selected gain on the input voltage provided at the analog input during this
	calibration sequence.  This input voltage should remain stable for the duration of the calibration. Once
	again, the DRDY output or bit goes high when calibration is initiated and returns low when this full-scale
	calibration is complete and a new valid word is available in the data register. At the end of the calibration,
	the part returns to Normal Mode with MD2, MD1 and MD0 returning to 0, 0, 0.
4 - System-Offset Calibration; this activates system-offset calibration on the channel selected by CH2, CH1
	and CH0 of the Communications Register. This is a one step calibration sequence and when complete
	the part returns to Normal Mode with MD2, MD1 and MD0 returning to 0, 0, 0. The DRDY output
	or bit goes high when calibration is initiated and returns low when this system offset calibration is com-
	plete and a new valid word is available in the data register. For this calibration type, the zero-scale cali-
	bration is performed at the selected gain on the input voltage provided at the analog input during this
	calibration sequence. This input voltage should remain stable for the duration of the calibration. The
	full-scale calibration is performed at the selected gain on an internally generated VREF/Selected Gain.
5 - Background Calibration; this activates background calibration on the channel selected by CH2, CH1
	and CH0 of the Communications Register. If the background calibration mode is on, then the AD7714
	provides continuous self-calibration of the shorted (zeroed) inputs. This calibration takes place as part
	of the conversion sequence, extending the conversion time and reducing the word rate by a factor of six.
	Its major advantage is that the user does not have to worry about recalibrating the offset of the device
	when there is a change in the ambient temperature or supplies. In this mode,  the zero-scale calibration
	is performed at the selected gain on internally shorted (zeroed) inputs. The calibrations are interleaved
	with normal conversions and the calibration registers of the device are automatically updated. Because
	the background calibration does not perform full-scale calibrations, a self-calibration should be per-
	formed before placing the part in the background calibration mode.
6 - Zero-Scale Self-Calibration; this activates zero-scale self-calibration on the channel selected by CH2,
	CH1 and CH0 of the Communications Register. This zero-scale self-calibration is performed at the
	selected gain on internally shorted (zeroed) inputs. This is a one step calibration sequence and when
	complete the part returns to Normal Mode with MD2, MD1 and MD0 returning to 0, 0, 0. The DRDY
	output or bit goes high when calibration is initiated and returns low when this zero-scale self-calibration
	is complete and a new valid word is available in the data register.
7 - Full-Scale Self-Calibration; this activates full-scale self-calibration on the channel selected by CH2,
	CH1 and CH0 of the Communications Register. This full-scale self-calibration is performed at the
	selected gain on an internally-generated VREF/Selected Gain. This is a one step calibration sequence and
	when complete the part returns to Normal Mode with MD2, MD1 and MD0 returning to 0, 0, 0. The
	DRDY output or bit goes high when calibration is initiated and returns low when this full-scale self-
	calibration is complete and a new valid word is available in the data register.
*/
template <class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
class AD7714
{
public:
	void Init();	//производит инициализацию АЦП
//	bool Reset();
	//устанавливает значение фильтра - интерполятора
	void SetFilter(bool BipolarMode, bool HighResolutionMode, uint16_t DataRate);
	//устанавливает усиление (шаг 6 дб) значение от 0 до 7
	void SetGain(uint8_t gain);
	//программно сбрасывает АЦП
	void ADCReset();
	//возвращает текущую частоту дискретизации
	float GetDataRate();
	//возвратит true в биполярном режиме
	bool isBipolarMode();
	//возвратит true если АЦП в режиме высокого разрешения
	bool isHighResolutionMode();
	//возвратит true если АЦП в режиме теста входа
	bool isTestMode();
	//устанавливает режим теста входа
	void SetTestMode(bool TestMode);
	//возвращает режим работы АЦП
	uint8_t GetMode();
	//ожидает готовности АЦП
	void ADCWait();

	//КАЛИБРОВКИ
	// выключает калибровку п. 0
	void CalibrateDisable();
	//выполнит Автокалибровку п.1
	void CalibrateAuto();
	//выполнит Системную калибровку нуля п.2
	void CalibrateSystemZero();
	//выполнит системную калибровку полной шкалы п. 3
	void CalibrateSystemScale();
	//выполнит системную калибровку смещения п. 4
	void CalibrateSystemOffset();
	//включает режим фоновой калибровки п. 5
	void CalibrateBackgroundEnable();
	//выполнит автокалибровку нуля п. 6
	void CalibrateAutoZero();
	//выполнит автокалибровку полной шкалы п. 7
	void CalibrateAutoScale();   
	// переключает канал см выше.
	void SelectChannel(uint8_t channel);
	// чтение регистра данных
	int32_t ReadResult();
	// чтение регистра калибровки нуля шкалы
//	uint32_t ReadZerroCalibrateRegister();
	// читает регистр калибровки полной шкалы
//	uint32_t ReadFullScaleCalibrateRegister();
private:
	uint8_t TX_Byte(uint8_t byte);	//spi
	//записывает байт в указанный регистр
	void WriteByteToReg(uint32_t data, uint8_t reg, uint8_t len);
	//читает значение из регистра
	uint32_t ReadByteFromReg(uint8_t reg, uint8_t len);
	//обновляет регистр режима
	void UpdateModeReg();
	//канал
	uint8_t channel;
	//усиление 
	uint8_t gain;
	//режим
	uint8_t mode;
	//тестовый режим
	bool testmode;
	bool high_resulution;
	bool bipolar_mode;
};

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline void AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::Init()
{
	channel = 0;
	mode = 0;
	gain = 0;
	testmode = false;
	high_resulution = false;

	CS_line::SetMode(Mode::Output);
	DRDY_line::SetMode(Mode::Input);
	RESET_line::SetMode(Mode::Output);
	MISO_line::SetMode(Mode::Input);
	MOSI_line::SetMode(Mode::Output);
	SCK_line::SetMode(Mode::Output);
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline void AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::SetFilter(bool BipolarMode, bool HighResolutionMode, uint16_t DataRate)
{
	uint16_t FS = 2457600 / 128 / DataRate;
	uint8_t h = (BipolarMode ? 0 : 0x80) | (HighResolutionMode ? 0x40 : 0) | 0x20 | ((FS & 0xF00) >> 8);
	uint8_t l = FS & 0x00FF;
	WriteByteToReg(h, 2, 1);
	WriteByteToReg(l, 3, 1);
	high_resulution = HighResolutionMode;
	bipolar_mode = BipolarMode;
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline void AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::SetGain(uint8_t gain)
{
	AD7714::gain = gain;
	UpdateModeReg();
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline uint8_t AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::TX_Byte(uint8_t byte)
{

	uint8_t b = 0;
	for (int i = 7; i >= 0; i--)
	{
		SCK_line::Set();
		MOSI_line::Write(byte & (1 << i));
		Delay_us(1);		
		if (MISO_line::Read())
			b |= 1 << i;
		SCK_line::Reset();
		Delay_us(1);
	}
	return b;

}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline void AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::WriteByteToReg(uint32_t data, uint8_t reg, uint8_t len)
{
	SCK_line::Reset();
	CS_line::Reset();
	//запись в регистр обмена
	TX_Byte((reg << 4) | (0 << 3) | channel);
	while (len--)
	{
		TX_Byte(data >> (len * 8));
	}
	CS_line::Set();
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline uint32_t AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::ReadByteFromReg(uint8_t reg, uint8_t len)
{
	uint32_t data = 0;

	SCK_line::Reset();
	CS_line::Reset();
	//запись в регистр обмена
	TX_Byte((reg << 4) | (1 << 3) | channel);
	while (len--)
	{
		data |= ((uint32_t)TX_Byte(0xFF) << (len * 8));
	}
	CS_line::Set();
	return data;
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline void AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::UpdateModeReg()
{
	WriteByteToReg((mode << 5) | (gain << 2) | (testmode ? 2 : 0), 1, 1);
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline void AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::ADCReset()
{
	//hardware adc reset
	RESET_line::Reset();
	Delay_ms(1);
	RESET_line::Set();
	Delay_ms(1);
	//запись значений
	WriteByteToReg(0, 1, 1);
	gain = 0;
	mode = 0;
	testmode = false;
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline float AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::GetDataRate()
{
	uint8_t h = ReadByteFromReg(2, 1);
	uint8_t l = ReadByteFromReg(3, 1);
	uint16_t code = ((h & 0x0F) << 8) | l;
	float data_rate = 2457600.F / 128 / code;
	return data_rate;
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline bool AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::isBipolarMode()
{
	return bipolar_mode;
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline bool AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::isHighResolutionMode()
{
	return high_resulution;
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline bool AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::isTestMode()
{
	return testmode;
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline void AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::SetTestMode(bool TestMode)
{
	testmode = TestMode;
	UpdateModeReg();
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline uint8_t AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::GetMode()
{
	return mode;
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline void AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::ADCWait()
{

	while (DRDY_line::Read() != 0)
		Delay_ms(1);

}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline void AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::SelectChannel(uint8_t channel)
{
	AD7714::channel = channel;
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline int32_t AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::ReadResult()
{
	int32_t r = ReadByteFromReg(5, high_resulution ? 3 : 2);
	if (high_resulution && bipolar_mode)
		r = r - 0x800000;
	return r;
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline void AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::CalibrateDisable()
{
	mode = 0;
	UpdateModeReg();
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline void AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::CalibrateAuto()
{
	//mode reg
	mode = 1;
	UpdateModeReg();
	//waiting
	ADCWait();
	//Task::Sleep(3000);
	mode = 0;
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline void AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::CalibrateSystemZero()
{
	mode = 2;
	UpdateModeReg();
	ADCWait();
	mode = 0;
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline void AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::CalibrateSystemScale()
{
	mode = 3;
	UpdateModeReg();
	ADCWait();
	mode = 0;
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline void AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::CalibrateSystemOffset()
{
	mode = 4;
	UpdateModeReg();
	ADCWait();
	mode = 0;
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline void AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::CalibrateBackgroundEnable()
{
	mode = 5;
	UpdateModeReg();
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline void AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::CalibrateAutoZero()
{
	mode = 6;
	UpdateModeReg();
	ADCWait();
	mode = 0;
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
inline void AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::CalibrateAutoScale()
{
	mode = 7;
	UpdateModeReg();
	ADCWait();
	mode = 0;
}
