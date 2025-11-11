#pragma once
#include <stdint.h>
#include <memory>
#include <vector>

class Buffer : public std::unique_ptr<uint8_t[]>
{
public:
    Buffer();

	Buffer(const size_t buffer_size);

	Buffer(const uint8_t * data, const size_t buffer_size);

	Buffer(const Buffer& other) = delete;

	Buffer(Buffer&& other) noexcept;

	Buffer(std::vector<uint8_t>&& v) noexcept;

	Buffer& operator=(const Buffer& other) = delete;

	Buffer& operator=(Buffer&& other) noexcept;

	void memset(const uint8_t value);

	void resize(const size_t new_size);	//all data lost

	void resize();	//all data lost

	size_t size() const;
private:
	size_t _size;
};

