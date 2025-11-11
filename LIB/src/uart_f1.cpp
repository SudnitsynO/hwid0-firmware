#include "uart.h"
#include "stm32f1xx_hal.h"
#include <cmsis_os.h>
#include <cstring>
#include <FreeRTOS.h>
#include <task.h>
#include "semphr.h"


bool UartFreertos::CanRead()
{
	return true;
}

bool UartFreertos::CanWrite()
{
	return true;
}

UartFreertos::UartFreertos()
{
	_huart = nullptr;
	_rx_buffer_offset = 0;
	_tx_buffer_offset = 0;
	_tx_in_process = false;
	_new_sended_offset = 0;
}

UartFreertos::~UartFreertos()
{
}

void UartFreertos::Init(void* huart, size_t rx_buffer_size, size_t tx_buffer_size)
{
	_huart = huart;
	//выделение памяти для буферов приема и передачи
	_tx_buffer.resize(tx_buffer_size); //= new uint8_t[tx_buffer_size];
	_rx_buffer.resize(rx_buffer_size); //= new uint8_t[rx_buffer_size];
	//if (_tx_buffer == NULL) return;
	//if (_rx_buffer == NULL) return;
	//запуск приема в кольцевом буфере
	HAL_UART_Receive_DMA(static_cast<UART_HandleTypeDef*>(_huart), _rx_buffer.data(), rx_buffer_size);
	_tx_buffer_size = tx_buffer_size;
	_rx_buffer_size = rx_buffer_size;
	//	_rx_timeout_timer = xTimerCreate("UART_RX_TIMEOUT",ReadTimeout+1, pdFALSE,this, ReadTimeotCallback);
	//	_tx_timeout_timer = xTimerCreate("UART_TX_TIMEOUT", WriteTimeout + 1, pdFALSE, this, WriteTimeotCallback);
	_tx_complete_semaphore = xSemaphoreCreateBinary();
	//_rx_semaphore = xSemaphoreCreateBinary();
	//	rx_task_handle = nullptr;
	//	xTaskCreate((TaskFunction_t)RxBufferTask, "RX_BUF", 16, this, rx_task_priority, &rx_task_handle);
	_init_complete = true;
}

void UartFreertos::DmaErrorIsr()
{
	if (!_init_complete) return;
	_rx_buffer_offset = 0;
	HAL_UART_Receive_DMA(static_cast<UART_HandleTypeDef*>(_huart), _rx_buffer.data(), _rx_buffer_size);
}

void UartFreertos::TxCpltIsr(bool is_isr)
{
	if (!_init_complete) return;

	//передача блока завершена
	//_tx_buffer_lock_begin = _tx_buffer_lock_end;
	if(_tx_buffer_offset > _tx_buffer_lock_end)	//значит добавили новые данные справа
	{
		_tx_buffer_lock_begin = _tx_buffer_lock_end;	//начало блока на конце предидущего
		_tx_buffer_lock_end = _tx_buffer_offset;		//конец на конце пользовательских данных
	}
	else if (_tx_buffer_offset == _tx_buffer_lock_end)	//значит новых данных нет
	{
		_tx_buffer_lock_begin = _tx_buffer_lock_end;	//сжимаем блок до нуля
	}
	else if(_tx_buffer_lock_end < _tx_buffer_size)//добавили данные справа + слева
	{
		_tx_buffer_lock_begin = _tx_buffer_lock_end;	//начало блока на конце предидущего
		_tx_buffer_lock_end = _tx_buffer_size;			//конец блока на конце буфера
	}
	else	//данные только слева
	{
		_tx_buffer_lock_begin = 0;		//начало слева
		_tx_buffer_lock_end = _tx_buffer_offset;	//конец на конце пользовательских данных
	}	
	auto data_count_to_tx = _tx_buffer_lock_end - _tx_buffer_lock_begin;
	if (data_count_to_tx)
	{
		//если есть что передать
		//передадим
		_tx_in_process = true;
		//_tx_buffer_lock_end += data_count_to_tx;
		HAL_UART_Transmit_DMA((UART_HandleTypeDef*)_huart, _tx_buffer.data() + _tx_buffer_lock_begin, data_count_to_tx);
		
	}
	else
		_tx_in_process = false;
	if (is_isr)
	{
		BaseType_t  p = pdFALSE;
		xSemaphoreGiveFromISR(_tx_complete_semaphore, &p);
		portYIELD_FROM_ISR(p);
	}
}

bool UartFreertos::Flush()
{
	if (!_init_complete) return true;
	if (!_tx_in_process) return true;
	TickType_t end_time = xTaskGetTickCount();
	end_time += WriteTimeout;
	while(_tx_in_process)
	{
		auto local_timeout = portMAX_DELAY;
		if (WriteTimeout)
		{
			local_timeout = xTaskGetTickCount();
			if (local_timeout >= end_time)
			{
				return false;//закончить
			}
			else
				local_timeout = end_time - local_timeout;
			
		}
		auto re = xQueueSemaphoreTake(_tx_complete_semaphore, local_timeout);

		if (re == pdFALSE)
		{
			return false;//закончить
		}
	}
	return true;
}

/*
void UartFreertos::TxCpltIsr(bool is_interrupt)
{
	if (!_init_complete) return;
	//interrupt!!
	_tx_buffer_lock_end = _new_sended_offset;
	
	if ((_tx_buffer_lock_end == _tx_buffer_offset))
	{
		//tx fully complete
		_tx_in_process = false;
	}
	else
	{
		//tx next block
		const size_t bytes_to_send = _tx_buffer_lock_end < _tx_buffer_offset ? _tx_buffer_offset - _tx_buffer_lock_end : _tx_buffer_size - _tx_buffer_lock_end;
		HAL_UART_Transmit_DMA(_huart, _tx_buffer + _tx_buffer_lock_end, bytes_to_send);
		_tx_in_process = true;
		_new_sended_offset = _tx_buffer_lock_end + bytes_to_send;
		if (_new_sended_offset >= _tx_buffer_size)
			_new_sended_offset = 0;
	}
	BaseType_t  p = pdFALSE;;
	if (is_interrupt)
		xSemaphoreGiveFromISR(_tx_complete_semaphore,&p);
}
*/
uint64_t UartFreertos::Read(uint8_t* buffer, uint64_t count)
{
	TickType_t end_time = xTaskGetTickCount();
	end_time += ReadTimeout;
	size_t read_count = 0; //count of read bytes


	//в цикле принимаем данные
	while ((read_count < count))
	{
		size_t dma_offset = _rx_buffer_size - static_cast<UART_HandleTypeDef*>(_huart)->hdmarx->Instance->CNDTR;
		//индекс принятого байта в кольцевом буфере ПДП
		int bytes_to_copy = dma_offset - _rx_buffer_offset; //число принятых байт
		if (bytes_to_copy < 0) //обнаружено заворачивание 
			bytes_to_copy = _rx_buffer_size - _rx_buffer_offset; //тогда копируем до конца буфера
		if (bytes_to_copy > (count - read_count))
			bytes_to_copy = count - read_count; //байт в буфере больше чем нужно
		if (bytes_to_copy == 0)
		{
			//данных нету, ждем
			osThreadYield();
			if (ReadTimeout)
			{
				const int32_t remaning_time = end_time - xTaskGetTickCount();
				if (remaning_time < 0)
					break;
			}
		}
		else
		{
			//данные пришли, копируем
			std::memcpy(buffer + read_count, _rx_buffer.data() + _rx_buffer_offset, bytes_to_copy);
			read_count += bytes_to_copy;
			_rx_buffer_offset += bytes_to_copy;
			if (_rx_buffer_offset >= _rx_buffer_size)
				_rx_buffer_offset = 0;
		}
	}
	return read_count;
}

uint64_t UartFreertos::Write(const uint8_t* buffer, uint64_t count)
{
	auto end_time = xTaskGetTickCount();
	end_time += WriteTimeout;
	size_t write_count = 0; //столько байт обработано
	//в цикле передаем данные
	while ((write_count < count))
	{
		auto local_timeout = portMAX_DELAY; 
		if (WriteTimeout)
		{
			local_timeout = xTaskGetTickCount();
			if (local_timeout >= end_time)
			{
				break;//закончить
			}
			else
				local_timeout = end_time - local_timeout;
		}

		auto bytes_to_copy = count - write_count;	//еще нужно обработать в буфере пользователя
		auto buffer_free_bytes = _tx_buffer_size - _tx_buffer_offset;	//свободно места до конца буфера
		if(_tx_buffer_offset < _tx_buffer_lock_end)	//значит мы пишем в начало
		{
			buffer_free_bytes = _tx_buffer_lock_begin - _tx_buffer_offset ;	//свободно в буфере до безопасной границы
		}
		if (bytes_to_copy > buffer_free_bytes)  
			bytes_to_copy = buffer_free_bytes;
		if (buffer_free_bytes > 0)
		{
			//копируем из пользовательского буфера в системный
			std::memcpy(_tx_buffer.data() + _tx_buffer_offset, buffer + write_count, bytes_to_copy);
			_tx_buffer_offset += bytes_to_copy;
			write_count += bytes_to_copy;
		}
		else
		{
			//место в системном буфере закончилось
			if(_tx_buffer_offset >= _tx_buffer_lock_end)
			{
				// есть место в начале буфера, можно писать туда
				_tx_buffer_offset = 0;
			}
			else if(_tx_in_process)
			{
				//ждем завершения передач
				
				auto re = xQueueSemaphoreTake(_tx_complete_semaphore, local_timeout);
				
				if(re == pdFALSE)
				{
					break;//закончить
				}
			}
			else
			{
				//буфер полностью передан
				_tx_buffer_offset = 0;
				_tx_buffer_lock_end = 0;
			}
		}
		if(/*(_tx_buffer_offset != _tx_buffer_lock_end)&&*/( _tx_in_process == false))
		{
			//данные в буфере есть, но ничего не отправляется
			TxCpltIsr(false);	//инициируем передачу
		}
	}
	return write_count;
}