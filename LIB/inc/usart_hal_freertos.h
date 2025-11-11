#pragma once
//#include "yanisutil.h"
#include "stream.h"
#include "FreeRTOS.h"
#include <stm32f0xx_hal.h>
#include <timers.h>
#include <semphr.h>
//#include "../../../Tools/CPP/mutex_freertos.h"

//данный клас предостовляет потоковый ввод-вывод для USART STM32
//для пакета CUBE + HAL + FREERTOS
//для настройки USART необходимо задействовать режим DMA (обычный для передачи, и кольцевой на прием)
//создать экземпляр класса, задать таймауты чтения и записи потока
//затем вызвать функцию Init передав необходимые параметры
//класс использует функционал таймеров freertos для контроля таймаутов
//необходимо включить поддержку таймеров в CUBE, задав наивысший приоритет (6) задаче таймеров
//функции чтения и записи блокируют выполнение низкоприоритетных задач, в т.ч. Idle
//т.к. прием данных осуществляется поолингом DMA буфера
//в основной программе необходимо создать CallBack функцию окончания передачи DMA для данного USART
//в которой вызывать обработчик TxCpltIsr данного экземпляра класса
//разрешить прерывания от USART в CUBE

class UsartDMA : public Stream
{
	
public:
	UsartDMA();
	virtual ~UsartDMA();
	void Init(UART_HandleTypeDef * huart,size_t tx_buffer_size, size_t rx_buffer_size);
	void TxCpltIsr();
	virtual uint64_t Read(uint8_t* buffer, uint64_t count);
	virtual uint64_t Write(uint8_t* buffer, uint64_t count);
	void TxDmaErrorIsr();
private:
	static void ReadTimeotCallback(TimerHandle_t xTimer);
	static void WriteTimeotCallback(TimerHandle_t xTimer);

public:
	virtual bool CanRead();
	virtual bool CanWrite();
private:
	UART_HandleTypeDef * _huart;
	uint8_t * _rx_buffer;
	uint8_t * _tx_buffer;
	size_t _rx_buffer_offset;
	size_t _tx_buffer_offset;
	size_t _tx_buffer_sended_offset;
	size_t _rx_buffer_size;
	size_t _tx_buffer_size;
	TimerHandle_t _rx_timeout_timer;
	volatile bool rx_timeout;
	volatile bool _tx_in_process;
	volatile bool tx_timeout;
	TimerHandle_t _tx_timeout_timer;
	QueueHandle_t _tx_complete_semaphore;
	volatile size_t _new_sended_offset;
};