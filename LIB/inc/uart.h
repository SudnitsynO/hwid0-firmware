#pragma once
//		Библиотека - реализация интерфеса Stream
//		для UART с применением HAL и FREERTOS
//
//		1) Настроить UART в STM32CubeMX
//		2) Включить DMA на прием и на передачу (передача нормальный режим, прием - кольцевой режим)
//		3) создать обработчик прерывания HAL_UART_ErrorCallback и вызвать из него DmaErrorIsr
//		4) добавить в обработчик прерывания передатчика DMA функцию HAL_UART_IRQHandler после HAL_DMA_IRQHandler
//		5) включить прерывания UART
//		6) создать обработчик HAL_UART_TxCpltCallback и вызвать из него TxCpltIsr()
//
//
#include <vector>
#include "stream.h"
#include "FreeRTOS.h"
#include "queue.h"

class UartFreertos : public Stream
{
public:
	UartFreertos();
	virtual ~UartFreertos();
	void Init(void * huart, size_t rx_buffer_size, size_t tx_buffer_size);
	virtual uint64_t Read(uint8_t* buffer, uint64_t count);
	virtual uint64_t Write(const uint8_t* buffer, uint64_t count);
	virtual bool Flush();// очищает все буферы данного потока и вызывает запись данных буферов в базовое устройство
	//interrupts
	void DmaErrorIsr();
	void TxCpltIsr(bool is_isr = true);
	
private:
//	static void ReadTimeotCallback(TimerHandle_t xTimer);
//	static void WriteTimeotCallback(TimerHandle_t xTimer);

public:
	virtual bool CanRead();
	virtual bool CanWrite();
	//volatile bool _debug1 = false;
private:
	void * _huart;
	std::vector<uint8_t> _rx_buffer;
	std::vector<uint8_t> _tx_buffer;
	volatile size_t _rx_buffer_offset;
	volatile size_t _tx_buffer_offset;			// сколько байт положено в буфер
	volatile size_t _tx_buffer_lock_end{};	// сколько байт уже отправлено
	volatile size_t _tx_buffer_lock_begin{};	// сколько байт свободно в начале буфера
	size_t _rx_buffer_size{};
	size_t _tx_buffer_size{};
//	TimerHandle_t _rx_timeout_timer;
	volatile bool rx_timeout{};
	volatile bool _tx_in_process;
	volatile bool tx_timeout{};
//	TimerHandle_t _tx_timeout_timer;
	QueueHandle_t _tx_complete_semaphore{};
//	QueueHandle_t _rx_semaphore{};
	volatile size_t _new_sended_offset;
	volatile bool _init_complete = false;
	
};
