/*************************************************************************
 *
 * STM32 HAL FREERTOS USB-CDC to Stream class interface
 *
 * Using:
 * 1) Include CdcStream.h into usbd_cdc_if.c
 * 2) add void Enable_USB_data_rx(); definition in usbd_cdc_if.h (USER CODE BEGIN EXPORTED_FUNCTIONS section)
 * 3) move USBD_CDC_ReceivePacket(&hUsbDeviceFS); from CDC_Receive_FS to Enable_USB_data_rx (usbd_cdc_if.c)
 * 4) call CDC_ReceiveCallback from CDC_Receive_FS (usbd_cdc_if.c)
 * 5) include "CdcStream.h" into program.h
 * 6) create instance of CdcStream class
 * 7) call Init(x,y) (x,y as configured in CubeMX)
 * 
 * Also need to define LINE_CODING management
 * 
 *   case CDC_SET_LINE_CODING:
 *		tempbuf[0] = pbuf[0];
 *		tempbuf[1] = pbuf[1];
 *		tempbuf[2] = pbuf[2];
 *		tempbuf[3] = pbuf[3];
 *		tempbuf[4] = pbuf[4];
 *		tempbuf[5] = pbuf[5];
 *		tempbuf[6] = pbuf[6];
 *
 *		break;
 *
 *	case CDC_GET_LINE_CODING:
 *		pbuf[0] = tempbuf[0];
 *		pbuf[1] = tempbuf[1];
 *		pbuf[2] = tempbuf[2];
 *		pbuf[3] = tempbuf[3];
 *		pbuf[4] = tempbuf[4];
 *		pbuf[5] = tempbuf[5];
 *		pbuf[6] = tempbuf[6];
 *
 *		break;
 **
 *************************************************************************/
#ifndef CDC_STREAM_H
#define CDC_STREAM_H
#ifdef __cplusplus

#include "FreeRTOS.h"
#include "usbd_cdc_if.h"
#include "stream.h"

#include "semphr.h"
#include "timers.h"

class CdcStream : public Stream
{
public:
	CdcStream();
	virtual ~CdcStream();
	void Init(int cdcTxBufferLenght, int cdcRxBufferLenght);
	virtual uint64_t Read(uint8_t* buffer, uint64_t count);
	virtual uint64_t Write(const uint8_t* buffer, uint64_t count);
	//interrupts
//	void TxDmaErrorIsr();
//	void TxCpltIsr();
	static void ReceiveCallback(uint8_t* Buf, uint32_t len);
private:
	//static void TimeotCallback(TimerHandle_t xTimer);

public:
	virtual bool CanRead();
	virtual bool CanWrite();
private:
	//UART_HandleTypeDef * _huart;
	uint8_t * _rx_buffer;
	//uint8_t * _tx_buffer;
	size_t _rx_buffer_offset;
	//size_t _tx_buffer_offset;
	//size_t _tx_buffer_sended_offset;
	size_t _rx_buffer_size;
	//size_t _tx_buffer_size;
	//TimerHandle_t _rx_timeout_timer;
    //volatile bool rx_timeout;
	//volatile bool _tx_in_process;
	//volatile bool tx_timeout;
	//TimerHandle_t _tx_timeout_timer;
	//QueueHandle_t _tx_complete_semaphore;
	QueueHandle_t _rx_semaphore;
	//volatile size_t _new_sended_offset;
	//TaskHandle_t rx_task_handle;
	size_t _rx_data_count;	//shutdown /s /t 0
	int _cdcRxBufferLenght;
	int _cdcTxBufferLenght;
	volatile bool _usb_hold;
	static CdcStream * _instance;
};
#endif

#ifdef __cplusplus
extern "C" {
#endif
	void CDC_ReceiveCallback(uint8_t* Buf, uint32_t *Len);
#ifdef __cplusplus
}
#endif
#endif
