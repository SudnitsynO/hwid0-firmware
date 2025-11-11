#include "Buffer.h"
#include <cstring>
#include <memory>

Buffer::Buffer(const size_t buffer_size) 
: std::unique_ptr<uint8_t[]>(std::make_unique<uint8_t[]>(buffer_size)),
_size(buffer_size)
{
}

Buffer::Buffer(const uint8_t* data, const size_t buffer_size)
: std::unique_ptr<uint8_t[]>(std::make_unique<uint8_t[]>(buffer_size)),
_size(buffer_size)
{
	std::memcpy(get(), data, buffer_size);
}

Buffer::Buffer(Buffer&& other) noexcept: std::unique_ptr<uint8_t[]>(std::move(other)), _size(other._size)
{
}

Buffer::Buffer(std::vector<uint8_t>&& v) noexcept : std::unique_ptr<uint8_t[]>(std::make_unique<uint8_t[]>(v.size())), _size(v.size())
{
	std::copy(v.begin(), v.end(), get());
	v.clear();
}

Buffer& Buffer::operator=(Buffer&& other) noexcept
{
	if (this == &other)
		return *this;
	_size = other._size;
	std::unique_ptr<uint8_t[]>::operator =(std::move(other));
	return *this;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void Buffer::memset(const uint8_t value)
{
	std::memset(this->get(), value, _size);  // NOLINT(bugprone-undefined-memory-manipulation)
}

size_t Buffer::size() const
{	
	return _size;
}

Buffer::Buffer() : std::unique_ptr<uint8_t[]>()
{
	_size = 0;
}


void Buffer::resize(const size_t new_size)
{
	std::unique_ptr<uint8_t[]>::operator =(std::make_unique<uint8_t[]>(new_size));
	_size = new_size;
}

void Buffer::resize()
{
	reset(nullptr);
	_size = 0;
}