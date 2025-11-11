#pragma once
#include <cstddef>
#include <string>

bool convert_utf8_to_windows1251(const char* utf8, char* windows1251, size_t max_size);
std::string convert_utf8_to_windows1251(const std::string utf8);
