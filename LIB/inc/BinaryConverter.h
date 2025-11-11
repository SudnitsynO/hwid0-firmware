#pragma once
#include <cstdint>
#include <vector>

namespace BinaryConverter
{
	template<class T>
	std::vector<uint8_t> to_vector(T object)
	{
		const auto ptr = reinterpret_cast<uint8_t*>(&object);
		const auto size = sizeof(object);
		return std::vector<uint8_t>(ptr, ptr + size);
	}

	template<class T>
	T to_object(const std::vector<uint8_t> & vect,size_t offset = 0)
	{
		const auto ptr = reinterpret_cast<const T*>(vect.data()+offset);
		const auto size = sizeof(T);
		if ((offset + size) > vect.size())
			return T();
		else
			return *ptr;
	}

	template<class T>
	T to_object(uint8_t * ptr)
	{
		return *reinterpret_cast<T*>(ptr);
	}
}
