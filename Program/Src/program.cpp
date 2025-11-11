#include "program.h"
//#include "ILI9341.h"
#include <fonts.h>
#include <StaticButtonKeyboard.h>
#include <gpio_stm32f1.h>
#include "FreeRTOS.h"
#include <delay.h>
#include <Format.h>
#include <string.h>
#include <string>
#include <settings.h>
#include <list>
#include <EditableValue.h>
#include "DateTime.h"
#include "RTC.h"
#include "CIC.h"
#include "AD840X.h"
#include "AD5293.h"
#include "AD7714_PIO.h"
#include "flowcalc.h"
#include "SystemFlash.h"
#include "Buffer.h"
#include "MeasureData.h"
#include "mutex_freertos.h"
#include <atomic>
#include "SPI_PIO.h"
#include "CC1101.h"
#include <main.h>
#include "random.h"
#include <task.h>
#include "BinarySerialize.h"
#include "Packet.h"
#include "rf_protocol.h"
#include <algorithm>
#include <array>
#include "SST26.h"
#include "FlashTranslator2.h"
#include <charconv>

float GetVoltage();

typedef char DayOfWeekName[30];
const DayOfWeekName day_of_week_names[7] = {
	{u8"Понедельник"},
	{u8"Вторник"},
	{u8"Среда"},
	{u8"Четверг"},
	{u8"Пятница"},
	{u8"Суббота"},
	{u8"Воскресенье"}
};

#ifdef LCD_SSD1289
#include "SSD1289.h"
SSD1289<0x60000000, 0x60020000> lcd;
#endif

#ifdef LCD_ILI9340
#include "ILI9340.h"
ILI9340<0x60000000, 0x60020000> lcd;
#endif

#ifdef LCD_ILI9325
#include "ILI9325.h"
ILI9325<0x60000000, 0x60020000> lcd;
#endif

const uint16_t graphic_colors[] = {
	RGB(0, 255, 0),
	RGB(255, 255, 0),
	RGB(0, 255, 255),
	RGB(255, 255, 255),
	RGB(255, 0, 255),
	RGB(255, 0, 0)
};

enum class StateColors : uint16_t
{
	NORMAL = RGB(150, 255, 150),
	DANGER = RGB(255, 150, 150)
};

#ifdef OLD_HW
StaticButtonKeyboard<VirtualPort<CreatePinList<PE4, PE6, PE5, PE2>::Result>, 0x7, true, 10> keyboard;
#else
StaticButtonKeyboard<VirtualPort<CreatePinList<PE4, PE6, PE5, PE2>::Result>, 0xF, true, 10> keyboard;
#endif
//StartScreen start_screen;
//MeasureScreen measure_screen;
//ZaboySelectScreen zaboy_select_screen;
Font<uint16_t> small_font(microsoftSansSerif_8ptFontInfo);
Font<uint16_t> big_font(anonymousPro_14ptFontInfo);
//Font<uint16_t> big_font(microsoftSansSerif_8ptFontInfo);
Settings settings;
StateColors state_color = StateColors::NORMAL;
RTC_DateTime rtc;
volatile bool pot_update = true;
Mutex mutex1;
Mutex rf_mutex;
MeasureData measure_data(&mutex1);
volatile bool measure_in_progress = false;
typedef CC1101<PA3, PA1, PA2, PA0> RF;
//RF_type rf;
XorShift rng;
volatile int pps = 0;
SpiPio<PB8,PB7,PB9,0,ClockPolarity::LOW_IDLE,ClockPhase::BEGIN,BitOrder::MSB_FIRST> spi_flash_bus;
Sst26Memory<PB6, 16, 2048> flash;
FlashTranslator2<16> flash_drive(&flash);



//0x08010000 (P32) to 0x0807FFFF (P255)


extern "C" ADC_HandleTypeDef hadc1;
uint16_t adc_buffer[100];
extern "C" TIM_HandleTypeDef htim8;
//extern "C" IWDG_HandleTypeDef hiwdg;
double adc_t = 0;
double adc_q = 0;
double Q = 0;
double T = 0;
bool pisk = false;

EditableValue measure_id(0, 10000, 0, 1, 0, u8"Номер измерения");

MeasureDB measure_db(flash_drive);

bool rf_tx_enable = false;

float battery_voltage {4};

void rf_send_response(const vector<uint8_t>& buffer);

inline void BacklightEnable()
{
	PC7::Set();
}

inline void BacklightDisable()
{
	PC7::Reset();
}

void main_task()
{
	flash.init(&spi_flash_bus);
	//flash.test();
	//flash_drive.init_table();
	flash_drive.load();
	//flash_drive.ReadSector(3, dbgbuf);
	//strcpy(reinterpret_cast<char*>(dbgbuf), "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	//flash_drive.WriteSector(3,dbgbuf);
	//strcpy(reinterpret_cast<char*>(dbgbuf), "-----------------------------");
	//flash_drive.ReadSector(3,dbgbuf);
	BacklightEnable();
	keyboard.Init(10);
	keyboard.SetTimeout(500);
	lcd.Init();
	//for (;;);
	lcd.Clear();
	lcd.SetTextPos(100, 100);
	lcd.DrawText("ЗАПУСК...", RGB(255, 255, 0), &big_font);

	BacklightEnable();

#ifdef OLD_HW
	while (PE2::Read() == true);
#else
	while (PE2::Read() == false);
#endif
	while (keyboard.ReadKey() != -1);
	keyboard.SetTimeout(5000);
	//GraphicScreen<uint16_t>::Show(unique_ptr<GraphicScreen<uint16_t>>(new StartScreen()), nullptr);
	GraphicScreen<uint16_t>::Show(unique_ptr<GraphicScreen<uint16_t>>(new StartScreen()), nullptr);

	//DEBUG***************************************
	/*GraphData g;
	g.q_graph.name = u8"График q";
	float q = 5;
	for (size_t j = 0; j < 1000; j++)
	{
		q += rng.random_single()-0.5;
		q = std::max(q, float(1));
		q = std::min(q, float(8));
		g.q_graph.insert_q(q);
	}
	g.t_graph.name = u8"График t";
	float t = 20;
	for (size_t j = 0; j < 1000; j++)
	{
		t += rng.random_single()*10 - 5;
		t = std::max(t, float(10));
		t = std::min(t, float(40));
		g.t_graph.insert_t(t);
	}

	GraphicScreen<uint16_t>::Show(make_unique<GraphScreen>(std::move(g)));*/

	//DEBUG***************************************

	for (;;)
	{
		
		GraphicScreen<uint16_t>::Redraw(lcd);
		auto kk = keyboard.ReadKey();
		if ((kk == 0x83)||(battery_voltage < 2.9))
		{
			lcd.Clear();
			lcd.SetTextPos(100, 100);
			lcd.DrawText("ВЫКЛЮЧЕНИЕ...", RGB(255, 255, 0), &big_font);
			//settings.Save();
			for (;;)
			{
				PE1::Reset();
			}
		}
		else if (kk != -1)
		{
			static const uint8_t key_table[4] = { KEY_DOWN,KEY_UP,KEY_ESC,KEY_OK };
			kk &= 0x0F;
			kk = key_table[kk];
			GraphicScreen<uint16_t>::KeyProc(kk);
		}
	}
}

void TairgScreen::onRedraw(Graphic<uint16_t>& display)
{
	uint16_t text_color = 0;
	display.Clear(0xFFFF);
	display.Fill(0, 0, 319, 15, (uint16_t)state_color);
	DateTime time1 = rtc.get_time();
	display.DrawText(time1.to_string_time(false).c_str(), text_color, &small_font);
	display.DrawText(" ", text_color, &small_font);
	display.DrawText(time1.to_string_date().c_str(), text_color, &small_font);
	display.DrawText(" ", text_color, &small_font);
	display.DrawText(day_of_week_names[static_cast<int>(time1.day_of_week())], text_color, &small_font);
	//memory
	auto mem = xPortGetFreeHeapSize();
	ToString mem_str(10);
	mem_str.Convert(mem);
	display.DrawText(" M:", text_color, &small_font);
	display.DrawText(mem_str.LeftAllign(), text_color, &small_font);
	//battery
	
	battery_voltage = GetVoltage();
	int percent = (battery_voltage - 3) * 100;
	if (percent < 0) percent = 0;
	if (percent > 99) percent = 99;
	int w = percent * 25 / 100;
	uint16_t c = percent < 20 ? RGB(255, 0, 0) : RGB(0, 100, 0);
	int x0 = 263;
	int y0 = 3;
	display.Rectangle(x0, y0, x0 + 25, y0 + 10, c);
	display.Rectangle(x0 + 25, y0 + 3, x0 + 28, y0 + 7, c);
	display.Fill(x0, y0, x0 + w, y0 + 10, c);
	display.SetTextPos(295, 2);
	ToString pc_string(2);
	pc_string.Convert(percent, true);
	display.DrawText(pc_string.RightAllign(), text_color, &small_font);
	display.DrawText("%", text_color, &small_font);

	//wifi
	constexpr int x_wifi = 250;
	constexpr int y_wifi = 12;
	display.Fill(x_wifi - 1, y_wifi - 1, x_wifi + 1, y_wifi + 1, 0);
	for (size_t i = 0; i < 3; i++)
	{
		display.Arc(x_wifi, y_wifi, (i + 1) * 3, 50, 130, 0);
	}

	draw_tairg_content(display);
}

void TairgScreen::draw_tairg_content(Graphic<uint16_t>& display)
{
	display.SetTextPos(0, 20);
	display.DrawText("Контент", 0, &big_font);
}


void ShowResultDigitalData::onKeyPress(char key)
{
	switch (key)
	{
	case KEY_UP:
	case KEY_DOWN:
	case KEY_OK:
	{
		Show(make_shared<BoringPlaceShowScreen>(measure_data.boring_pos));
	}
	break;
	case KEY_ESC:
		Close();
		break;
	}
}

void ShowResultDigitalData::draw_tairg_content(Graphic<uint16_t>& display)
{
	ToString val(4);
	auto title = u8"Результаты измерения";
	auto font = &big_font;
	display.SetTextPos((display.Width() - font->GetTextWidth(title)) / 2, 20);
	display.DrawText(title, 0, &big_font);

	display.Rectangle(10, 40, 310, 130, 0);
	display.Line(10, 70, 310, 70, 0);

	display.SetTextPos(30, 40);
	display.DrawText(u8"                ср.   ср.кв.  макс.  мин.", 0, &big_font);

	display.SetTextPos(20, 75);
	display.DrawText(u8"Q(л/м):", 0, &big_font);
	display.SetTextPos(90, 75);

	val.Convert(measure_data.median_q, 1, false);
	display.DrawText(val.RightAllign(), 0, &big_font);
	display.DrawText(u8"     ", 0, &big_font);

	val.Convert(measure_data.sqr_q, 1, false);
	display.DrawText(val.RightAllign(), 0, &big_font);
	display.DrawText(u8"     ", 0, &big_font);

	val.Convert(measure_data.max_q, 1, false);
	display.DrawText(val.RightAllign(), 0, &big_font);
	display.DrawText(u8"     ", 0, &big_font);

	val.Convert(measure_data.min_q, 1, false);
	display.DrawText(val.RightAllign(), 0, &big_font);

	display.SetTextPos(20, 95);
	display.DrawText(u8"T( C):", 0, &big_font);
	display.SetTextPos(90, 95);

	val.Convert(measure_data.median_t, 1, false);
	display.DrawText(val.RightAllign(), 0, &big_font);
	display.DrawText(u8"     ", 0, &big_font);

	val.Convert(measure_data.sqr_t, 1, false);
	display.DrawText(val.RightAllign(), 0, &big_font);
	display.DrawText(u8"     ", 0, &big_font);

	val.Convert(measure_data.max_t, 1, false);
	display.DrawText(val.RightAllign(), 0, &big_font);
	display.DrawText(u8"     ", 0, &big_font);

	val.Convert(measure_data.min_t, 1, false);
	display.DrawText(val.RightAllign(), 0, &big_font);

	display.SetTextPos(20, 140);
	val.Convert(measure_data.shpur_lenght, 1, false);
	display.DrawText(u8"Глубина шпура: ", 0, &big_font);
	display.DrawText(val.LeftAllign(), 0, &big_font);
	display.DrawText(u8" м.", 0, &big_font);

	display.SetTextPos(20, 160);
	val.Convert(measure_data.shtyb_amount, 1, false);
	display.DrawText(u8"Количество штыба: ", 0, &big_font);
	display.DrawText(val.LeftAllign(), 0, &big_font);
	display.DrawText(u8" л.", 0, &big_font);

	display.SetTextPos(20, 180);
	val.Convert(measure_data.median_q, 1, false);
	display.DrawText(u8"R=(", 0, &big_font);
	display.DrawText(val.LeftAllign(), 0, &big_font);
	display.DrawText(u8"-4)*(", 0, &big_font);
	val.Convert(measure_data.shtyb_amount, 1, false);
	display.DrawText(val.LeftAllign(), 0, &big_font);
	display.DrawText(u8"-1.8)-6=", 0, &big_font);
	float R = (measure_data.median_q - 4) * (measure_data.shtyb_amount - 1.8) - 6;
	val.Convert(R, 1, false);
	display.DrawText(val.LeftAllign(), 0, &big_font);
	
	display.SetTextPos(20, 200);
	display.DrawText(u8"Вывод:", 0, &big_font);
	if(R<0)
		display.DrawText(u8" НЕ ОПАСНО.", RGB(0,128,0), &big_font);
	else
		display.DrawText(u8" ОПАСНО!", RGB(128, 0, 0), &big_font);

}

void ShowResultDigitalData::onShow()
{
	measure_data.calc_stat();
}

ShowResultStat::ShowResultStat(MeasureHeader measure_header)
	:_measure_header{ measure_header }
{
}

void ShowResultStat::onKeyPress(char key)
{
	switch (key)
	{
	case KEY_ESC:
		Close();
		break;
	case KEY_OK:
	case KEY_UP:
	case KEY_DOWN:
		Show(make_shared<ShowResultDigitalData>());
		break;
	default:;
	}
}

void ShowResultStat::draw_tairg_content(Graphic<uint16_t>& display)
{
	//int tabx = 10;
	const int offset_x = 25;
	const int step_y = 20;
	const int tabx1 = 150+offset_x;
	const int right = 250+offset_x;
	const int left = 6+offset_x;
	int y = 50;
	const int n_lines = 8;

	display.Line(left, y, left, step_y * n_lines + y, RGB(0, 0, 0));
	display.Line(tabx1-2, y, tabx1-2, step_y * n_lines + y, RGB(0, 0, 0));
	display.Line(right, y, right, step_y * n_lines + y, RGB(0, 0, 0));

	for(int i = 0; i <= n_lines; i++)
	{
		display.Line(left, y + i * step_y, right, y + i * step_y, RGB(0, 0, 0));
	}
	
	ToString val{ 10 };
	const char* title;
	display.SetTextPos(20, 25);
	display.DrawText(u8"  Общие сведения об измерении:", RGB(0, 0, 0), &big_font);
	
	title = u8"Номер: ";
	display.SetTextPos(tabx1-big_font.GetTextWidth(title), y);
	display.DrawText(title, RGB(0, 0, 0), &big_font);
	val.Convert(_measure_header.id);
	display.DrawText(val.LeftAllign(), RGB(0, 0, 0), &big_font);

	title = u8"Дата: ";
	y += step_y;
	display.SetTextPos(tabx1 - big_font.GetTextWidth(title), y);
	display.DrawText(title, RGB(0, 0, 0), &big_font);
	display.DrawText(DateTime{ _measure_header.time_mark }.to_string_date().c_str(), RGB(0, 0, 0), &big_font);

	title = u8"Время: ";
	y += step_y;
	display.SetTextPos(tabx1 - big_font.GetTextWidth(title), y);
	display.DrawText(title, RGB(0, 0, 0), &big_font);
	display.DrawText(DateTime{ _measure_header.time_mark }.to_string_time().c_str(), RGB(0, 0, 0), &big_font);

	title = u8"Длительность: ";
	y += step_y;
	display.SetTextPos(tabx1 - big_font.GetTextWidth(title), y);
	display.DrawText(title, RGB(0, 0, 0), &big_font);
	val.Convert(double{ static_cast<double>(_measure_header.count_of_points) } / 10, 1, false);
	display.DrawText(val.LeftAllign(), RGB(0, 0, 0), &big_font);

	title = u8"Гл. шпура: ";
	y += step_y;
	display.SetTextPos(tabx1 - big_font.GetTextWidth(title), y);
	display.DrawText(title, RGB(0, 0, 0), &big_font);
	val.Convert(_measure_header.shpur, 1, false);
	display.DrawText(val.LeftAllign(), RGB(0, 0, 0), &big_font);
	display.DrawText(u8" м.", RGB(0, 0, 0), &big_font);

	title = u8"Кол. штыба: ";
	y += step_y;
	display.SetTextPos(tabx1 - big_font.GetTextWidth(title), y);
	display.DrawText(title, RGB(0, 0, 0), &big_font);
	val.Convert(_measure_header.shtyb, 1, false);
	display.DrawText(val.LeftAllign(), RGB(0, 0, 0), &big_font);
	display.DrawText(u8" л.", RGB(0, 0, 0), &big_font);

	title = u8"Забой: ";
	y += step_y;
	display.SetTextPos(tabx1 - big_font.GetTextWidth(title), y);
	display.DrawText(title, RGB(0, 0, 0), &big_font);
	display.DrawText(settings.zaboy_list[_measure_header.zaboy_id].name, RGB(0, 0, 0), &big_font);

	title = u8"Полож. шпура: ";
	y += step_y;
	display.SetTextPos(tabx1 - big_font.GetTextWidth(title), y);
	display.DrawText(title, RGB(0, 0, 0), &big_font);
	val.Convert(_measure_header.boring_pos);
	display.DrawText(val.LeftAllign(), RGB(0, 0, 0), &big_font);
}

void ResultList::onKeyPress(char key)
{
	switch (key)
	{
	case KEY_UP:
		if (_pos != _entries.begin())
		{
			--_pos;
			if (_pos < _offset)
				_offset = _pos;
		}
		break;

	case KEY_DOWN:
		if (_pos != _entries.end())
		{
			++_pos;
			if (_pos - _offset >= count_in_page)
			{
				_offset = _pos - count_in_page;
			}
		}
		break;

	case KEY_OK:
		if (_pos == _entries.end())
			Close();
		else
		{
			//_pos->Show();
			measure_data = move(measure_db.read_data(_pos->index()));
			//measure_data.calc_stat();
			Show(make_shared<ShowResultStat>(*_pos));
			++_pos;
			if (_pos - _offset >= count_in_page)
			{
				_offset = _pos - count_in_page;
			}
		}
		break;

	case KEY_ESC:
		Close();
		break;
	}
}

void ResultList::draw_tairg_content(Graphic<uint16_t>& display)
{
	constexpr int heaght = 180;
	constexpr int top = 20;
	constexpr int tab = 33;
	const auto font = &big_font;
	constexpr auto text_color = RGB(0, 0, 0);
	constexpr auto selected_text_color = RGB(0, 128, 0);
	const int step = 20;
	const auto count_in_page = heaght / step;
	const auto title = "Результаты измерений";
	const auto no_data_text = "Данных нет";

	display.SetTextPos((display.Width() - font->GetTextWidth(title)) / 2, top);
	display.DrawText(title, text_color, font);

	display.Rectangle(10, top + font->Heaght(), display.Width() - 10, top + heaght, RGB(100, 100, 100));

	if(_entries.empty())
	{
		display.SetTextPos((display.Width() - font->GetTextWidth(no_data_text)) / 2, top+3*step);
		display.DrawText(no_data_text, text_color, font);
	}
	else
	{
		auto it = _offset;
		int y = top;

		while ((it != _entries.end()) && (y < (heaght + top)))
		{
			y += step;
			display.SetTextPos(tab, y);
			auto color = text_color;
			if (_pos == it)
			{
				display.Circle(20, y + step / 2 - 2, 5, text_color);
				color = selected_text_color;
			}

			display.DrawText(it->to_string().c_str(), color, font);

			/*
			display.DrawText(it->title(), color, font);

			display.DrawText("  ", color, font);

			display.DrawText(it->result_string(), RGB(255, 0, 0), font);
			*/
			++it;
		}
	}
}

void ResultList::onShow()
{
	_entries = move(measure_db.load_headers());
	if (_entries.size() < count_in_page)
		_offset = _entries.begin();
	else
	{
		_offset = _entries.end();
		for (size_t i = 0; i < count_in_page; i++)
		{
			--_offset;
		}
	}
	_pos = _entries.end();
	if (!_entries.empty())
		--_pos;
}

void PositionSelectScreen::onKeyPress(char key)
{
	switch (key)
	{
	case KEY_DOWN:
		if (selected_pos < (n_x * n_y))
			selected_pos++;
		break;
	case KEY_UP:
		if (selected_pos > 0)
			selected_pos--;
		break;
	case KEY_OK:
		measure_data.boring_pos = selected_pos;
		if (_param)
		{
			const int x = selected_pos % n_x;
			const int y = selected_pos / n_x;
			ToString str_x(2);
			str_x.Convert(x);
			ToString str_y(2);
			str_y.Convert(y);
			auto str = reinterpret_cast<string*>(_param);
			str->operator=("X:");
			str->append(str_x.LeftAllign());
			str->append(" Y:");
			str->append(str_y.LeftAllign());
		}
		Close();
		break;
	case KEY_ESC:
		Close();
		break;
	}
}

void PositionSelectScreen::draw_tairg_content(Graphic<uint16_t>& display)
{
	constexpr int width = 300;
	constexpr int height = 200;
	constexpr int top = 30;
	const int tab = (display.Width() - width) / 2;
	const int step_x = width / n_x;
	const int step_y = height / n_y;

	display.Rectangle(tab, top, tab + width, top + height, 0);

	for (int i = 1; i < n_x; i++)
	{
		const int x = tab + i * step_x;
		display.Line(x, top, x, top + height, 0);
	}

	for (int i = 1; i < n_y; i++)
	{
		const int y = top + i * step_y;
		display.Line(tab, y, tab + width, y, 0);
	}

	int x = selected_pos % n_x;
	int y = selected_pos / n_x;

	x *= step_x;
	x += tab;

	y *= step_y;
	y += top;

	//display.Circle(x, y, 10, RGB(255, 0, 0));
	display.Fill(x, y, x + step_x, y + step_y, RGB(255, 0, 0));
}

WizardEntry::WizardEntry(shared_ptr<TairgScreen>&& tairg_screen, const char* const title) :
	_screen(move(tairg_screen)),
	_title(title)
{
}

WizardEntry::WizardEntry(const WizardEntry& other) :
	_screen(other._screen),
	_title(other._title)
{
}


WizardEntry::WizardEntry(WizardEntry&& other) noexcept :
	_screen(move(other._screen)),
	_title(other._title)
{
}

WizardEntry& WizardEntry::operator=(WizardEntry&& other) noexcept
{
	_screen = move(other._screen);
	_title = other._title;
	return *this;
}

const char* WizardEntry::title() const
{
	return _title;
}

const char* WizardEntry::result_string() const
{
	return response.c_str();
}

void WizardEntry::Show()
{
	if (_screen)
		GraphicScreen<uint16_t>::Show(_screen, &response);
}

WizardScreen::WizardScreen(initializer_list<WizardEntry> entries, const char* title) :
	_entries(entries),
	_title(title)
{
	offset = pos = _entries.begin();
}

void WizardScreen::onKeyPress(char key)
{
	switch (key)
	{
	case KEY_UP:
		if (pos != _entries.begin())
		{
			--pos;
			if (pos < offset)
				offset = pos;
		}
		break;

	case KEY_DOWN:
		if (pos != _entries.end())
		{
			++pos;
			if (pos - offset >= count_in_page)
			{
				offset = pos - count_in_page;
			}
		}
		break;

	case KEY_OK:
		if (pos == _entries.end())
			Close();
		else
		{
			pos->Show();
			++pos;
			if (pos - offset >= count_in_page)
			{
				offset = pos - count_in_page;
			}
		}
		break;

	case KEY_ESC:
		Close();
		break;
	}
}

void WizardScreen::onShow()
{
}

void WizardScreen::onClose()
{
}

void WizardScreen::draw_tairg_content(Graphic<uint16_t>& display)
{
	constexpr int heaght = 180;
	constexpr int top = 20;
	constexpr int tab = 33;
	const auto font = &big_font;
	constexpr auto text_color = RGB(0, 0, 0);
	constexpr auto selected_text_color = RGB(0, 128, 0);
	const int step = 20;
	const auto count_in_page = heaght / step;

	display.SetTextPos((display.Width() - font->GetTextWidth(_title)) / 2, top);
	display.DrawText(_title, text_color, font);

	display.Rectangle(10, top + font->Heaght(), display.Width() - 10, top + heaght, RGB(100, 100, 100));

	auto it = offset;
	int y = top;

	while ((it != _entries.end()) && (y < (heaght + top)))
	{
		y += step;
		display.SetTextPos(tab, y);
		auto color = text_color;
		if (pos == it)
		{
			display.Circle(20, y + step / 2 - 2, 5, text_color);
			color = selected_text_color;
		}

		display.DrawText(it->title(), color, font);

		display.DrawText("  ", color, font);

		display.DrawText(it->result_string(), RGB(255, 0, 0), font);


		++it;
	}
}

float GetVoltage()
{
	volatile uint64_t out_samples[15];
	int out_sample_count = 0;
	CIC::Decimator<uint64_t, 2, 3> dec1, dec2;
	for (size_t i = 0; i < 100; i++)
	{
		dec1.InSample(adc_buffer[i]);
		if (dec1.IsOutSampleReady())
		{
			dec2.InSample(dec1.GetOutSample());
			if (dec2.IsOutSampleReady())
			{
				out_samples[out_sample_count++] = dec2.GetOutSample();
			}
		}
	}
	float result = 0;
	for (int i = 4; i < 11; i++)
	{
		result += out_samples[i];
	}
	result /= 7;
	result /= dec1.gain();
	result /= dec2.gain();
	result /= 0x1000;
	result *= 3.3 * 2;
	return result;
}

void init()
{
	delay_system_init();
	settings.Init();
	settings.Load();
	rtc.init();
	HAL_TIM_Base_Start(&htim8);
	HAL_ADC_Start_DMA(&hadc1, (unsigned long*)adc_buffer, 100);
}

void kbtask()
{
	static int p = 0;
	keyboard.Scan();
	PE0::Reset();
	Delay_us(100);
	PE0::Set();
	HAL_IWDG_Refresh(&hiwdg);
	if(pisk)
		PD3::Toggle();
	else
		PD3::Reset();
}

void StartScreen::draw_tairg_content(Graphic<uint16_t>& display)
{
	//display.Clear(0xFFFF);
	display.SetTextPos(65, 40);
	display.DrawText("CoalAwakeningBeast", 0, &big_font);

	auto time = rtc.get_time();
	display.SetTextPos(130, 80);
	display.DrawText(time.to_string_date().c_str(), 0, &big_font);
	display.SetTextPos(130, 100);
	display.DrawText(time.to_string_time().c_str(), 0, &big_font);
	display.SetTextPos(65, 200);
	display.DrawText("ВЫБЕРИТЕ ФУНКЦИЮ", RED_MASK, &big_font);
}

void StartScreen::onKeyPress(char key)
{
	switch (key)
	{
	case KEY_UP:
	{
		//Show(unique_ptr<GraphicScreen<uint16_t>>(new ZaboySelectScreen), nullptr);
		Show(make_unique<WizardScreen>(WizardScreen(
			{
				WizardEntry(std::make_shared<ZaboySelectScreen>(), u8"Выбор забоя"),
				WizardEntry(std::make_shared<PositionSelectScreen>(), u8"Выбор места"),
				WizardEntry(std::make_shared<ShtybScreen>(), u8"Доп. параметры"),
				WizardEntry(std::make_shared<MeasureScreen>(), u8"Измерение")
			}, u8"Начальное газовыделение")));
	}
	break;
	case KEY_OK:
	{
		//Show(std::make_unique<TestAdcScreen>(), nullptr);
		Show(make_unique<WizardScreen>(WizardScreen(
			{
				WizardEntry(std::make_shared<RfTestScreen>(), u8"Радиоканал"),
				WizardEntry(std::make_shared<PasswordEnterScreen>(), u8"Ввод пароля разблокировки"),
				WizardEntry(std::make_shared<TimeSetupScreen>(rtc.get_time()), u8"Установка времени"),
				WizardEntry(std::make_shared<TestAdcScreen>(), u8"Настройка потенциометров"),
				WizardEntry(std::make_shared<FlashMemoryScreen>(), u8"Хранилище"),
			}, u8"Служебное меню")));
	}
	break;
	case KEY_ESC:
		Show(make_shared<ResultList>());
		break;
	}
}



void MeasureScreen::draw_tairg_content(Graphic<uint16_t>& display)
{
	const int width = 300;
	const int height = 200;
	const int top = 20;
	auto title = u8"Измерение расхода!";
	auto font = &big_font;

	keyboard.SetTimeout(1000);

	ToString q_string(4);
	ToString t_string(4);

	if (measure_in_progress)
	{
		const auto m = measure_data.last_n_median();
		q_string.Convert(m.q, 1, false);
		t_string.Convert(m.t, 1, false);
		const float r = (m.q - 4) * (measure_data.shtyb_amount - 1.8) - 6;
		pisk = r > 0;
	}
	else
	{
		q_string.Convert(Q, 1, false);
		t_string.Convert(T, 1, false);
	}

	display.SetTextPos((display.Width() - font->GetTextWidth(title)) / 2, top);
	display.DrawText(title, 0, &big_font);

	display.SetTextPos(10, top + 40);
	display.DrawText(u8"Расход:", 0, &big_font);
	display.Rectangle(80, top + 40, 120, top + 40 + font->Heaght(), RGB(100, 100, 100));
	display.SetTextPos(81, top + 40);
	display.DrawText(q_string.RightAllign(), 0, &big_font);

	display.SetTextPos(130, top + 40);
	display.DrawText(u8"Температура:", 0, &big_font);
	display.Rectangle(250, top + 40, 300, top + 40 + font->Heaght(), RGB(100, 100, 100));
	display.SetTextPos(251, top + 40);
	display.DrawText(t_string.RightAllign(), 0, &big_font);


	display.Rectangle(10, top + 100, 310, top + 120, RED_MASK);
	float percent = measure_data.count_of_points() / 1000.0;
	if (percent > 1)
	{
		percent = 1;
		exit();
	}
	display.Fill(10, top + 100, (display.Width() - 20) * percent + 10, top + 120, RED_MASK);

	display.SetTextPos(20, top + 130);

	if (!measure_in_progress)
	{
		display.DrawText(u8"НАЖМИТЕ КНОПКУ 4 ДЛЯ ЗАПУСКА", RED_MASK, &small_font);
	}
	else
	{
		display.DrawText(u8"НАЖМИТЕ КНОПКУ 4 ДЛЯ ОСТАНОВКИ", RED_MASK, &small_font);
	}

	
}

void MeasureScreen::onKeyPress(char key)
{
	switch (key)
	{
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_OK:
		if (measure_in_progress)
		{
			exit();
		}
		else
		{
			measure_in_progress = true;
			measure_data.reset();
			measure_data.start_time = rtc.get_time();
		}
		break;
	case KEY_ESC:
		measure_in_progress = false;
		Close();
		break;
	}
}

void MeasureScreen::exit()
{
	measure_in_progress = false;
	if (_param)
	{
		ToString n_str(3);
		n_str.Convert(measure_data.count_of_points(), false);
		auto str = reinterpret_cast<string*>(_param);
		str->operator=("N=");
		str->append(n_str.LeftAllign());
	}
	Close();
	TairgScreen::Show(make_shared<ShowResultDigitalData>());
	measure_db.write_data(measure_data);
	measure_id.increase();
}

void MeasureScreen::onClose()
{
	pisk = false;
}


void ZaboySelectScreen::draw_tairg_content(Graphic<uint16_t>& display)
{
	//display.Clear(0xFFFF);
	display.SetTextPos(80, 20);
	display.DrawText("ВЫБОР ЗАБОЯ", 0, &big_font);
	display.Rectangle(10, 40, 300, 200, RGB(200, 200, 200));
	display.SetTextPos(10, 200);
	display.DrawText("ВЫБЕРИТЕ ЗАБОЙ КНОПКАМИ 1 и 2", RED_MASK, &small_font);
	display.SetTextPos(10, 220);
	display.DrawText("ПОДТВЕРДИТЕ ВЫБОР КНОПКОЙ 4", RED_MASK, &small_font);
	for (int i = 0; i < 10; i++)
	{
		if ((i + list_offset) > settings.zaboy_list.size())
			break;
		display.SetTextPos(20, 40 + 17 * i);
		uint16_t color = 0;
		if (i + list_offset == selected)
		{
			color = RGB(0, 180, 0);
			display.Rectangle(10, 40 + 17 * i, 300, 40 + 17 * i + 17, color);
		}
		display.DrawText(settings.zaboy_list[i + list_offset].name, color, &big_font);
	}
}

void ZaboySelectScreen::onKeyPress(char key)
{
	switch (key)
	{
	case KEY_DOWN:
	{
		selected++; //next ithem
		if (selected >= settings.zaboy_list.size()) //overflow
			selected = 0;
		if ((selected - list_offset) > 9) //scroll down
			list_offset = selected - 9;
		if (selected < list_offset) //scroll up
			list_offset = selected;
	}
	break;
	case KEY_OK:
	{
		Close();
		//EnterDigitalValueScreen * scr = new EnterDigitalValueScreen("Введите количество штыба", { &shtyb_05, &shtyb_15 });
		//Show(std::unique_ptr<EnterDigitalValueScreen>(scr));
		//Show(unique_ptr<EnterDigitalValueScreen>(
		//new EnterDigitalValueScreen("ВВЕДИТЕ КОЛИЧЕСТВО ШТЫБА", 4, {&shtyb_05, &shtyb_10, &shtyb_15, &shtyb_20})));
	}
	break;
	}
	if (_param)
	{
		*reinterpret_cast<string*>(_param) = settings.zaboy_list[selected].name;
	}
}


void EnterDigitalValueScreen::draw_tairg_content(Graphic<uint16_t>& display)
{
	int work_area_heaght = lcd.Height() - big_font.Heaght() * 2 - 40;
	int work_area_top = big_font.Heaght() * 2 + 10;
	int work_area_space = work_area_heaght / _rows_on_screen;

	//display.Clear(0xFFFF);
	volatile int width = display.Width();
	volatile int text_width = big_font.GetTextWidth(_title);
	display.SetTextPos((width - text_width) / 2, big_font.Heaght() / 2 + 8);
	display.DrawText(_title, 0, &big_font);
	//need_to_clear = false;
	lcd.Rectangle(10, work_area_top, display.Width() - 10, work_area_top + work_area_heaght, RGB(200, 200, 200));

	int current_page = _current_index / _rows_on_screen;
	for (int i = 0; i < _rows_on_screen; i++)
	{
		const size_t index = current_page * _rows_on_screen + i;
		const int row_x = work_area_top + work_area_space / 2 - big_font.Heaght() / 2 + i * work_area_space;
		if (index < _value_list.size())
		{
			const uint16_t color = index == _current_index ? RGB(0, 128, 0) : RGB(0, 0, 0);
			display.SetTextPos(20, row_x);
			display.DrawText(_value_list[index]->_message, color, &big_font);
			display.Rectangle(display.Width() - 100, row_x, display.Width() - 20, row_x + big_font.Heaght(), color);
			display.SetTextPos(display.Width() - 95, row_x);
			display.DrawText(_value_list[index]->to_string().c_str(), color, &big_font);
		}
	}
}

void EnterDigitalValueScreen::on_edit_complete()
{
}

void EnterDigitalValueScreen::onKeyPress(char key)
{
	switch (key)
	{
	case KEY_UP:
		_value_list[_current_index]->increase();
		break;
	case KEY_DOWN:
		_value_list[_current_index]->decrease();
		break;
	case KEY_OK:
		_current_index++;
		if (_current_index >= _value_list.size())
		{
			on_edit_complete();
			Close();
		}
		break;
	case KEY_ESC:
		_current_index--;
		if (_current_index < 0)
			Close();
		break;
	}
}

void EnterDigitalValueScreen::onShow()
{
	need_to_clear = true;
	_current_index = 0;
}

ShtybScreen::ShtybScreen() :
	EnterDigitalValueScreen(u8"Количество штыба", 3, { &shpur_deep, &shtyb_value , &measure_id}),
	shpur_deep(0, 50, 0, 5, 1, u8"Глубина шпура, м"),
	shtyb_value(0, 100, 0, 1, 1, u8"Количество штыба, л")
{
}

void ShtybScreen::onKeyPress(char key)
{
	EnterDigitalValueScreen::onKeyPress(key);
	if (_param)
	{
		auto str = reinterpret_cast<string*>(_param);
		str->operator=(u8"Гл.:");
		str->append(shpur_deep.to_string());
		str->append(u8" Кол.ш.:");
		str->append(shtyb_value.to_string());

		measure_data.shtyb_amount = shtyb_value.to_float();
		measure_data.shpur_lenght = shpur_deep.to_float();
		measure_data.id = measure_id.get();
	}
}

void TestAdcScreen::onKeyPress(char key)
{
	switch (key)
	{
	case KEY_OK:
		if (pot_index < 3)
			pot_index++;
		else
		{
			settings.Save();
			Close();
		}
		break;

	case KEY_ESC:
		if (pot_index > 0)
			pot_index--;
		else
			Close();
		break;

	case KEY_DOWN:
		*pots_values[pot_index] += 0.001;
		if (*pots_values[pot_index] > 1)
			* pots_values[pot_index] = 1;
		break;

	case KEY_UP:
		*pots_values[pot_index] -= 0.001;
		if (*pots_values[pot_index] < 0)
			* pots_values[pot_index] = 0;
		break;
	}
	pot_update = true;
}


void TestAdcScreen::draw_tairg_content(Graphic<uint16_t>& display)
{
	display.SetTextPos(10, 20);
	display.DrawText("Диагностика:", RED_MASK, &big_font);

	ToString value(5);
	int y = 40;
	constexpr auto step = 20;
	constexpr auto horizontal_offset = 150;

	display.SetTextPos(10, y);
	value.Convert(adc_t, 2, false);
	display.DrawText("АЦП T:", 0, &big_font);
	display.SetTextPos(horizontal_offset, y);
	display.DrawText(value.LeftAllign(), 0, &big_font);
	display.DrawText(" В", 0, &big_font);
	y += step;

	display.SetTextPos(10, y);
	value.Convert(adc_q, 2, false);
	display.DrawText("АЦП Q:", 0, &big_font);
	display.SetTextPos(horizontal_offset, y);
	display.DrawText(value.LeftAllign(), 0, &big_font);
	display.DrawText(" В", 0, &big_font);
	y += step;

	display.SetTextPos(10, y);
	value.Convert(T, 1, false);
	display.DrawText("T:", 0, &big_font);
	display.SetTextPos(horizontal_offset, y);
	display.DrawText(value.LeftAllign(), 0, &big_font);
	display.DrawText(" град.", 0, &big_font);
	y += step;

	display.SetTextPos(10, y);
	value.Convert(Q, 1, false);
	display.DrawText("Q:", 0, &big_font);
	display.SetTextPos(horizontal_offset, y);
	display.DrawText(value.LeftAllign(), 0, &big_font);
	display.DrawText(" л/мин", 0, &big_font);
	y += step;

	display.SetTextPos(10, y);
	value.Convert(settings.t_scale, 3, false);
	display.DrawText("крутизна T:", pot_index == 0 ? GREEN_MASK : 0, &big_font);
	display.SetTextPos(horizontal_offset, y);
	display.DrawText(value.LeftAllign(), 0, &big_font);
	y += step;

	display.SetTextPos(10, y);
	value.Convert(settings.t_offset, 3, false);
	display.DrawText("смещение T:", pot_index == 1 ? GREEN_MASK : 0, &big_font);
	display.SetTextPos(horizontal_offset, y);
	display.DrawText(value.LeftAllign(), 0, &big_font);
	y += step;

	display.SetTextPos(10, y);
	value.Convert(settings.q_scale, 3, false);
	display.DrawText("крутизна Q:", pot_index == 2 ? GREEN_MASK : 0, &big_font);
	display.SetTextPos(horizontal_offset, y);
	display.DrawText(value.LeftAllign(), 0, &big_font);
	y += step;

	display.SetTextPos(10, y);
	value.Convert(settings.q_offset, 3, false);
	display.DrawText("смещение Q:", pot_index == 3 ? GREEN_MASK : 0, &big_font);
	display.SetTextPos(horizontal_offset, y);
	display.DrawText(value.LeftAllign(), 0, &big_font);
	y += step;
}

void TestAdcScreen::onShow()
{
	pot_index = 0;
	pots_values[0] = &settings.t_scale;
	pots_values[1] = &settings.t_offset;
	pots_values[2] = &settings.q_scale;
	pots_values[3] = &settings.q_offset;
	keyboard.SetTimeout(1000);
	rf_tx_enable = true;
}



void TestAdcScreen::onClose()
{
	rf_tx_enable = false;
}


TimeSetupScreen::TimeSetupScreen(DateTime how)
	: EnterDigitalValueScreen("Установка времени", 5, { &_year, &_month, &_day, &_hour, &_min }),
	_year(how.year(), 99, 1, 1, 0, u8"Год"),
	_month(how.month(), 12, 1, 1, 0, u8"Месяц"),
	_day(how.day(), 31, 1, 1, 0, u8"День"),
	_hour(how.hour(), 23, 0, 1, 0, u8"Час"),
	_min(how.minute(), 59, 0, 1, 0, u8"Минуты")
{
}

void TimeSetupScreen::on_edit_complete()
{
	DateTime t(0, _min.get(), _hour.get(), _day.get(), _month.get(), _year.get());
	rtc.set_time(t);
	const auto str = reinterpret_cast<string*>(_param);
	*str = "OK";
}


void PasswordEnterScreen::onKeyPress(char key)
{
	switch (key)
	{
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_ESC:
		break;
	case KEY_OK:
		break;
	}
}

void PasswordEnterScreen::draw_tairg_content(Graphic<uint16_t>& display)
{
}

void RfTestScreen::onKeyPress(char key)
{
	Close();
}

void RfTestScreen::draw_tairg_content(Graphic<uint16_t>& display)
{
	display.SetTextPos(20, 20);
	ToString str(10);
	//str.Convert(rf.RSSI, 1, false);
	display.DrawText("RSSI: ", 0);
	display.DrawText(str.LeftAllign(), 0);
	display.DrawText(" дб", 0);

	display.SetTextPos(20, 40);
	str.Convert(pps);
	display.DrawText("PPS: ", 0);
	display.DrawText(str.LeftAllign(), 0);
}

float GraphInfo::max_y()
{
	return *std::max_element(points.begin(), points.end());
}

float GraphInfo::min_y()
{
	return *std::min_element(points.begin(), points.end());
}

void GraphInfo::clear()
{
	points.clear();
}

void GraphInfo::insert_t(float t)
{
	if (t < 5) t = 5;
	if (t > 35) t = 35;
	points.push_back(t * 4);
}

void GraphInfo::insert_q(float q)
{
	if (q < 0) q = 0;
	if (q > 10) q = 10;
	points.push_back(q * 20);
}

GraphScreen::GraphScreen(GraphData&& graph_data)
	: _graph_data(std::move(graph_data))
{
}

void GraphScreen::onKeyPress(char key)
{
	Close();
}

void GraphScreen::draw_tairg_content(Graphic<uint16_t>& display)
{
	keyboard.SetTimeout(portMAX_DELAY);
	display.SetTextPos(100, 100);
	display.Fill(0, 16, 319, 239, 0);

	ToString val(3);

	int graph_count = 2;
	graph_count = static_cast<int>(std::min(graph_count, 6));
	display.SetTextPos(0, 20);

	display.Line(30, 220, 310, 220, 0xFFFF);
	display.Line(30, 220, 30, 20, 0xFFFF);
	display.Line(310, 220, 300, 217, 0xFFFF);
	display.Line(310, 220, 300, 223, 0xFFFF);
	display.Line(30, 20, 27, 30, 0xFFFF);
	display.Line(30, 20, 33, 30, 0xFFFF);

	display.SetTextPos(25, 220);
	display.DrawText("0", 0xFFFF, &small_font);

	for (size_t i = 1; i < 10; i++)
	{
		display.Line(29, 220 - i * 20, 31, 220 - i * 20, 0xFFFF);
		display.Line(30, 220 - i * 20, 310, 220 - i * 20, RGB(80, 80, 80));
		display.SetTextPos(14, 220 - i * 20 - 5);
		val.Convert(i);
		display.DrawText(val.RightAllign(), graphic_colors[0], &small_font);
		display.SetTextPos(0, 220 - i * 20 - 5);
		val.Convert(i * 5);
		display.DrawText(val.RightAllign(), graphic_colors[1], &small_font);
	}
	display.SetTextPos(20, 20); display.DrawText("Q", graphic_colors[0], &small_font);
	display.SetTextPos(6, 20); display.DrawText("T", graphic_colors[1], &small_font);




	int old_x = 30;
	auto it_q = _graph_data.q_graph.points.begin();
	int old_y = 220 - *it_q;
	int n = 0;
	int scale = _graph_data.q_graph.points.size() / 300 + 1;
	if (scale > 3) scale = 3;

	for (int i = 1; i < 6; i++)
	{
		int x = 30 + i * 50;
		display.Line(x, 220 - 2, x, 220 + 2, 0xFFFF);
		display.Line(x, 220, x, 20, RGB(80, 80, 80));
		display.SetTextPos(x - 10, 220 + 3);
		val.Convert(i * 5 * scale);
		display.DrawText(val.RightAllign(), 0xFFFF, &small_font);
	}

	while (it_q != _graph_data.q_graph.points.end())
	{
		int x = 30 + n / scale;
		int y = 220 - *it_q;
		if (x >= 320)
			break;
		display.Line(old_x, old_y, x, y, graphic_colors[0]);
		old_x = x;
		old_y = y;
		++it_q;
		++n;
	}

	old_x = 30;
	auto it_t = _graph_data.t_graph.points.begin();
	old_y = 220 - *it_t;
	n = 0;
	while (it_t != _graph_data.t_graph.points.end())
	{
		int x = 30 + n / scale;
		int y = 220 - *it_t;
		if (x >= 320)
			break;
		display.Line(old_x, old_y, x, y, graphic_colors[1]);
		old_x = x;
		old_y = y;
		++it_t;
		++n;
	}

	display.SetTextPos(200, 20); display.DrawText(_graph_data.q_graph.name.c_str(), graphic_colors[0], &small_font);
	display.SetTextPos(200, 35); display.DrawText(_graph_data.t_graph.name.c_str(), graphic_colors[1], &small_font);
}

//volatile int nnn = 0;

volatile int32_t adc_debug_buf1[20], adc_debug_buf2[20];
int debug_index1 = 0, debug_index2 = 0;
int debug_cnt = 0;

void measure_task()
{
	AD5293<PB12, PB11, PB2> dp1;
	AD5293<PB12, PB11, PD12> dp2;
	AD5293<PB12, PB11, PD13> dp3;
	AD840X<PB12, PB11, PB4> dp4;
	AD7714<PB13, PB12, PB11, PB15, PC6, PB14> adc;

	dp1.init();
	dp2.init();
	dp3.init();

	adc.ADCReset();
	adc.Init();
	adc.SelectChannel(5);
	adc.SetFilter(true, false, 350);
	adc.CalibrateAuto();


	for (;;)
	{
		if (pot_update)
		{
			dp1.set_position(1023 * settings.q_scale);
			dp2.set_position(1023 * settings.q_offset);
			dp3.set_position(1023 * settings.t_scale);
			dp4.set_position(0, 255 * settings.t_offset);
			pot_update = false;
		}
		adc.ReadResult();
		adc.SelectChannel(4);
		adc.CalibrateAuto();
		float adc_q_temp = 0;
		debug_index1 = 0;
		for (size_t j = 0; j < 10; j++)
		{
			adc.ADCWait();
			adc_debug_buf1[debug_index1] = adc.ReadResult();
			adc_q_temp += adc_debug_buf1[debug_index1];
			debug_index1++;
			
		}
		adc_q = adc_q_temp / static_cast<float>(0xFFFF) * 5.0 / 10.0;

		/*PE0::Reset();
		Delay_us(100);
		PE0::Set();
		*/
		adc.SelectChannel(5);
		adc.CalibrateAuto();
		float adc_t_temp = 0;
		debug_index2 = 0;
		for (size_t j = 0; j < 10; j++)
		{
			adc.ADCWait();
			adc_debug_buf2[debug_index2] = adc.ReadResult();
			adc_t_temp += adc_debug_buf2[debug_index2];
			debug_index2++;
			
		}
		adc_t = adc_t_temp / static_cast<float>(0xFFFF) * 5.0 / 10.0;

		const FlowCalc calc(adc_q, adc_t);
		Q = calc.q;
		T = calc.t;

		if (measure_in_progress)
		{
			measure_data.insert_point({ static_cast<float>(T), static_cast<float>(Q) });
			debug_cnt++;
		}

		if(rf_tx_enable)
		{
			GetCurrentData gcr(Q, T);
			rf_send_response(gcr.serialize());
		}
	}
}
/*
extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_1)
		rf.miso_falling_isr();
}
*/
uint32_t uid;

std::unique_ptr<PacketBase> rf_parse_shared(BinaryDeSerializer& de_serializer)
{
	std::unique_ptr<PacketBase> response;

	auto command = de_serializer.deserialise<RfCommand>();

	switch (command)
	{
	case RfCommand::GET_ID:
	{
		auto r = make_unique<RfGetId>(uid);
		if (rng.random_single() < r->_chance)
		{
			response = std::move(r);
		}
	}
	break;
	}
	return response;
}

std::unique_ptr<PacketBase> rf_parse_individual(BinaryDeSerializer& binary_de_serializer)
{
	std::unique_ptr<PacketBase> response;
	return response;
}

/**
 * \brief отправляет ответ хосту
 * \param buffer данные для передачи
 * \param tx_id идентификатор отправителя
 */
void rf_send_response(const vector<uint8_t>& buffer)
{
	if (buffer.size() > 50) return;

	BinarySerializer bs;
	bs.serialise(static_cast<uint8_t>(buffer.size()));
	const uint32_t crc = Crc32Block(buffer); //расчитаем crc пакета
	bs.serialise(crc);
	bs.append(buffer);
	rf_mutex.lock();
	//std::memcpy(rf.buffer, bs.binary_data.data(), bs.binary_data.size()); //поместим crc в буфер передатчика
	//rf.data_len = bs.binary_data.size(); //задаем длинну пакета
	//rf.tx_buf(100); //инициируем передачу
	rf_mutex.release();
}

constexpr std::array<RF::RegValue, 23> rf_init{
    std::make_pair(0x0001, 0x06), // GDO1 OUTPUT PIN CONFIGURATION
    std::make_pair(0x0002, 0x06), // GDO0 OUTPUT PIN CONFIGURATION
    std::make_pair(0x0003, 0x47), // RX FIFO AND TX FIFO THRESHOLDS
    std::make_pair(0x0007, 0x0C), // PACKET AUTOMATION CONTROL
    std::make_pair(0x000B, 0x06), // FREQUENCY SYNTHESIZER CONTROL
    std::make_pair(0x000D, 0x10), // FREQUENCY CONTROL WORD, HIGH BYTE
    std::make_pair(0x000E, 0xB0), // FREQUENCY CONTROL WORD, MIDDLE BYTE
    std::make_pair(0x000F, 0x71), // FREQUENCY CONTROL WORD, LOW BYTE
    std::make_pair(0x0010, 0xFA), // MODEM CONFIGURATION
    std::make_pair(0x0011, 0x86), // MODEM CONFIGURATION
    std::make_pair(0x0012, 0x13), // MODEM CONFIGURATION
    std::make_pair(0x0015, 0x24), // MODEM DEVIATION SETTING
    std::make_pair(0x0018, 0x18), // MAIN RADIO CONTROL STATE MACHINE CONFIGURATION
    std::make_pair(0x0019, 0x16), // FREQUENCY OFFSET COMPENSATION CONFIGURATION
    std::make_pair(0x001B, 0x43), // AGC CONTROL
    std::make_pair(0x0020, 0xFB), // WAKE ON RADIO CONTROL
    std::make_pair(0x0023, 0xE9), // FREQUENCY SYNTHESIZER CALIBRATION
    std::make_pair(0x0024, 0x2A), // FREQUENCY SYNTHESIZER CALIBRATION
    std::make_pair(0x0025, 0x00), // FREQUENCY SYNTHESIZER CALIBRATION
    std::make_pair(0x0026, 0x1F), // FREQUENCY SYNTHESIZER CALIBRATION
    std::make_pair(0x002C, 0x81), // VARIOUS TEST SETTINGS
    std::make_pair(0x002D, 0x35), // VARIOUS TEST SETTINGS
    std::make_pair(0x002E, 0x09)  // VARIOUS TEST SETTINGS
};

void rf_task()
{
	//PC3::SetMode(Mode::Output);
	int nn = 0;
	auto t1 = xTaskGetTickCount();

	//RF_type::RegList reg_list;
//	reg_list.emplace_back(make_pair(0x0001, 0x06)); //GDO1 OUTPUT PIN CONFIGURATION
//	reg_list.emplace_back(make_pair(0x0002, 0x06)); //GDO0 OUTPUT PIN CONFIGURATION
//	reg_list.emplace_back(make_pair(0x0003, 0x47)); //RX FIFO AND TX FIFO THRESHOLDS
//	reg_list.emplace_back(make_pair(0x0006, 0xFF)); //PACKET LENGTH
//	reg_list.emplace_back(make_pair(0x0007, 0x04)); //PACKET AUTOMATION CONTROL
//	reg_list.emplace_back(make_pair(0x0008, 0x45)); //PACKET AUTOMATION CONTROL
//	reg_list.emplace_back(make_pair(0x000B, 0x06)); //FREQUENCY SYNTHESIZER CONTROL
//	reg_list.emplace_back(make_pair(0x000D, 0x10)); //FREQUENCY CONTROL WORD, HIGH BYTE
//	reg_list.emplace_back(make_pair(0x000E, 0xB0)); //FREQUENCY CONTROL WORD, MIDDLE BYTE
//	reg_list.emplace_back(make_pair(0x000F, 0x3F)); //FREQUENCY CONTROL WORD, LOW BYTE
//	reg_list.emplace_back(make_pair(0x0010, 0xFA)); //MODEM CONFIGURATION
//	reg_list.emplace_back(make_pair(0x0011, 0x86)); //MODEM CONFIGURATION
//	reg_list.emplace_back(make_pair(0x0012, 0x13)); //MODEM CONFIGURATION
//	reg_list.emplace_back(make_pair(0x0013, 0x21)); //MODEM CONFIGURATION
//	reg_list.emplace_back(make_pair(0x0015, 0x24)); //MODEM DEVIATION SETTING
//	reg_list.emplace_back(make_pair(0x0018, 0x18)); //MAIN RADIO CONTROL STATE MACHINE CONFIGURATION
//	reg_list.emplace_back(make_pair(0x0019, 0x16)); //FREQUENCY OFFSET COMPENSATION CONFIGURATION
//	reg_list.emplace_back(make_pair(0x0020, 0xFB)); //WAKE ON RADIO CONTROL
//	reg_list.emplace_back(make_pair(0x0023, 0xE9)); //FREQUENCY SYNTHESIZER CALIBRATION
//	reg_list.emplace_back(make_pair(0x0024, 0x2A)); //FREQUENCY SYNTHESIZER CALIBRATION
//	reg_list.emplace_back(make_pair(0x0025, 0x00)); //FREQUENCY SYNTHESIZER CALIBRATION
//	reg_list.emplace_back(make_pair(0x0026, 0x1F)); //FREQUENCY SYNTHESIZER CALIBRATION
//	reg_list.emplace_back(make_pair(0x002C, 0x81)); //VARIOUS TEST SETTINGS
//	reg_list.emplace_back(make_pair(0x002D, 0x35)); //VARIOUS TEST SETTINGS
//	reg_list.emplace_back(make_pair(0x002E, 0x09)); //VARIOUS TEST SETTINGS
//	rf.rf_init(reg_list, RF_MISO_EXTI_IRQn);

	for (;;)
	{
		Delay_ms(1000);

		//rf.data_len = 20;
	//	rf_mutex.lock();
	//	const bool r = rf.rx_buf(1000);
	//	rf_mutex.release();
	//	if (r)
	//	{
	//		//получен пакет данных
	//		Buffer packet(rf.buffer, rf.data_len);
	//		BinaryDeSerializer ds(std::move(packet));
	//		//проверка на широковещательный пакет
	//		auto crc = ds.deserialise<uint32_t>();
	//		auto local_crc = ds.crc32_tail(CRC32_INIT_VALUE);
	//		if (crc == local_crc)
	//		{
	//			//пакет широковещательный
	//			const auto response = rf_parse_shared(ds);
	//			if (response)
	//				rf_send_response(response->serialize(), CRC32_INIT_VALUE);
	//		}
	//		local_crc = ds.crc32_tail(uid);
	//		if (crc == local_crc)
	//		{
	//			//пакет индивидуальный
	//			const auto response = rf_parse_individual(ds);
	//			if (response)
	//				rf_send_response(response->serialize(), uid);
	//		}
	//		nn++;
	//	}
	//	auto t = xTaskGetTickCount() - t1;
	//	if (t > 1000)
	//	{
	//		t1 = xTaskGetTickCount();
	//		pps = nn;
	//		nn = 0;
	//	}
	//	//rf.wait_for_idle();
	}
}

FlashMemoryScreen::FlashMemoryScreen()
	: WizardScreen{
		{
			WizardEntry{std::make_shared<TairgScreen>(), u8"Информация"},
			WizardEntry{std::make_shared<FormatDBScreen>(), u8"Форматироавние БД"},
			WizardEntry{std::make_shared<FormatSettingScreen>(), u8"Форматирование настроек"}
		} , u8"Хранилище" }
{}

void FormatDBScreen::draw_tairg_content(Graphic<uint16_t>& display)
{
	display.SetTextPos(30, 50);
	display.DrawText(u8"Форматирование Data Flash", RGB(0, 0, 0), &big_font);
	display.SetTextPos(30, 70);
	display.DrawText(u8"Будет потеряны все данные",RGB(255,0,0),&big_font);
	display.SetTextPos(30, 140);
	display.DrawText(u8"Нажмите кнопку 2 для удаления", RGB(255, 0, 0), &big_font);
	display.SetTextPos(30, 160);
	display.DrawText(u8"Нажмите кнопку 4 для отмены", RGB(0, 64, 0), &big_font);
}

void FormatDBScreen::onKeyPress(char key)
{
	switch (key)
	{
	case KEY_DOWN:
	{
		flash_drive.init_table();
		Close();
	} break;
		default:
			Close();
	}
}

void FormatSettingScreen::draw_tairg_content(Graphic<uint16_t>& display)
{
	display.SetTextPos(30, 50);
	display.DrawText(u8"Форматирование Internal Flash", RGB(0, 0, 0), &big_font);
	display.SetTextPos(30, 70);
	display.DrawText(u8"Будет потеряны все данные", RGB(255, 0, 0), &big_font);
	display.SetTextPos(30, 140);
	display.DrawText(u8"Нажмите кнопку 2 для удаления", RGB(255, 0, 0), &big_font);
	display.SetTextPos(30, 160);
	display.DrawText(u8"Нажмите кнопку 4 для отмены", RGB(0, 64, 0), &big_font);
}

void FormatSettingScreen::onKeyPress(char key)
{
	switch (key)
	{
	case KEY_DOWN:
	{
		settings.format();
		settings.Save();
		Close();
	} break;
	default:
		Close();
	}
}

void BoringPlaceShowScreen::onKeyPress(char key)
{
	
	if (key == KEY_OK)
	{
		//подготовка графика
		GraphData gd;
		for (auto sample : measure_data.samples)
		{
			gd.q_graph.insert_q(sample.q);
			gd.t_graph.insert_t(sample.t);
		}
		gd.q_graph.name = "Расход (л/м)";
		gd.t_graph.name = "Температура C";
		TairgScreen::Show(std::make_shared<GraphScreen>(std::move(gd)));
	}
	else
		Close();
}

void BoringPlaceShowScreen::draw_tairg_content(Graphic<uint16_t>& display)
{
	constexpr int width = 300;
	constexpr int height = 200;
	constexpr int top = 40;
	const int tab = (display.Width() - width) / 2;
	const int step_x = width / n_x;
	const int step_y = height / n_y;

	const char* title = u8"Место забуривания";
	display.SetTextPos((display.Width() - big_font.GetTextWidth(title)) / 2, top - big_font.Heaght() - 5);
	display.DrawText(title, RGB(0, 0, 0), &big_font);

	display.Rectangle(tab, top, tab + width, top + height, 0);

	for (int i = 1; i < n_x; i++)
	{
		const int x = tab + i * step_x;
		display.Line(x, top, x, top + height, 0);
	}

	for (int i = 1; i < n_y; i++)
	{
		const int y = top + i * step_y;
		display.Line(tab, y, tab + width, y, 0);
	}

	
	int x = _selected_pos % n_x;
	int y = _selected_pos / n_x;

	x *= step_x;
	x += tab;

	y *= step_y;
	y += top;

	//display.Circle(x, y, 10, RGB(255, 0, 0));
	display.Fill(x, y, x + step_x, y + step_y, RGB(255, 0, 0));

	for (int i = 0; i < n_x; i++)
		for (int j = 0; j < n_y; j++)
		{
			ToString val(3);
			val.Convert(i + j * n_x);
			display.SetTextPos(tab + i * step_x + (step_x - big_font.GetTextWidth(val.LeftAllign()))/2, top + j * step_y + (step_y - big_font.Heaght())/2);
			display.DrawText(val.LeftAllign(), RGB(0, 0, 0), &big_font);
		}
}

BoringPlaceShowScreen::BoringPlaceShowScreen(int position)
	:_selected_pos{ position }
{}