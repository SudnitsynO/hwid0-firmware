#pragma once
#include <algorithm>
#include <charconv>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace Serialization
{
	class Value;

	enum class ValueType
	{
		Null,
		Double,
		Integer,
		String,
		Object,
		Array,
		Boolean
	};

	class NullValue {};

	typedef std::map<std::string, Value> ObjectStore;

	typedef std::vector<Value> ArrayStore;

	class Value
	{
	public:
		Value() = default;
		Value(int int_value) :_value(static_cast<int64_t>(int_value)) {}
		Value(int64_t int_value) :_value(int_value) {}
		Value(double double_value) :_value(double_value) {}
		Value(float float_value) :_value(static_cast<double>(float_value)) {}
		Value(const Value& other) noexcept :_value(other._value) {}
		Value(Value&& other) noexcept :_value(std::move(other._value)) {}
		Value(ObjectStore& object) :_value(object) {}
		Value(ObjectStore&& object) :_value(move(object)) {}
		Value(ArrayStore&& object) :_value(move(object)) {}
		Value(ArrayStore& object) :_value(object) {}
		Value(std::string& str) :_value(str) {}
		Value(std::string&& str) :_value(move(str)) {}
		Value(bool bool_value) :_value(bool_value) {}
		Value(const char* str) :_value(std::string{ str }) {}


		Value& operator=(const bool right)
		{
			_value = right;
			return *this;
		}

		Value& operator=(const double right)
		{
			_value = right;
			return *this;
		}

		Value& operator=(const float right)
		{
			_value = static_cast<double>(right);
			return *this;
		}

		Value& operator=(const int64_t right)
		{
			_value = right;
			return *this;
		}

		Value& operator=(const int right)
		{
			_value = static_cast<int64_t>(right);
			return *this;
		}

		Value& operator=(const char* text)
		{
			_value.emplace<std::string>(text);
			return *this;
		}

		Value& operator=(const std::string& str)
		{
			_value.emplace<std::string>(str);
			return *this;
		}

		Value& operator=(std::string&& str)
		{
			_value = move(str);
			return *this;
		}

		Value& operator=(const ArrayStore& array)
		{
			_value = array;
			return *this;
		}

		Value& operator=(ArrayStore&& array)
		{
			_value = move(array);
			return *this;
		}

		Value& operator=(const ObjectStore& obj)
		{
			_value = obj;
			return *this;
		}

		Value& operator=(ObjectStore&& obj)
		{
			_value = move(obj);
			return *this;
		}

		Value& operator=(Value& other)
		{
			_value = other._value;
			return *this;
		}

		Value& operator=(Value&& other)
		{
			_value = std::move(other._value);
			return *this;
		}

		bool is_double()
		{
			return std::get_if<double>(&_value) != nullptr;
		}

		double get_double() const
		{
			auto* const intptr = std::get_if<int64_t>(&_value);
			if (intptr)
			{
				return static_cast<double>(*intptr);
			}

			return std::get<double>(_value);
		}

		float get_float() const
		{
			return static_cast<float>(get_double());
		}

		bool is_integer()
		{
			return std::get_if<int64_t>(&_value) != nullptr;
		}

		int64_t get_int() const
		{
			return  std::get<int64_t>(_value);
		}

		bool is_string()
		{
			return std::get_if<std::string>(&_value) != nullptr;
		}

		std::string& get_string()
		{
			return std::get<std::string>(_value);
		}

		bool is_boolean()
		{
			return std::get_if<bool>(&_value) != nullptr;
		}

		bool get_boolean()
		{
			return std::get<bool>(_value);
		}

		ValueType value_type() const
		{
			return static_cast<ValueType>(_value.index());
		}

		bool is_object()
		{
			return std::get_if<ObjectStore>(&_value) != nullptr;
		}

		ObjectStore& get_object()
		{
			return std::get<ObjectStore>(_value);
		}

		bool is_array()
		{
			return std::get_if<ArrayStore>(&_value) != nullptr;
		}

		ArrayStore& get_array()
		{
			return std::get<ArrayStore>(_value);
		}

		Value& operator[](const std::string & key_sublevel)
		{
			auto& subobject = get_object();
			return subobject[key_sublevel];
		}

		Value& operator[](const size_t index)
		{
			auto& subobject = get_array();
			return subobject[index];
		}

		void push_back(Value& value)
		{
			get_array().emplace_back(value);
		}

		void push_back(Value&& value)
		{
			get_array().emplace_back(std::move(value));
		}



	private:
		std::variant<NullValue, double, int64_t, std::string, ObjectStore, ArrayStore, bool> _value;

	};

	static const std::string_view json_value_charset = "nN0123456789.+-\"tTfF";

	class JSON
	{
	public:
		Value value;
	public:
		JSON() = default;
		JSON(const std::string& str) :_str{ str } {}
		JSON(std::string&& str) :_str{std::move(str) } {}

		bool parse()
		{
			TextZone zone;
			zone.begin_zone = _str.begin();
			zone.end_zone = _str.end();
			while (zone.begin_zone != zone.end_zone)	//ищем начало объекта или массива
			{
				if (*zone.begin_zone == '{')
				{
					ObjectStore obj;
					if (!parse_object(zone, obj))
						return false;
					value = move(obj);
					break;
				}
				if (*zone.begin_zone == '[')
				{
					ArrayStore arr;
					if (!parse_array(zone, arr))
						return false;
					value = move(arr);
					break;
				}
				++zone.begin_zone;
			}
			return true;
		}

		void serialize(ObjectStore object, bool app = false)
		{
			bool coma_needed = false;
			if (!app)_str.clear();
			_str.push_back('{');
			for (auto& pair : object)
			{
				if (coma_needed)
					_str.push_back(',');
				else
					coma_needed = true;

				_str.push_back('"');
				_str.append(pair.first);
				_str.push_back('"');
				_str.push_back(':');
				value_to_JSON(pair.second);
			}
			_str.push_back('}');
		}
		void serialize(ArrayStore arr, bool app = false)
		{
			bool coma_needed = false;
			if (!app)_str.clear();
			_str.push_back('[');
			for (auto& value : arr)
			{
				if (coma_needed)
					_str.push_back(',');
				else
					coma_needed = true;
				value_to_JSON(value);
			}
			_str.push_back(']');
		}

		std::string& dump()
		{
			return _str;
		}
	private:

		std::string str_toupper(std::string s) const
		{
			std::transform(s.begin(), s.end(), s.begin(), [](char c) { return std::toupper(c); } // correct
			);
			return s;
		}

		struct TextZone
		{
			std::string::iterator begin_zone;
			std::string::iterator end_zone;
		};

		static std::string unescape_string(const std::string& text)
		{
			std::string new_text{};
			std::string escape_text{};
			bool escape_started = false;
			for (auto i = text.begin(); i != text.end(); ++i)
			{
				const auto ch = *i;
				if (ch == '\\')
				{
					++i;
					switch (*i)
					{
					case '"':
						new_text.push_back('"');
						break;
					case '\\':
						new_text.push_back('\\');
						break;
					case '/':
						new_text.push_back('/');
						break;
					case 'b':
						new_text.push_back('\b');
						break;
					case 'f':
						new_text.push_back('\f');
						break;
					case 'n':
						new_text.push_back('\n');
						break;
					case 'r':
						new_text.push_back('\r');
						break;
					case 't':
						new_text.push_back('\t');
						break;
					default:;
					}
				}
				else
					new_text.push_back(ch);
			}
			return new_text;
		}

		static std::string escape_string(const std::string& text)
		{
			std::string new_text{};
			for (auto i = text.begin(); i != text.end(); ++i)
			{
				switch (*i)
				{
				case '"':
					new_text.append("\\\"");
					break;
				case '\\':
					new_text.append("\\\\");
					break;
				case '/':
					new_text.append("\\/");
					break;
				case '\b':
					new_text.append("\\b");
					break;
				case '\f':
					new_text.append("\\f");
					break;
				case '\n':
					new_text.append("\\n");
					break;
				case '\r':
					new_text.append("\\r");
					break;
				case '\t':
					new_text.append("\\t");
					break;
				default:
					new_text.push_back(*i);
				}
			}
			return new_text;
		}

		bool parse_value(TextZone& zone, Value& value)	//не массив и не объект. число или строка или null
		{
			if ((*zone.begin_zone) == 'n' || (*zone.begin_zone) == 'N')	// null?
			{
				auto end_null = zone.begin_zone;
				advance(end_null, 4);
				auto substr1 = str_toupper(std::string{ zone.begin_zone,end_null });
				if (substr1 == "NULL")	//обнаружен NULL
				{
					zone.begin_zone = end_null;
					value = Value{};
					return true;
				}
			}
			else if ((*zone.begin_zone) == '"')	//string
			{
				++zone.begin_zone;	//съедаем кавычку
				auto start = zone.begin_zone;	//начало зоны поиска кавычки
				auto stop = zone.end_zone;	//конец зоны поиска кавычки
				for (;;)
				{
					auto kav = std::find(start, stop, '"');
					if (kav != stop)	//" нашлась
					{
						--kav;
						if (*kav == '\\')
						{
							//кавычка экранирована, ищем дальше
							++kav;	//пропуск символа экранировки
							++kav;  //пропуск "
							start = kav;
							stop = zone.end_zone;
							continue;
						}
						else
						{
							++kav;
							value = unescape_string(std::string{ zone.begin_zone,kav });
							zone.begin_zone = kav;
							++zone.begin_zone;	//съедаем кавычку
							return true;
						}
					}
				}
			}
			else if (((*zone.begin_zone) >= '0' && (*zone.begin_zone) <= '9') || (*zone.begin_zone) == '.' || (*zone.
				begin_zone) == '+' || (*zone.begin_zone) == '-')	//число?
			{
				size_t endposd{};
				size_t endposi{};
				auto text = std::string{ zone.begin_zone, zone.end_zone };
				auto d_val = std::stod(text, &endposd);
				auto i_val = std::stoll(text, &endposi);
				if (endposd > endposi)
				{
					//double
					value = d_val;
					advance(zone.begin_zone, endposd);
					return true;
				}
				else
				{
					//int
					value = i_val;
					advance(zone.begin_zone, endposi);
					return true;
				}
			}
			else if ((*zone.begin_zone) == 't' || (*zone.begin_zone) == 'T')	// true?
			{
				auto end_null = zone.begin_zone;
				advance(end_null, 4);
				auto substr1 = str_toupper(std::string{ zone.begin_zone,end_null });
				if (substr1 == "TRUE")	//обнаружен TRUE
				{
					zone.begin_zone = end_null;
					value = Value{ true };
					return true;
				}
			}
			else if ((*zone.begin_zone) == 'f' || (*zone.begin_zone) == 'F')	// true?
			{
				auto end_null = zone.begin_zone;
				advance(end_null, 5);
				auto substr1 = str_toupper(std::string{ zone.begin_zone,end_null });
				if (substr1 == "FALSE")	//обнаружен NULL
				{
					zone.begin_zone = end_null;
					value = Value{ false };
					return true;
				}
			}
			return false;
		}

		bool parse_object(TextZone& zone, ObjectStore& obj)
		{
			if (*zone.begin_zone == '{')
			{
				++zone.begin_zone;
			}
			else
				return false;
			bool splitter_nedded = true;
			bool coma_needed = false;
			std::string name_of_element{};

			while (zone.begin_zone != zone.end_zone)
			{
				if ((*zone.begin_zone) == '}')
				{
					//end of object
					++zone.begin_zone;
					return true;
				}
				else if (splitter_nedded)
				{

					if ((*zone.begin_zone) == '"')	//string name
					{
						++zone.begin_zone;	//съедаем кавычку
						auto start = zone.begin_zone;	//начало зоны поиска второй кавычки
						auto stop = zone.end_zone;	//конец зоны поиска второй кавычки
						for (;;)
						{
							auto kav = std::find(start, stop, '"');
							if (kav != stop)	//" нашлась
							{
								--kav;
								if (*kav == '\\')
								{
									//кавычка экранирована, ищем дальше
									++kav;	//пропуск символа экранировки
									++kav;  //пропуск "
									start = kav;
									stop = zone.end_zone;
									continue;
								}
								else
								{
									++kav;
									name_of_element = unescape_string(std::string{ zone.begin_zone,kav });
									zone.begin_zone = kav;
									++zone.begin_zone;	//съедаем кавычку
									break;
								}
							}
							else
								return false;
						}
						continue;
					}
					else if ((*zone.begin_zone) == ':')
					{
						if (!splitter_nedded)
							return false;	//двоеточие тут не нужно
						++zone.begin_zone;	//пропуск :
						splitter_nedded = false;
						continue;
					}
					else if ((*zone.begin_zone) == ',')
					{
						return false;
					}

				}
				else
				{
					//парсим значение после двоеточия
					if (*zone.begin_zone == '[')
					{
						//найдено начало массива
						if (coma_needed)
							return false;	//пропущена запятая
						ArrayStore sub_arr;
						if (!parse_array(zone, sub_arr))	//парсим массив
							return false;
						obj[name_of_element] = move(sub_arr);
						coma_needed = true;
						continue;
					}
					else if (*zone.begin_zone == '{')
					{
						//найдено начало Объекта
						if (coma_needed)
							return false;	//пропущена запятая
						ObjectStore sub_obj;
						if (!parse_object(zone, sub_obj))//парсим объект
							return false;
						obj[name_of_element] = move(sub_obj);
						coma_needed = true;
						continue;
					}
					else if (json_value_charset.find(*zone.begin_zone) != json_value_charset.npos)
					{
						if (coma_needed)
							return false;	//пропущена запятая
						Value v;
						if (!parse_value(zone, v)) // парсим значение
							return false;		   // не распарсилось
						obj[name_of_element] = std::move(v);    // заносим значение в массив
						coma_needed = true;
						continue;
					}
					else if ((*zone.begin_zone) == ',')
					{
						if (coma_needed)
						{
							coma_needed = false;
							name_of_element = {};
							splitter_nedded = true;
							++zone.begin_zone;
							continue;
						}
						else
							return false;	//неожиданная запятая
					}
				}
				++zone.begin_zone;
			}
			return false;
		}

		bool parse_array(TextZone& zone, ArrayStore& arr)
		{
			if (*zone.begin_zone == '[')
			{
				++zone.begin_zone;
			}
			else
				return false;
			bool coma_needed = false;
			while (zone.begin_zone != zone.end_zone)
			{

				if (*zone.begin_zone == '[')
				{
					//найдено начало массива
					if (coma_needed)
						return false;	//пропущена запятая
					ArrayStore sub_arr;
					if (!parse_array(zone, sub_arr))	//парсим массив
						return false;
					arr.push_back(move(sub_arr));
					coma_needed = true;
					continue;
				}
				else if (*zone.begin_zone == '{')
				{
					//найдено начало Объекта
					if (coma_needed)
						return false;	//пропущена запятая
					ObjectStore sub_obj;
					if (!parse_object(zone, sub_obj))//парсим объект
						return false;
					arr.push_back(move(sub_obj));
					coma_needed = true;
					continue;
				}
				else if (json_value_charset.find(*zone.begin_zone) != json_value_charset.npos)
				{
					if (coma_needed)
						return false;	//пропущена запятая
					Value v;
					if (!parse_value(zone, v)) // парсим значение
						return false;		   // не распарсилось
					arr.push_back(std::move(v));    // заносим значение в массив
					coma_needed = true;
					continue;
				}
				else if ((*zone.begin_zone) == ',')
				{
					if (coma_needed)
						coma_needed = false;
					else
						return false;	//неожиданная запятая
				}
				else if ((*zone.begin_zone) == ']')
				{
					//end of array
					++zone.begin_zone;
					break;
				}
				++zone.begin_zone;
			}
			return true;
		}

		void value_to_JSON(Value& value)
		{
			char buf[32];

			switch (value.value_type())
			{
			case ValueType::Null:
				_str.append("null");
				break;
			case ValueType::Double:
				auto r = to_chars(buf, buf + 30, value.get_double(), std::chars_format::scientific, 15);
				*r.ptr = 0;
				_str.append(buf);
				break;
			case ValueType::Integer:
				r = std::to_chars(buf, buf + 30, value.get_int());
				*r.ptr = 0;
				_str.append(buf);
				break;
			case ValueType::String:
				_str.push_back('"');
				_str.append(escape_string(value.get_string()));
				_str.push_back('"');
				break;
			case ValueType::Object:
				serialize(value.get_object(), true);
				break;
			case ValueType::Array:
				serialize(value.get_array(), true);
				//array_print(value.get_array());
				break;
			case ValueType::Boolean:
				if (value.get_boolean())
					_str.append("true");
				else
					_str.append("false");
				break;
			default:;
			}
		}
		std::string _str;
	};

};