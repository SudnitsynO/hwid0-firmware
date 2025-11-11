#include "SPI_base.h"

std::vector<uint8_t> SPI_MasterBase::tx_rx_vector(const std::vector<uint8_t>& buffer)
{
	const auto size = buffer.size();
	std::vector<uint8_t> rx_buf(size);
	tx_rx_buf(buffer.data(), rx_buf.data(), size);
	return rx_buf;
}

void SPI_MasterBase::tx_vector(const std::vector<uint8_t>& buffer)
{
	tx_buf(buffer.data(), buffer.size());
}

std::vector<uint8_t> SPI_MasterBase::rx_vector(const size_t data_count)
{
	std::vector<uint8_t> rx_buffer(data_count);
	rx_buf(rx_buffer.data(),data_count);
	return rx_buffer;
}
