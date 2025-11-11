#include "AD7714_PIO.h"
//#include "spi.h"
/*
void AD7714::Init(IoBit & MISO_line, IoBit & MOSI_line, IoBit & SCK_line, IoBit & CS_line, IoBit & RESET_line, IoBit & DRDY_line)
{
	channel = 0;
	mode = 0;
	gain = 0;
	testmode = false;
	high_resulution = false;

	AD7714::CS_line = &CS_line;
	AD7714::RESET_line = &RESET_line;
	AD7714::DRDY_line = &DRDY_line;
	AD7714::MISO_line = &MISO_line;
	AD7714::MOSI_line = &MOSI_line;
	AD7714::SCK_line = &SCK_line;
	CS_line.SetMode(PORT_Mode_Out_PP,PORT_Speed_10MHz,true);
	DRDY_line.SetMode(PORT_Mode_IN_FLOATING);
	RESET_line.SetMode(PORT_Mode_Out_PP, PORT_Speed_10MHz, true);
	MISO_line.SetMode(PORT_Mode_IN_FLOATING);
	MOSI_line.SetMode(PORT_Mode_Out_PP, PORT_Speed_10MHz, true);
	SCK_line.SetMode(PORT_Mode_Out_PP, PORT_Speed_10MHz, true);
}

void AD7714::WriteByteToReg(UInt32 data, UInt8 reg, UInt8 len)
{
	SCK_line->Reset();
	CS_line->Reset();
	//запись в регистр обмена
	TX_Byte((reg << 4) | (0 << 3) | channel);
	while(len--)
	{
		TX_Byte(data >> (len * 8));
	}
	CS_line->Set();
}

UInt32 AD7714::ReadByteFromReg(UInt8 reg, UInt8 len)
{
	UInt32 data = 0;
	
	SCK_line->Reset();
	CS_line->Reset();
	//запись в регистр обмена
	TX_Byte((reg << 4) | (1 << 3) | channel);
	while(len--)
	{
		data |= ((UInt32)TX_Byte(0xFF) << (len * 8));
	}
	CS_line->Set();
	return data;
}

void AD7714::ADCReset()
{
	//аппаратный сброс АЦП
	if (RESET_line != NULL)
	{
		RESET_line->Reset();
		Task::Sleep(1);
		RESET_line->Set();
		Task::Sleep(1);
	}
	else
	{
		SCK_line->Reset();
		CS_line->Reset();
		//запись едениц
		for (UInt8 i = 0; i < 5; i++)
			TX_Byte(0xFF);
		//	spi->Wait();
		//	SetCS();
		CS_line->Set();
	}
	//запись значений
	WriteByteToReg(0, 1, 1);
	gain = 0;
	mode = 0;
	testmode = false;
}

void AD7714::SelectChannel(UInt8 channel)
{
	AD7714::channel = channel;
	//ReadByteFromReg(0,1);
}

bool AD7714::isBipolarMode()
{
	return bipolar_mode;
}

bool AD7714::isHighResolutionMode()
{
	return high_resulution;
}

template<class MISO_line, class MOSI_line, class SCK_line, class CS_line, class RESET_line, class DRDY_line>
void AD7714<MISO_line, MOSI_line, SCK_line, CS_line, RESET_line, DRDY_line>::Init()
{

}

void AD7714::SetFilter(bool BipolarMode, bool HighResolutionMode, UInt16 DataRate)
{
	UInt16 FS = 2457600 / 128 / DataRate;
	UInt8 h = (BipolarMode ? 0 : 0x80) | (HighResolutionMode ? 0x40 : 0) | 0x20 | ((FS & 0xF00) >> 8);
	UInt8 l = FS & 0x00FF;
	WriteByteToReg(h,2,1);
	WriteByteToReg(l,3,1);
	high_resulution = HighResolutionMode;
	bipolar_mode = BipolarMode;
}

void AD7714::SetGain(UInt8 gain)
{
	AD7714::gain = gain;
	UpdateModeReg();
}

float AD7714::GetDataRate()
{
	UInt8 h = ReadByteFromReg(2,1);
	UInt8 l = ReadByteFromReg(3,1);
	UInt16 code  = ((h & 0x0F) << 8) | l;
	float data_rate = 2457600.F / 128 / code;
	return data_rate;
}

bool AD7714::isTestMode()
{
	return testmode;
}

void AD7714::SetTestMode(bool TestMode)
{
	testmode = TestMode;
	UpdateModeReg();
}

void AD7714::CalibrateAuto()
{
	//регистр режима
	mode = 1;
	UpdateModeReg();
	//ожидание завершения
	ADCWait();
	//Task::Sleep(3000);
	mode = 0;
}

inline void AD7714::UpdateModeReg()
{
	WriteByteToReg((mode << 5) | (gain << 2) | (testmode ? 2 : 0),1,1);
}

//ожидание на ацп
void AD7714::ADCWait()
{
	while(DRDY_line->GetValue() != 0)
		Task::Sleep(1);
}

void AD7714::CalibrateSystemZero()
{
	mode = 2;
	UpdateModeReg();
	ADCWait();
	mode = 0;
}

void AD7714::CalibrateSystemScale()
{
	mode = 3;
	UpdateModeReg();
	ADCWait();
	mode = 0;
}

void AD7714::CalibrateSystemOffset()
{
	mode = 4;
	UpdateModeReg();
	ADCWait();
	mode = 0;
}

void AD7714::CalibrateBackgroundEnable()
{
	mode = 5;
	UpdateModeReg();
}

void AD7714::CalibrateDisable()
{
	mode = 0;
	UpdateModeReg();
}

void AD7714::CalibrateAutoZero()
{
	mode = 6;
	UpdateModeReg();
	ADCWait();
	mode = 0;
}

void AD7714::CalibrateAutoScale()
{
	mode = 7;
	UpdateModeReg();
	ADCWait();
	mode = 0;
}

UInt8 AD7714::GetMode()
{
	return mode;
}

Int32 AD7714::ReadResult()
{
	Int32 r = ReadByteFromReg(5, high_resulution ? 3 : 2);
	if (high_resulution && bipolar_mode)
		r = r - 0x800000;
	return r;
}

UInt8 AD7714::TX_Byte(UInt8 byte)
{
	UInt8 b = 0;
	for(int i = 7; i >= 0; i--)
	{
		SCK_line->Set();
		MOSI_line->SetValue(byte & (1 << i));
		if(MISO_line->GetValue())
			b |= 1 << i;
		SCK_line->Reset();
	}
	return b;
}

*/
