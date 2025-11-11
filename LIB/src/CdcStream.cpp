//#include "usart.h"
//#include <task.h>
#include "CdcStream.h"
#include <timers.h>
#include <string.h>

//#include "gpio_stm32f1.h"

//size_t kkk;
CdcStream * CdcStream::_instance = nullptr;

//void CdcStream::ReadTimeotCallback(TimerHandle_t xTimer)
//{
//	CdcStream * usart = reinterpret_cast<UsartDMA*>(pvTimerGetTimerID(xTimer));
//	usart->rx_timeout = true;
//}

//void CdcStream::WriteTimeotCallback(TimerHandle_t xTimer)
//{
//	CdcStream * usart = reinterpret_cast<UsartDMA*>(pvTimerGetTimerID(xTimer));
//	usart->tx_timeout = true;
//}

bool CdcStream::CanRead()
{
	return true;
}

bool CdcStream::CanWrite()
{
	return true;
}

CdcStream::CdcStream(): _rx_buffer(nullptr), _rx_buffer_offset(0), _rx_buffer_size(0), _rx_semaphore(nullptr),
                        _rx_data_count(0), _cdcRxBufferLenght(0), _cdcTxBufferLenght(0), _usb_hold(false)
{
}

CdcStream::~CdcStream()
{
	delete[] _rx_buffer;
	//delete[] _tx_buffer;
}

//void RxBufferTask(CdcStream * usart)	//task for parse input rx dma buffer
//{
//	size_t read_count = 0;		//сколько байт прочитано
//	for(;;)
//	{ 
//		xSemaphoreTake(_rx_semaphore, portMAX_DELAY);	//get semaphore to start rx_parsing
//
//		read_count = 0;
//		//в цикле принимаем данные
//		while ((read_count < count_bytes_to_rx) && (rx_timeout == false))
//		{
//			size_t dma_offset = _rx_buffer_size - _huart->hdmarx->Instance->CNDTR;	//индекс принятого байта в кольцевом буфере ПДП
//			int bytes_to_copy = dma_offset - _rx_buffer_offset;							//число принятых байт
//			if (bytes_to_copy < 0)														//обнаружено заворачивание 
//				bytes_to_copy = _rx_buffer_size - _rx_buffer_offset;						//тогда копируем до конца буфера
//			if (bytes_to_copy > (count_bytes_to_rx - read_count))
//				bytes_to_copy = count_bytes_to_rx - read_count;										//байт в буфере больше чем нужно
//			if (bytes_to_copy == 0)
//			{
//				//данных нету, ждем
//				taskYIELD();
//			}
//			else
//			{
//				//данные пришли, копируем
//				memcpy(buffer + read_count, _rx_buffer + _rx_buffer_offset, bytes_to_copy);
//				read_count += bytes_to_copy;
//				_rx_buffer_offset += bytes_to_copy;
//				if (_rx_buffer_offset >= _rx_buffer_size)
//					_rx_buffer_offset = 0;
//			}
//		}
//		if (ReadTimeout != 0)
//			//		xTimerStop(_rx_timeout_timer, 1000);
//				//taskYIELD();
//			return  read_count;
//	}
//}

void CdcStream::Init(int cdcTxBufferLenght, int cdcRxBufferLenght)
{
	//_huart = huart;
	//выделение памяти для буферов приема и передачи
	//_tx_buffer = new uint8_t[tx_buffer_size];
	_rx_buffer = new uint8_t[cdcRxBufferLenght];
	//if (_tx_buffer == NULL) return;
	if (_rx_buffer == NULL) return;
	_rx_data_count = 0;
	//запуск приема в кольцевом буфере
	//HAL_UART_Receive_DMA(_huart,_rx_buffer,rx_buffer_size);
	//_tx_buffer_size = tx_buffer_size;
	//_rx_buffer_size = rx_buffer_size;
	//_rx_timeout_timer = xTimerCreate("UART_RX_TIMEOUT",ReadTimeout+1, pdFALSE,(void*)&rx_timeout, TimeotCallback);
	//_tx_timeout_timer = xTimerCreate("UART_TX_TIMEOUT", WriteTimeout + 1, pdFALSE, (void*)&tx_timeout, TimeotCallback);
	//_tx_complete_semaphore = xSemaphoreCreateBinary();
	_rx_semaphore = xSemaphoreCreateBinary();
	//rx_task_handle = nullptr;
	//xTaskCreate((TaskFunction_t)RxBufferTask, "RX_BUF", 16, this, rx_task_priority, &rx_task_handle);
	
	_cdcRxBufferLenght = cdcRxBufferLenght;
	_cdcTxBufferLenght = cdcTxBufferLenght;
	_instance = this;
}

//void CdcStream::TxCpltIsr()
//{
	//прерывание!!
	//_tx_buffer_sended_offset = _new_sended_offset;
	//
	//if ((_tx_buffer_sended_offset == _tx_buffer_offset))
	//{
	//	//передача завершена полностью
	//	//_new_sended_offset = 0;
	//	_tx_in_process = false;
	//}
	//else
	//{
	//	//передаем
	//	size_t bytes_to_send = _tx_buffer_sended_offset < _tx_buffer_offset ? _tx_buffer_offset - _tx_buffer_sended_offset : _tx_buffer_size - _tx_buffer_sended_offset;
	//	HAL_UART_Transmit_DMA(_huart, _tx_buffer + _tx_buffer_sended_offset, bytes_to_send);
	//	_tx_in_process = true;
	//	_new_sended_offset = _tx_buffer_sended_offset + bytes_to_send;
	//	if (_new_sended_offset >= _tx_buffer_size)
	//		_new_sended_offset = 0;
	//}
//	//BaseType_t  p = pdFALSE;;
//	//xSemaphoreGiveFromISR(_tx_complete_semaphore,&p);	
//}

void CdcStream::ReceiveCallback(uint8_t* Buf, uint32_t len)
{
//	PC13::Reset();
	if (_instance)
	{
		while (_instance->_usb_hold)
		{
			return;
		}
		memcpy(_instance->_rx_buffer, Buf, len);
		_instance->_rx_data_count = len;
		_instance->_rx_buffer_offset = 0;
		_instance->_usb_hold = true;
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(_instance->_rx_semaphore, &xHigherPriorityTaskWoken);
		//portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
//	PC13::Set();
}
//
//void CdcStream::TimeotCallback(TimerHandle_t xTimer)
//{
//	*static_cast<bool*>(pvTimerGetTimerID(xTimer)) = true;
//}
volatile static int debug_v1 = 0;
uint64_t CdcStream::Read(uint8_t* buffer, uint64_t count)
{
	TickType_t start = xTaskGetTickCount();
	/*if (ReadTimeout != 0)
	{
		xTimerChangePeriod(_rx_timeout_timer, ReadTimeout, 1000);
		xTimerStart(_rx_timeout_timer, 1000);
	}
	rx_timeout = false;*/
	size_t buffer_index = 0;
	while (buffer_index < count)
	{
		debug_v1 = 1;
		TickType_t xRemainingTime = ReadTimeout ? start + ReadTimeout - xTaskGetTickCount() : portMAX_DELAY;
		if (ReadTimeout)
			if (xRemainingTime & 0x80000000)
			{
				/*if (_usb_hold)
				{
					Enable_USB_data_rx();
					_usb_hold = false;
				}*/
				debug_v1 = 2;
				break;
			}
		auto count_in_rx_buf = _rx_data_count - _rx_buffer_offset;
		if(count_in_rx_buf == 0)
		{
			debug_v1 = 3;
			if (_usb_hold)
			{
				debug_v1 = 4;
				Enable_USB_data_rx();
				debug_v1 = 5;
				_usb_hold = false;
			}

			if(xSemaphoreTake(_rx_semaphore, xRemainingTime) == pdTRUE)
			{
				debug_v1 = 6;
				count_in_rx_buf = _rx_data_count - _rx_buffer_offset;
			}
			else
			{
				debug_v1 = 7;
				break;
			}
		}

		auto bytes_to_copy = count - buffer_index;
		
		if(bytes_to_copy > count_in_rx_buf)
		{
			bytes_to_copy = count_in_rx_buf;
		}
		memcpy(buffer + buffer_index, _rx_buffer + _rx_buffer_offset, bytes_to_copy);
		buffer_index += bytes_to_copy;
		_rx_buffer_offset += bytes_to_copy;
	}
	return buffer_index;
}

uint64_t CdcStream::Write(const uint8_t* buffer, uint64_t count)
{
	TickType_t start = xTaskGetTickCount();
	size_t offset = 0;
	
	/*if (WriteTimeout != 0)
	{
		xTimerChangePeriod(_tx_timeout_timer, WriteTimeout, 1000);
		xTimerStart(_tx_timeout_timer, 1000);
	}*/
	//tx_timeout = false;
//	//в цикле передаем данные
	while (offset < count)
	{
		TickType_t xRemainingTime = ReadTimeout ? start + ReadTimeout - xTaskGetTickCount() : portMAX_DELAY;
		int bytes_to_copy = count - offset;
		if (bytes_to_copy > _cdcTxBufferLenght)
			bytes_to_copy = _cdcTxBufferLenght;
		while (CDC_Transmit_FS((uint8_t*)(buffer + offset), bytes_to_copy) != USBD_OK)
		{
			xRemainingTime = ReadTimeout ? start + ReadTimeout - xTaskGetTickCount() : portMAX_DELAY;
			if(WriteTimeout)
			if (xRemainingTime & 0x80000000)
				break;
			portYIELD();
		}
		if (WriteTimeout)
			if (xRemainingTime & 0x80000000)
				break;
		offset += bytes_to_copy;

	}
	//место от указателя, вперед
//		if (bytes_to_copy == 0)
//		{
//			//в буфере нет места, ждем освобождения части буфера
//			if (WriteTimeout != 0)
////				xSemaphoreTake(_tx_complete_semaphore, xTimerGetExpiryTime(_tx_timeout_timer));
//			else
//				xSemaphoreTake(_tx_complete_semaphore, portMAX_DELAY);
//		}
//		else
//		{
//			if (bytes_to_copy > (count - write_count))			//места в буфере много, передадим сколько надо
//				bytes_to_copy = (count - write_count);
//			//копируем
//			memcpy(_tx_buffer + _tx_buffer_offset, buffer + write_count, bytes_to_copy);
//			//инкремент
//			write_count += bytes_to_copy;
//			_tx_buffer_offset += bytes_to_copy;
//			if (_tx_buffer_offset >= _tx_buffer_size)
//				_tx_buffer_offset = 0;
//			if (_tx_in_process == false)	//если передача данных не ведется, то запустим
//			{
//				//	_tx_in_process = true;
//				TxCpltIsr();
//			}
//
//		}
//	}
//
	//if (WriteTimeout != 0)
		//xTimerStop(_tx_timeout_timer, 1000);
	return offset;
}

//void CdcStream::TxDmaErrorIsr()
//{
//	if(_rx_buffer != NULL)
//	{
//		_rx_buffer_offset = 0;
//		HAL_UART_Receive_DMA(_huart, _rx_buffer, _rx_buffer_size);
//	}
//}


void CDC_ReceiveCallback(uint8_t* Buf, uint32_t* Len)
{
	CdcStream::ReceiveCallback(Buf, *Len);
}
