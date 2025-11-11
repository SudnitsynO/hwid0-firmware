#pragma once
#include <vector>
#include "BinaryConverter.h"
#include <Buffer.h>
#include "crc.h"
#include <cstring>

class BinarySerializer
{
public:

	std::vector<uint8_t> binary_data;

	template <class T>
	void serialise(T object)
	{
		static_assert(std::is_pod<T>::value,"NO POD TYPE");
		auto v = BinaryConverter::to_vector(object);
		binary_data.insert(binary_data.end(),v.begin(),v.end());
	}

	void append(const std::vector<uint8_t> &raw_data)
	{
		binary_data.insert(binary_data.end(), raw_data.begin(), raw_data.end());
	}

	uint32_t crc32()
	{
		return Crc32Block(CRC32_INIT_VALUE, binary_data.data(), binary_data.size());
	}
	
private:
};

class BinaryDeSerializer  // NOLINT
{
public:
	// ReSharper disable CppNonExplicitConvertingConstructor
	BinaryDeSerializer(const Buffer &buf)
	// ReSharper restore CppNonExplicitConvertingConstructor
		:_local_buffer(buf.size()), _offset(0)
	{
		std::memcpy(_local_buffer.get(), buf.get(), buf.size());
	}

	BinaryDeSerializer(Buffer&& buf)
		// ReSharper restore CppNonExplicitConvertingConstructor
		:_local_buffer(std::move(buf)), _offset(0)
	{
		//std::memcpy(_local_buffer.get(), buf.get(), buf.size());
	}

	// ReSharper disable CppNonExplicitConvertingConstructor
	BinaryDeSerializer(const std::vector<uint8_t>& buf)
	// ReSharper restore CppNonExplicitConvertingConstructor
		:_local_buffer(buf.size()), _offset(0)
	{
		std::memcpy(_local_buffer.get(), buf.data(), buf.size());
	}

	BinaryDeSerializer(const BinaryDeSerializer& other) = delete;  // NOLINT

	BinaryDeSerializer(BinaryDeSerializer&& other) noexcept
		: _local_buffer(std::move(other._local_buffer)),_offset(0)
	{
	}

	void operator=(const BinaryDeSerializer& other) = delete;  // NOLINT

	void operator=(BinaryDeSerializer&& other) = delete;  // NOLINT

	template<class T>
	T deserialise()
	{
		static_assert(std::is_pod<T>::value, "NO POD TYPE");
		const auto size = sizeof(T);
		const auto ptr = reinterpret_cast<T*>(_local_buffer.get() + _offset);
		_offset += size;
		if (_offset > _local_buffer.size())
			return T();
		else
			return *ptr;
	}

	void reset(size_t offset = 0)
	{
		if(offset < _local_buffer.size())
		_offset = offset;
	}

	//считает CRC32 от текущей позиции до конца буфера
	uint32_t crc32_tail(const uint32_t init_value = CRC32_INIT_VALUE)
	{
		return Crc32Block(init_value, _local_buffer.get() + _offset, _local_buffer.size() - _offset);
	}

	//считает CRC32 от текущей позиции указаное число байтов
	uint32_t crc32_len(const size_t len_crc_block, const uint32_t init_value = CRC32_INIT_VALUE)
	{
		if (len_crc_block > (_local_buffer.size() - _offset)) return 0;

		return Crc32Block(init_value, _local_buffer.get() + _offset, len_crc_block);
	}

private:
	Buffer _local_buffer;
	size_t _offset;
};
