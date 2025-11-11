#pragma once
#include <utility>
#include <stdexcept>

template <class KeyType, class DataType, size_t cache_size>
class Cache
{
public:
	void add(const KeyType key, const DataType data) noexcept
	{
		auto it = find(key);
		if (it != end())
		{
			_data[it].second = data;
			return;
		}
		if (_start_pos)
			_start_pos--;
		else
		{
			_start_pos = cache_size - 1;
			_is_full = true;
		}
		_data[_start_pos].first = key;
		_data[_start_pos].second = data;
	}

	size_t end() const noexcept { return cache_size; } ;
	size_t begin() const noexcept { return _is_full ? 0 : _start_pos; }

	size_t find(const KeyType key) noexcept //поиск записи в кеш возвратит индекс записи
	{
		//от начала кеша до конца массива
		for (size_t i = _start_pos; i < cache_size; i++)
			if (_data[i].first == key)
			{
				std::swap(_data[i], _data[_start_pos]);
				return _start_pos;
			}
		if (_is_full)
			//от начала массива до конца кеша
			for (size_t i = 0; i < _start_pos; i++)
				if (_data[i].first == key)
				{
					std::swap(_data[i], _data[_start_pos]);
					return _start_pos;
				}
		return end();
	}

	DataType& operator[](size_t index)
	{
		if (index >= cache_size)
		{
			throw std::out_of_range("cache index out of range");
		}
		else if (_is_full)
		{
			return _data[index].second;
		}
		else if (index >= _start_pos)
		{
			return _data[index].second;
		}
		throw std::out_of_range("cache index out of range");
	}

	void reset() noexcept
	{
		_is_full = false;
		_start_pos = cache_size;
	}

private:
	std::pair<KeyType, DataType> _data[cache_size];
	size_t _start_pos = cache_size;
	bool _is_full = false;
};
