#pragma once
#include "ui.h"
#include <vector>
#include "EditableValue.h"
#include <DateTime.h>
#include "MeasureDB.h"

using namespace std;

class TairgScreen : public GraphicScreen<uint16_t>
{
public:
	virtual void onRedraw(Graphic<uint16_t>& display) final;
	virtual void draw_tairg_content(Graphic<uint16_t>& display);
	//inline ~TairgScreen() override;
};

class ShowResultDigitalData : public TairgScreen
{
public:
	inline void onKeyPress(char key) override;
	void draw_tairg_content(Graphic<uint16_t>& display) override;
	void onShow() override;
};

class ShowResultStat : public TairgScreen	//отображает статистику и информацию по измерению
{
public:
	ShowResultStat(MeasureHeader measure_header);
	inline void onKeyPress(char key) override;
	void draw_tairg_content(Graphic<uint16_t>& display) override;
private:
	MeasureHeader _measure_header{};
};

class ResultList : public TairgScreen
{
public:
	inline void onKeyPress(char key) override;
	void draw_tairg_content(Graphic<uint16_t>& display) override;
	void onShow() override;
private:
	static constexpr int count_in_page = 7;
	std::vector<MeasureHeader> _entries {};
	vector<MeasureHeader>::iterator _offset;
	vector<MeasureHeader>::iterator _pos;
};

class PositionSelectScreen : public TairgScreen
{
	static constexpr int n_x = 6;
	static constexpr int n_y = 5;
public:
	inline void onKeyPress(char key) override;
	void draw_tairg_content(Graphic<uint16_t>& display) override;
private:
	int selected_pos;
};

class WizardEntry
{
public:

	WizardEntry(shared_ptr<TairgScreen>&& tairg_screen, const char* const title);
	WizardEntry(const WizardEntry& other);
	WizardEntry(WizardEntry&& other) noexcept;
	void operator=(const WizardEntry& other) = delete;
	WizardEntry& operator=(WizardEntry&& other) noexcept;
	const char* title() const;
	void Show();
	const char* result_string() const;

private:
	shared_ptr<TairgScreen> _screen;
	const char* _title;
	string response;
};

class WizardScreen : public TairgScreen
{
	static constexpr int count_in_page = 8;
public:
	WizardScreen(initializer_list<WizardEntry> entries, const char* title);
	inline void onKeyPress(char key) override;

	//WizardScreen(initializer_list<TairgScreen>);

	inline void onShow() override;
	inline void onClose() override;
	void draw_tairg_content(Graphic<uint16_t>& display) override;
private:
	vector<WizardEntry> _entries;
	const char* _title;
	vector<WizardEntry>::iterator pos;
	vector<WizardEntry>::iterator offset;
};

class StartScreen : public TairgScreen
{
	virtual void draw_tairg_content(Graphic<uint16_t> &display);	//���������� ���������� ������
	virtual void onKeyPress(char key);						//���������� ����������
	//virtual void onShow();							//���������� ����� ������������ ������
	//virtual void onClose();							//���������� ����� ��������� ������
};

class MeasureScreen : public TairgScreen
{
public:
	virtual void draw_tairg_content(Graphic<uint16_t> &display);	
	virtual void onKeyPress(char key);					
private:
	void exit();

public:
	void onClose() override;
};

class ZaboySelectScreen : public TairgScreen
{
public:
	ZaboySelectScreen()
		: list_offset(0), selected(0)
	{
	}

private:
	virtual void draw_tairg_content(Graphic<uint16_t> &display);	//���������� ���������� ������
	virtual void onKeyPress(char key);					//���������� ����������
	//virtual void onShow();							//���������� ����� ������������ ������
	//virtual void onClose(); 
	int list_offset;
	int selected;
};

class EnterDigitalValueScreen : public TairgScreen
{
public:
	EnterDigitalValueScreen(const char * title,int rows_on_screen, initializer_list<EditableValue*> values) :
		_title(title), _value_list(values), need_to_clear(true), _current_index(0), _rows_on_screen(rows_on_screen)
	{
	}
	virtual void draw_tairg_content(Graphic<uint16_t> &display);
	virtual void on_edit_complete();
	virtual void onKeyPress(char key);					
	virtual void onShow();
private:
	//virtual void onClose();							
	const char * const _title;
	const vector<EditableValue*> _value_list;
	bool need_to_clear;
	int  _current_index;
	const int _rows_on_screen;
};

class ShtybScreen : public EnterDigitalValueScreen
{
public:
	ShtybScreen();
	void onKeyPress(char key) override;
private:
	EditableValue shpur_deep;
	EditableValue shtyb_value;
};

class TestAdcScreen : public TairgScreen
{
public:
	inline void onKeyPress(char key) override;
	void draw_tairg_content(Graphic<uint16_t>& display) override;
	inline void onShow() override;
	inline void onClose() override;
//	virtual ~TestAdcScreen() override;
private:
	float* pots_values[4];
	int pot_index; //0 - 3	
};

class TimeSetupScreen : public EnterDigitalValueScreen
{
public:
	TimeSetupScreen(DateTime how);
	void on_edit_complete() override;
private:
	EditableValue _year;
	EditableValue _month;
	EditableValue _day;
	EditableValue _hour;
	EditableValue _min;
};

class PasswordEnterScreen : public TairgScreen
{
public:
	inline void onKeyPress(char key) override;
	void draw_tairg_content(Graphic<uint16_t>& display) override;
};


class RfTestScreen : public TairgScreen
{
public:
	inline void onKeyPress(char key) override;
	void draw_tairg_content(Graphic<uint16_t>& display) override;
};

class BoringPlaceShowScreen : public TairgScreen
{
	static constexpr int n_x = 6;
	static constexpr int n_y = 5;
public:
	BoringPlaceShowScreen(int position);
	void onKeyPress(char key) override;
	void draw_tairg_content(Graphic<uint16_t>& display) override;
private:
	int _selected_pos{ 0 };
};

struct GraphInfo
{
	vector<uint8_t> points;
	string name;
	float max_y();
	float min_y();
	void clear();
	void insert_t(float t);
	void insert_q(float q);
};

struct GraphData
{
	GraphInfo t_graph;
	GraphInfo q_graph;
};

class GraphScreen : public TairgScreen
{public:
	GraphScreen(GraphData&& graph_data);
	inline void onKeyPress(char key) override;
	void draw_tairg_content(Graphic<uint16_t>& display) override;
private:
	GraphData _graph_data;
};

class FlashMemoryScreen : public WizardScreen
{
public:
	FlashMemoryScreen();
};


class FormatDBScreen : public TairgScreen
{
public:
	void draw_tairg_content(Graphic<uint16_t>& display) override;
	void onKeyPress(char key) override;
};

class FormatSettingScreen : public TairgScreen
{
public:
	void draw_tairg_content(Graphic<uint16_t>& display) override;
	void onKeyPress(char key) override;
};