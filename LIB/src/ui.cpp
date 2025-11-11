#include "ui.h"
#include <string.h>


void TextScreen::Redraw(TextDisplay & display)
{
	if(_current_screen != NULL)
	{
		if (_current_screen->screen != NULL)
			_current_screen->screen->onRedraw(display);
	}
	else
	{
		display.Clear();
		display.WriteString("NULL");
	}
}

void TextScreen::KeyProc(char key)
{
	if(_current_screen != NULL)
	{
		if (_current_screen->screen != NULL)
		_current_screen->screen->onKeyPress(key);
	}
}

void* TextScreen::GetParam()
{
	return _param;
}

void TextScreen::onKeyPress(char key){}

void TextScreen::onRedraw(TextDisplay &display)
{
	display.Clear();
	display.putchar('E');
	display.putchar('M');
	display.putchar('P');
	display.putchar('T');
	display.putchar('Y');
	display.putchar(' ');
	display.putchar('S');
	display.putchar('C');
	display.putchar('R');
	display.putchar('E');
	display.putchar('E');
	display.putchar('N');
}

void TextScreen::Show(void * param)
{
	_param = param;
//	__enter_critical();
	ScreenListIthem * new_ithem = new ScreenListIthem;
	new_ithem->prev = _current_screen;
	new_ithem->screen = this;
//	__levay_critical();
	_current_screen = new_ithem;
	onShow();
}

void TextScreen::onShow()
{
}

void TextScreen::onClose()
{
}

TextScreen * TextScreen::Close()
{
	if (_current_screen != NULL)
	{
		_current_screen->screen->onClose();
		ScreenListIthem * p = _current_screen;
		TextScreen * r = p->screen;
		_current_screen = p->prev;
		delete p;
		return r;
	}
	else
		return NULL;
}

TextMenu::TextMenu()
{
	_current_menu_ithem = NULL;
	_first_menu_ithem = NULL;
	_top_menu_ithem = NULL;
	_menu_name = NULL;
}

void TextMenu::Init(char * menu_name)
{
	_menu_name = menu_name;
}

void TextMenu::onRedraw(TextDisplay &display)
{

	display.Clear();
	display.SetCursorType(display.CURSOR_OFF);
	if ((display.RowCount() > 1) && (_menu_name != NULL))
	{
		display.WriteString(_menu_name);
		display.SetCursorPos(0, 1);
	}
	if (_current_menu_ithem != NULL)
	{
		TextMenuIthem* p = _first_menu_ithem;
		int n = 0;
		while (p)
		{
			if (p == _current_menu_ithem)
			{
				if(n == 0)
					_top_menu_ithem = _current_menu_ithem;
				break;
			}
			if ((n == 0) && (p == _top_menu_ithem))
				n = 1;

			p = p->next;

			if (n)
			{
				if (n >= (display.RowCount() - 1))
					_top_menu_ithem = _top_menu_ithem->next;
				else
					n++;
			}
		}

		//прорисовка
		p = _top_menu_ithem;
		for (int i = 0; i < (display.RowCount() - 1); i++)
		{
			if (p)
			{
				display.SetCursorPos(0, i + 1);
				if (p == _current_menu_ithem)
					display.putchar('>');
				else
					display.putchar(' ');
				display.WriteString(p->text);
			}
			else
				break;
			p = p->next;
		}
	}
	else
	{
		display.WriteString("NULL");
	}
}

void TextMenu::onKeyPress(char key)
{
	switch (key)
	{
	case KEY_UP:
		if (_current_menu_ithem != NULL)
		if (_current_menu_ithem->next != NULL)
			_current_menu_ithem = _current_menu_ithem->next;
		else
			_current_menu_ithem = _first_menu_ithem;
		break;
	case KEY_DOWN:
		if (_current_menu_ithem != NULL)
		if (_current_menu_ithem->prev != NULL)
			_current_menu_ithem = _current_menu_ithem->prev;
		break;
	case KEY_OK:
		if (_current_menu_ithem != NULL)
		if (_current_menu_ithem->screen != NULL)
			_current_menu_ithem->screen->Show(_current_menu_ithem->param);
		break;
	case KEY_ESC:
		Close();
		break;
	}
}

TextMenu::~TextMenu()
{
	TextMenuIthem * p = _first_menu_ithem;
	while (p != NULL)
	{
		TextMenuIthem * d = p;
		p = p->next;
		delete d;
	}
}

void TextMenu::Add(char * text, TextScreen * screen, void * param)
{
	if (_first_menu_ithem != NULL)
	{
		TextMenuIthem * last_ithem = _first_menu_ithem;
		while (last_ithem->next != NULL)
			last_ithem = last_ithem->next;
		TextMenuIthem * new_ithem = new TextMenuIthem;
		new_ithem->next = NULL;
		new_ithem->prev = last_ithem;
		new_ithem->screen = screen;
		new_ithem->text = text;
		new_ithem->param = param;
		last_ithem->next = new_ithem;
	}
	else
	{
		_first_menu_ithem = new TextMenuIthem;
		_first_menu_ithem->next = NULL;
		_first_menu_ithem->prev = NULL;
		_first_menu_ithem->screen = screen;
		_first_menu_ithem->text = text;
		_first_menu_ithem->param = param;
		_current_menu_ithem = _first_menu_ithem;
	}
}

EditFormatedScreen::EditFormatedScreen()
{
	_format = NULL;
	text = NULL;
	len = 0;
	_cursor_pos = 0;
}

void EditFormatedScreen::Init(char * name, char * format, char * init_text)
{
	_name = name;
	_format = format;
	//EditFormatedScreen::text = text;
	len = strlen(format);
	delete[] EditFormatedScreen::text;
	EditFormatedScreen::text = NULL;
	EditFormatedScreen::text = new char[len + 1];
	for (int i = 0; i < len; i++)
		EditFormatedScreen::text[i] = ' ';
	EditFormatedScreen::text[len] = 0;
	if (init_text != NULL)
	{
		int n = strlen(init_text);
		if (n > len)
			n = len;
		for (int i = 0; i < n; i++)
			EditFormatedScreen::text[i] = init_text[i];
	}

	for (int i = 0; i < len; i++)
	{
		if ((_format[i] != '\x1') && (_format[i] != '\x2'))
		{
			EditFormatedScreen::text[i] = _format[i];
		}
	}
	_cursor_pos = 0;
	while (_cursor_pos < len)
	{	
		if ((_format[_cursor_pos] == '\x1') || (_format[_cursor_pos] == '\x2'))
			break;
		_cursor_pos++;
	}
//	complete.Reset();
}

void EditFormatedScreen::onRedraw(TextDisplay &display)
{
	display.Clear();
	display.WriteString(_name);
	display.SetCursorPos(0, 1);
	display.WriteString(text);
	display.SetCursorType(display.CURSOR_BLINKING);
	display.SetCursorPos(_cursor_pos,1);
}

void EditFormatedScreen::onKeyPress(char key)
{
	if (key == KEY_OK)
	{
		//select
		while (_cursor_pos < len)
		{
			_cursor_pos++;
			if ((_format[_cursor_pos] == '\x1') || (_format[_cursor_pos] == '\x2'))
				break;
		}
		if (_cursor_pos == len)
		{
			Close();
			onOk();
		}
	}
	if (key == KEY_UP)
	{
		if (_format[_cursor_pos] == '\x1')	//цифра
		{
			if (++text[_cursor_pos] > '9')
				text[_cursor_pos] = '0';
			if (text[_cursor_pos] < '0')
				text[_cursor_pos] = '9';
		}
		if (_format[_cursor_pos] == '\x2')	//Буква
		{
			if (++text[_cursor_pos] > 0xA0)
				text[_cursor_pos] = 0x21;
		}
	}
	if (key == KEY_DOWN)
	{
		if (_format[_cursor_pos] == '\x1')	//цифра
		{
			if (--text[_cursor_pos] > '9')
				text[_cursor_pos] = '0';
			if (text[_cursor_pos] < '0')
				text[_cursor_pos] = '9';
		}
		if (_format[_cursor_pos] == '\x2')	//Буква
		{
			if (--text[_cursor_pos] < 0x21)
				text[_cursor_pos] = 0xA0;
		}
	}
	if (key == KEY_ESC)
	{
		if (_cursor_pos == 0)
		{
			Close();
		}
		else while (_cursor_pos > 0)
		{
			_cursor_pos--;
			if ((_format[_cursor_pos] == '\x1') || (_format[_cursor_pos] == '\x2'))
				break;
		}

	}
}

EditFormatedScreen::~EditFormatedScreen()
{
	delete[] text;
}

void EditFormatedScreen::onOk()
{
//	complete.Set();
}


void ExitMenuScreen::onRedraw(TextDisplay& display)
{
	Close();
	Close();
}
