#include "WH1602.h"

static const char LCD_Table[] =
//таблица для преобразование букв русского алфавита
//начинается с 0xC0
{
	    'A',
		0xA0,
		'B',
		0xA1,
		0xE0,
		'E',
		0xA3,
		0xA4,
		0xA5,
		0xA6,
		'K',
		0xA7,
		'M',
		'H',
		'O',
		0xA8,
		'P',
		'C',
		'T',
		0xA9,
		0xAA,
		'X',
		0xE1,
		0xAB,
		0xAC,
		0xE2,
		0xAD,
		0xAE,
		'b',
		0xAF,
		0xB0,
		0xB1,
		'a',
		0xB2,
		0xB3,
		0xB4,
		0xE3,
		'e',
		0xB6,
		0xB7,
		0xB8,
		0xB9,
		0xBA,
		0xBB,
		0xBC,
		0xBD,
		'o',
		0xBE,
		'p',
		'c',
		0xBF,
		'y',
		0xE4,
		'x',
		0xE5,
		0xC0,
		0xC1,
		0xE6,
		0xC2,
		0xC3,
		0xC4,
		0xC5,
		0xC6,
		0xC7
};

void WH1602::Init(IoPort &data_port, IoBit &rw_line, IoBit &rs_line, IoBit &e_line, bool half_port)
{
	failure = false;
	_half_port = half_port;
	_data_port = &data_port;
	_e_line = &e_line;
	_rs_line = &rs_line;
	_rw_line = &rw_line;
	data_port.SetMode(PORT_Mode_IN_FLOATING, PORT_InputMode, 0);
	data_port.SetMode(PORT_Mode_Out_PP, PORT_Speed_50MHz, 0);
//	for(int i = 0; i <8; i++)
//	data_port[i].Set();
	rw_line.SetMode(PORT_Mode_Out_PP, PORT_Speed_50MHz, false);
	rs_line.SetMode(PORT_Mode_Out_PP, PORT_Speed_50MHz, false);
	e_line.SetMode(PORT_Mode_Out_PP, PORT_Speed_50MHz, false);
	Task::Sleep(40);
	//_write(0x0);
	_write(0x30);	//function set
	Delay_us(37);
	_write(0x30);	//function set
	Delay_us(37);
	if (half_port)
		_write(0x28);
	else
		_write(0x38);
	_write(0x80);	//display on/off
	Delay_us(37);
	_write(0x01);	//display clear
	Task::Sleep(2);
	_write(0x06);	//entry mode set

	if(!_wait()) return;

	
//	_send_cmd(0x38);	//font, 8-bit, 2 line;
	_send_cmd(0x0F);	//display on
	_send_cmd(0x01);	//display clear
}

void WH1602::SetCursorType(CursorType cursor_type)
{
	UInt8 cmd = 0x0C;
	switch(cursor_type)
	{
	case CURSOR_OFF:
		cmd |= 0;
		break;
	case CURSOR_ON:
		cmd |= 2;
		break;
	case CURSOR_BLINKING:
		cmd |= 3;
		break;
	default:
		break;
	}
	_send_cmd(cmd);
}

void WH1602::_send_cmd(UInt8 cmd)
{
	_wait();
	_rs_line->Reset();
	_write(cmd);
}

void WH1602::	_write(UInt8 data)
{
	_rw_line->Reset();
	if (_half_port)
	{
		Delay_us(1);
		_e_line->Set();
		_data_port->SetMode(PORT_Mode_Out_PP, PORT_Speed_50MHz, (data & 0xF0) >> 4);
		Delay_us(1);
		_e_line->Reset();
		Delay_us(1);
		_e_line->Set();
		_data_port->SetMode(PORT_Mode_Out_PP, PORT_Speed_50MHz, data & 0x0F);
		Delay_us(1);
		_e_line->Reset();
		Delay_us(1);
	}
	else
	{
		Delay_us(1);
		_e_line->Set();
		_data_port->SetMode(PORT_Mode_Out_PP, PORT_Speed_50MHz, data);
		Delay_us(1);
		_e_line->Reset();
		Delay_us(1);
	}
}

UInt8 WH1602::_read()
{
	UInt8 data = 0;
	_rw_line->Set();
	if (_half_port)
	{
		Delay_us(1);
		_data_port->SetMode(PORT_Mode_IN_FLOATING, PORT_InputMode, 0);
		_e_line->Set();
		Delay_us(1);
		data = _data_port->GetValue() << 4;
		_e_line->Reset();
		Delay_us(1);
		_e_line->Set();
		Delay_us(1);
		data |= _data_port->GetValue() & 0x0F;
		_e_line->Reset();
		Delay_us(1);
	}
	else
	{
		Delay_us(1);
		_data_port->SetMode(PORT_Mode_IN_FLOATING, PORT_InputMode, 0);
		_e_line->Set();
		Delay_us(1);
		data = _data_port->GetValue();
		_e_line->Reset();
		Delay_us(1);
	}
	return data;
}

bool WH1602::_wait()
{
	_rs_line->Reset();
	PeriodicTimer t;
	t.Start(1000);
	while(_read() & 0x80)
	{
		if(t.isSet)
		{
			failure = true;
			return false;
		}
	}
	return true;
}

UInt16 WH1602::ColumnCount()
{
	return 16;
}

UInt16 WH1602::RowCount()
{
	return 2;
}

void WH1602::SetCursorPos(UInt16 column, UInt16 row)
{
	_send_cmd(0x80 | (row * 0x40 + column));
}

void WH1602::putchar(char ch)
{
    _wait();
	_rs_line->Set();
    _write(ch < 0xC0 ? ch : LCD_Table[ch - 0xC0]);
}

void WH1602::Clear()
{
	_send_cmd(1);
}
