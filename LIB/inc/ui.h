#pragma once
#include "TextDisplay.h"
#include "graphic.h"
#include <memory>
#include <list>

#define KEY_UP 1
#define KEY_DOWN 2
#define KEY_OK 0x0D
#define KEY_ESC 0x1B

class TextScreen
{
public:
	struct ScreenListIthem
	{
		TextScreen * screen;
		ScreenListIthem * prev;
	};
	void Show(void * param = nullptr);	//���������� �������� ����� � ���������� ��������� �� �������
	static TextScreen * Close();	//��������� ������� ����� � ���������� ��������� �� ����
	static void Redraw(TextDisplay & display);			//�������� ����������� �������� ������ �� �������
	static void KeyProc(char key);						//������� ������������ ���������� �������� ������
	void * GetParam();									//���������� ���������, ���������� � Show(void*)
//***************************************************
// ���� ��������������
//***************************************************
	virtual void onRedraw(TextDisplay &display);	//���������� ���������� ������
	virtual void onKeyPress(char key);						//���������� ����������
	virtual void onShow();							//���������� ����� ������������ ������
	virtual void onClose();							//���������� ����� ��������� ������
	void* _param;
private:
	
	static ScreenListIthem * _current_screen;
};

class TextMenu : public TextScreen
{
	struct TextMenuIthem
	{
		char * text;
		TextMenuIthem * next;
		TextMenuIthem * prev;
		TextScreen * screen;
		void * param;
	};
public:
	TextMenu();
	void Init(char * menu_name);
	~TextMenu();
	void Add(char * text,TextScreen * screen = nullptr, void * param = nullptr);
	virtual void onRedraw(TextDisplay &display);	//���������� ���������� ������
	virtual void onKeyPress(char key);						//���������� ����������
private:
	char * _menu_name;
	TextMenuIthem * _current_menu_ithem;
	TextMenuIthem * _first_menu_ithem;
	TextMenuIthem * _top_menu_ithem;
};

class EditFormatedScreen : public TextScreen
{
public:
	EditFormatedScreen();
	~EditFormatedScreen();
	void Init(char * name, char * format, char * init_text = nullptr);	//������ ������ ������������� ������ (\x1 - �����, \x2 �����)
	virtual void onRedraw(TextDisplay &display);	//���������� ���������� ������
	virtual void onKeyPress(char key);				//���������� ����������
	virtual void onOk();	//������������ ����� OK
	char * text;
	//Event complete;
private:
	char * _name;
	char * _format;
	int len;
	int _cursor_pos;
	
};

class ExitMenuScreen : public TextScreen
{
public:
	virtual void onRedraw(TextDisplay &display);	//���������� ���������� ������
};

template<class ColorType>
class GraphicScreen
{
public:
	static void Show(std::shared_ptr<GraphicScreen>&& new_screen, void* param = nullptr);	//���������� �������� ����� � ��������� ��������� �� �������
	static void CloseCurrent();
	static void CloseAll();
	void Close();
	static void Redraw(Graphic<ColorType>& display);			//�������� ����������� �������� ������ �� �������
	static void KeyProc(char key);						//������� ������������ ���������� �������� ������
	void* GetParam();									//���������� ���������, ���������� � Show(void*)
//***************************************************
// ���� ��������������
//***************************************************
	virtual void onRedraw(Graphic<ColorType>& display);	//���������� ���������� ������
	virtual void onKeyPress(char key);						//���������� ����������
	virtual void onShow();							//���������� ����� ������������ ������
	virtual void onClose();							//���������� ����� ��������� ������
//	virtual ~GraphicScreen();
	void* _param = nullptr;
private:
	static std::list<std::shared_ptr<GraphicScreen<ColorType>>> _screen_list;
	bool _closeRequest = false;
};

template<class ColorType>
void GraphicScreen<ColorType>::Show(std::shared_ptr<GraphicScreen>&& new_screen, void * param)
{
	new_screen->_param = param;
	new_screen->_closeRequest = false;
	new_screen->onShow();
	if(!new_screen->_closeRequest)
	{
		_screen_list.emplace_front(new_screen);
	}
}

template<class ColorType>
void GraphicScreen<ColorType>::CloseCurrent()
{
	const auto current = _screen_list.begin();
	if (current != _screen_list.end())
	{
		_screen_list.erase(current);
	}
}

template<class ColorType>
void GraphicScreen<ColorType>::CloseAll()
{
	for (auto& screen : _screen_list)
	{
		screen->onClose();
	}
	_screen_list.clear();
}

template<class ColorType>
void GraphicScreen<ColorType>::Close()
{
	onClose();
	_closeRequest = true;
}

template<class ColorType>
void GraphicScreen<ColorType>::Redraw(Graphic<ColorType>& display)
{
	const auto current = _screen_list.begin();
	if (current != _screen_list.end())
	{
		(**current).onRedraw(display);
		if ((**current)._closeRequest)
			_screen_list.erase(current);
	}
	else
	{
		display.Clear();
		display.DrawText("NULL",0xFFFF);
	}
}

template<class ColorType>
void GraphicScreen<ColorType>::KeyProc(char key)
{
	const auto current = _screen_list.begin();
	if (current != _screen_list.end())
	{
		(**current).onKeyPress(key);
		if ((**current)._closeRequest)
			_screen_list.erase(current);
	}
}

template<class ColorType>
void * GraphicScreen<ColorType>::GetParam()
{
	return _param;
}

template<class ColorType>
void GraphicScreen<ColorType>::onRedraw(Graphic<ColorType>& display)
{
	display.Clear();
	display.SetTextPos(0,0);
	display.DrawText("NULL", 0xFFFF);
}

template<class ColorType>
void GraphicScreen<ColorType>::onKeyPress(char key)
{
}

template<class ColorType>
void GraphicScreen<ColorType>::onShow()
{
}

template<class ColorType>
void GraphicScreen<ColorType>::onClose()
{
}

template <class ColorType>
std::list<std::shared_ptr<GraphicScreen<ColorType>>> GraphicScreen<ColorType>::_screen_list;
