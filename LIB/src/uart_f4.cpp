#include "uart.h"
#include "stm32f4xx_hal.h"
#include <cmsis_os.h>
#include <cstring>


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

void UartFreertos::Init(void* huart, size_t rx_buffer_size)
{
	_huart = huart;
	//выделение памяти для буферов приема и передачи
	//_tx_buffer.resize(tx_buffer_size); //= new uint8_t[tx_buffer_size];
	_rx_buffer.resize(rx_buffer_size); //= new uint8_t[rx_buffer_size];
	//if (_tx_buffer == NULL) return;
	//if (_rx_buffer == NULL) return;
	//запуск приема в кольцевом буфере
	HAL_UART_Receive_DMA(static_cast<UART_HandleTypeDef*>(_huart), _rx_buffer.data(), rx_buffer_size);
	//_tx_buffer_size = tx_buffer_size;
	_rx_buffer_size = rx_buffer_size;
	//	_rx_timeout_timer = xTimerCreate("UART_RX_TIMEOUT",ReadTimeout+1, pdFALSE,this, ReadTimeotCallback);
	//	_tx_timeout_timer = xTimerCreate("UART_TX_TIMEOUT", WriteTimeout + 1, pdFALSE, this, WriteTimeotCallback);
	//_tx_complete_semaphore = xSemaphoreCreateBinary();
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

/*
void UartFreertos::TxCpltIsr(bool is_interrupt)
{
	if (!_init_complete) return;
	//interrupt!!
	_tx_buffer_sended_offset = _new_sended_offset;
	
	if ((_tx_buffer_sended_offset == _tx_buffer_offset))
	{
		//tx fully complete
		_tx_in_process = false;
	}
	else
	{
		//tx next block
		const size_t bytes_to_send = _tx_buffer_sended_offset < _tx_buffer_offset ? _tx_buffer_offset - _tx_buffer_sended_offset : _tx_buffer_size - _tx_buffer_sended_offset;
		HAL_UART_Transmit_DMA(_huart, _tx_buffer + _tx_buffer_sended_offset, bytes_to_send);
		_tx_in_process = true;
		_new_sended_offset = _tx_buffer_sended_offset + bytes_to_send;
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


	//read_count = 0;
	//в цикле принимаем данные
	while ((read_count < count))
	{
		size_t dma_offset = _rx_buffer_size - static_cast<UART_HandleTypeDef*>(_huart)->hdmarx->Instance->NDTR;
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
	//if (ReadTimeout != 0)
	//		xTimerStop(_rx_timeout_timer, 1000);
	//taskYIELD();
	return read_count;
}

//uint64_t UartFreertos::Write(const uint8_t* buffer, uint64_t count)
//{
//	TickType_t end_time = xTaskGetTickCount();
//	end_time += WriteTimeout;
//	size_t write_count = 0; //count of write bytes
//	//bool timeout = false;
//
//	//в цикле передаем данные
//	while ((write_count < count))
//	{
//		//		int bytes_to_copy = _tx_buffer_offset < _tx_buffer_sended_offset ? _tx_buffer_sended_offset - _tx_buffer_offset - 1 : (_tx_buffer_sended_offset == 0 ? _tx_buffer_size - _tx_buffer_offset - 1 : _tx_buffer_size - _tx_buffer_offset);					//место от указателя, вперед
//		size_t bytes_to_copy = _tx_buffer_offset >= _tx_buffer_sended_offset
//			                       ? (_tx_buffer_sended_offset == 0
//				                          ? _tx_buffer_size - _tx_buffer_offset - 1
//				                          : _tx_buffer_size - _tx_buffer_offset)
//			                       : _tx_buffer_sended_offset - _tx_buffer_offset - 1;
//		if (bytes_to_copy == 0)
//		{
//			//в буфере нет места, ждем освобождения части буфера
//			if (WriteTimeout > 0)
//			{
//				int32_t remaining_time = end_time - xTaskGetTickCount();
//				if (remaining_time > 0)
//				{
//					if (xSemaphoreTake(_tx_complete_semaphore, remaining_time) != pdTRUE)
//					{
//						//timeout = true;
//						break;
//					}
//				}
//				else
//				{
//					//timeout = true;
//					break;
//				}
//			}
//			else
//				xSemaphoreTake(_tx_complete_semaphore, portMAX_DELAY);
//		}
//		else
//		{
//			if (bytes_to_copy > (count - write_count)) //места в буфере много, передадим сколько надо
//				bytes_to_copy = (count - write_count);
//			//копируем
//			memcpy(_tx_buffer + _tx_buffer_offset, buffer + write_count, bytes_to_copy);
//			//инкремент
//			write_count += bytes_to_copy;
//			_tx_buffer_offset += bytes_to_copy;
//			if (_tx_buffer_offset >= _tx_buffer_size)
//				_tx_buffer_offset = 0;
//			if (_tx_in_process == false) //если передача данных не ведется, то запустим
//			{
//				//	_tx_in_process = true;
//				TxCpltIsr(false);
//			}
//		}
//	}
//
//	return write_count;
//}


uint64_t UartFreertos::Write(const uint8_t* buffer, uint64_t count)
{
	TickType_t end_time = xTaskGetTickCount();
	end_time += WriteTimeout;

	for (;;)
	{
		const auto r = HAL_UART_Transmit_DMA(static_cast<UART_HandleTypeDef*>(_huart), const_cast<uint8_t*>(buffer),
		                                     count);
		if (r != HAL_OK)
			osThreadYield();
		else
			return count;
		if (WriteTimeout)
		{
			const int32_t remaning_time = end_time - xTaskGetTickCount();
			if (remaning_time < 0)
				break;
		}
	}
	return 0;
}
