#include "usart_hal_freertos.h"
#include <task.h>
#include <timers.h>
#include <string.h>


void UsartDMA::ReadTimeotCallback(TimerHandle_t xTimer)
{
	UsartDMA * usart = reinterpret_cast<UsartDMA*>(pvTimerGetTimerID(xTimer));
	usart->rx_timeout = true;
}

void UsartDMA::WriteTimeotCallback(TimerHandle_t xTimer)
{
	UsartDMA * usart = reinterpret_cast<UsartDMA*>(pvTimerGetTimerID(xTimer));
	usart->tx_timeout = true;
}

bool UsartDMA::CanRead()
{
	return true;
}

bool UsartDMA::CanWrite()
{
	return true;
}

UsartDMA::UsartDMA()
{
	_huart = NULL;
	_rx_buffer = NULL;
	_tx_buffer = NULL;
	_rx_buffer_offset = 0;
	_tx_buffer_offset = 0;
	_tx_in_process = false;
	_new_sended_offset = 0;
}

UsartDMA::~UsartDMA()
{
	delete[] _rx_buffer;
	delete[] _tx_buffer;
}

void UsartDMA::Init(UART_HandleTypeDef* huart, size_t tx_buffer_size, size_t rx_buffer_size)
{
	_huart = huart;
	//выделение памяти для буферов приема и передачи
	_tx_buffer = new uint8_t[tx_buffer_size];
	_rx_buffer = new uint8_t[rx_buffer_size];
	if (_tx_buffer == NULL) return;
	if (_rx_buffer == NULL) return;
	//запуск приема в кольцевом буфере
	HAL_UART_Receive_DMA(_huart,_rx_buffer,rx_buffer_size);
	_tx_buffer_size = tx_buffer_size;
	_rx_buffer_size = rx_buffer_size;
	_rx_timeout_timer = xTimerCreate("UART_RX_TIMEOUT",ReadTimeout+1, pdFALSE,this, ReadTimeotCallback);
	_tx_timeout_timer = xTimerCreate("UART_TX_TIMEOUT", WriteTimeout + 1, pdFALSE, this, WriteTimeotCallback);
	vSemaphoreCreateBinary(_tx_complete_semaphore);
}

void UsartDMA::TxCpltIsr()
{
	//прерывание!!
	_tx_buffer_sended_offset = _new_sended_offset;
	
	if ((_tx_buffer_sended_offset == _tx_buffer_offset))
	{
		//передача завершена полностью
		//_new_sended_offset = 0;
		_tx_in_process = false;
	}
	else
	{
		//передаем
		size_t bytes_to_send = _tx_buffer_sended_offset < _tx_buffer_offset ? _tx_buffer_offset - _tx_buffer_sended_offset : _tx_buffer_size - _tx_buffer_sended_offset;
		HAL_UART_Transmit_DMA(_huart, _tx_buffer + _tx_buffer_sended_offset, bytes_to_send);
		_tx_in_process = true;
		_new_sended_offset = _tx_buffer_sended_offset + bytes_to_send;
		if (_new_sended_offset >= _tx_buffer_size)
			_new_sended_offset = 0;
	}
	BaseType_t  p = pdFALSE;;
	xSemaphoreGiveFromISR(_tx_complete_semaphore,&p);	
}

uint64_t UsartDMA::Read(uint8_t* buffer, uint64_t count)
{
	rx_timeout = false;
	if (ReadTimeout != 0)
	{
		xTimerChangePeriod(_rx_timeout_timer, ReadTimeout, 1000);
		xTimerStart(_rx_timeout_timer, 1000);
	}
	size_t read_count = 0;		//сколько байт прочитано

	//в цикле принимаем данные
	while ((read_count < count) && (rx_timeout == false))
	{
		size_t dma_offset = _rx_buffer_size - _huart->hdmarx->Instance->CNDTR;	//индекс принятого байта в кольцевом буфере ПДП
		int bytes_to_copy = dma_offset - _rx_buffer_offset;							//число принятых байт
		if (bytes_to_copy < 0)														//обнаружено заворачивание 
			bytes_to_copy = _rx_buffer_size - _rx_buffer_offset;						//тогда копируем до конца буфера
		if (bytes_to_copy > (count - read_count))
			bytes_to_copy = count - read_count;										//байт в буфере больше чем нужно
		if (bytes_to_copy == 0)
		{
			//данных нету, ждем
			taskYIELD();
		}
		else
		{
			//данные пришли, копируем
			memcpy(buffer + read_count, _rx_buffer + _rx_buffer_offset, bytes_to_copy);
			read_count += bytes_to_copy;
			_rx_buffer_offset += bytes_to_copy;
			if (_rx_buffer_offset >= _rx_buffer_size)
				_rx_buffer_offset = 0;
		}
	}
	if (ReadTimeout != 0) 
		xTimerStop(_rx_timeout_timer, 1000);
	//taskYIELD();
	return  read_count;
}

uint64_t UsartDMA::Write(uint8_t* buffer, uint64_t count)
{
	size_t write_count = 0;
	tx_timeout = false;
	if (WriteTimeout != 0)
	{
		xTimerChangePeriod(_tx_timeout_timer, WriteTimeout, 1000);
		xTimerStart(_tx_timeout_timer, 1000);
	}
	//в цикле передаем данные
	while ((write_count < count) && (tx_timeout == false))
	{
		int bytes_to_copy = _tx_buffer_offset < _tx_buffer_sended_offset ? _tx_buffer_sended_offset - _tx_buffer_offset - 1 : (_tx_buffer_sended_offset == 0 ? _tx_buffer_size - _tx_buffer_offset - 1 : _tx_buffer_size - _tx_buffer_offset);					//место от указателя, вперед
		if (bytes_to_copy == 0)
		{
			//в буфере нет места, ждем освобождения части буфера
			if (WriteTimeout != 0)
				xSemaphoreTake(_tx_complete_semaphore, xTimerGetExpiryTime(_tx_timeout_timer));
			else
				xSemaphoreTake(_tx_complete_semaphore, portMAX_DELAY);
		}
		else
		{
			if (bytes_to_copy > (count - write_count))			//места в буфере много, передадим сколько надо
				bytes_to_copy = (count - write_count);
			//копируем
			memcpy(_tx_buffer + _tx_buffer_offset, buffer + write_count, bytes_to_copy);
			//инкремент
			write_count += bytes_to_copy;
			_tx_buffer_offset += bytes_to_copy;
			if (_tx_buffer_offset >= _tx_buffer_size)
				_tx_buffer_offset = 0;
			if (_tx_in_process == false)	//если передача данных не ведется, то запустим
			{
				//	_tx_in_process = true;
				TxCpltIsr();
			}

		}
	}

	if (WriteTimeout != 0)
		xTimerStop(_tx_timeout_timer, 1000);
	return write_count;
}

void UsartDMA::TxDmaErrorIsr()
{
	if(_rx_buffer != NULL)
	{
		_rx_buffer_offset = 0;
		HAL_UART_Receive_DMA(_huart, _rx_buffer, _rx_buffer_size);
	}
}


