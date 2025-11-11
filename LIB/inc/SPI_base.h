#pragma once
#include <stdint.h>
#include <vector>

class SPI_MasterBase
{
public:
	virtual ~SPI_MasterBase() = default;
	/**
	 * \brief передает байт и возвратит полученый байт
	 * \param byte байт для передачи
	 * \return полученый от ведомого ответ 
	 */
	virtual uint8_t tx_byte(const uint8_t byte) = 0;
	/**
	 * \brief Передает и принимает блок данных 
	 * \param buffer буфер для передачи данных
	 * \return буфер с принятыми данными
	 */
	virtual std::vector<uint8_t> tx_rx_vector(const std::vector<uint8_t>& buffer);
	/**
	 * \brief Передает и принимает блок данных 
	 * \param tx_buffer буфер для передачи данных
	 * \param rx_buffer буфер для приёма данных
	 * \param data_count количество байт для обмена
	 */
	virtual void tx_rx_buf(const uint8_t * const tx_buffer, uint8_t * const rx_buffer, const size_t data_count) = 0;
	/**
	 * \brief Передаст буфер данных 
	 * \param buffer буфер данных 
	 */
	virtual void tx_vector(const std::vector<uint8_t>& buffer);
	/**
	 * \brief Передаст буфер данных
	 * \param tx_buffer буфер данных 
	 * \param data_count количество байт для передачи
	 */
	virtual void tx_buf(const uint8_t* const tx_buffer, const size_t data_count) = 0;
	/**
	 * \brief Примет данные 
	 * \param data_count количество данных
	 * \return Буфер с принятыми данными
	 */
	virtual std::vector<uint8_t> rx_vector(const size_t data_count);
	/**
	 * \brief Примет данные
	 * \param rx_buffer Буфер для приема данных
	 * \param data_count количество байт для приёма
	 */
	virtual void rx_buf(uint8_t* rx_buffer, const size_t data_count) = 0;
	/**
	 * \brief ожидает, пока завершатся все фоновые передачи
	 */
	virtual void flush() = 0;
};
