#pragma once
#include <vector>
#include <stddef.h>
#include <stdint.h>

class Stream
{
public:
	enum Exceptoins
	{
		OK,					//операция ввода - вывода завершена
		IO_ERROR,			//Ошибка ввода-вывода
		READ_NOT_SUPPOTRED,	//Поток не поддерживает чтение
		WRITE_NOT_SUPPOTRED,//Этот поток не поддерживает запись
		SEEK_NOT_SUPPORTED,	//Поток не поддерживает поиск
		BUFFER_ORERRUN,		//произошла потеря данных в результате переполнения буфера
		TIMEOUT				//Операция ввода - вывода завершилась по таймауту
	};
	Stream(void);
	virtual ~Stream(void);
	virtual bool CanRead(void);// возвратит истину, когда из потока возможно чтение
	virtual bool CanWrite(void);	// возвращает значение, которое показывает, поддерживает ли текущий поток возможность записи
	virtual bool CanSeek(void);	// классе возвращает значение, которое показывает, поддерживается ли в текущем потоке возможность поиска
	// читает блок данных из потока, возвращает количество прочитанных байт
	virtual uint64_t Read(uint8_t* buffer, uint64_t count);	// читает блок данных из потока с текущей позиции, возвращает количество прочитанных байт
	virtual uint64_t Write(const uint8_t* buffer, uint64_t count);// Записывает блок данных длинной count из buffer в поток, возвратит количество фактически записаных байт
	virtual uint64_t Position();//позиция указателя ввода - вывода
	virtual uint64_t Seek(uint64_t new_position);// задает позицию в текущем потоке, возвращает новую позицию
	virtual bool SetLength(uint64_t value);// задает длину текущего потока возвратит false если это не возможно
	virtual uint64_t Length();// возвращает длинну потока
	virtual bool Flush();// очищает все буферы данного потока и вызывает запись данных буферов в базовое устройство
	uint32_t	ReadTimeout; // таймаут для чтения мс, 0 - бесконечно
	uint32_t  WriteTimeout;// таймаут для записи мс, 0 - бесконечно
	Exceptoins IoResult; // Результат последней оперции ввода - вывода
	int ReadByte(void);// читает один байт из потока, возвратит -1 если ошибка
	virtual bool WriteByte(uint8_t byte);	// запишет один байт в поток, вернет false в случае неудачи
	virtual bool WriteString(const char* str);  	// Запишет в поток строку
	template <class T>
	bool ReadObject(T& object);
	template <class T>
	bool WriteObject(T& object);
	bool WriteVector(const std::vector<uint8_t> data);
};

template <class T>
bool Stream::ReadObject(T& object)
{
	uint64_t n = Read((uint8_t*)& object, sizeof(object));
	if (n == sizeof(object))
		return true;
	else
		return false;
}

template <class T>
bool Stream::WriteObject(T& object)
{
	//запишем
	uint64_t n = Write((uint8_t*)& object, sizeof(object));
	if (n == sizeof(object))
		return true;
	else
		return false;
}

/************************************************
*    ШАБЛОНЫ ФУНКЦИЙ (УДОБСТВА)                 *
************************************************/

template <class T>
bool WriteToStream(Stream &stream, T &object)
{
	//запишем
	uint64_t n = stream.Write((uint8_t*)&object, sizeof(object));
	if (n == sizeof(object))
		return true;
	else
		return false;
}

template<class T>
bool ReadFromStream(Stream &stream, T &object)
{
	uint64_t n = stream.Read((uint8_t*)&object, sizeof(object));
	if (n == sizeof(object))
		return true;
	else
		return false;
}

template<class T>
bool WriteToArray(size_t StartIndex, size_t LenArray, uint8_t* Array, T &object)
{
	//сколько копировать
	if ((LenArray - StartIndex) >= sizeof(object))
	{
		//копируем
		memcpy(Array + StartIndex, &object, sizeof(object));
		return true;
	}
	else
		return false;
}
