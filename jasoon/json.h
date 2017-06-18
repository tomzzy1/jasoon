#pragma once
#include <variant>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <initializer_list>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string_view>
#include <type_traits>

namespace jasoon
{

	enum class Token
	{
		Object_begin,	 //{
		Object_end,		 //}
		Array_begin,     //[
		Array_end,       //]
		Name_separator,  //:
		Value_separator, //,
		String,
		Interger,
		Float,
		True,
		False,
		Null
	};

	enum class Json_type
	{
		Object,
		Array,
		String,
		Interger,
		Float,
		Boolean,
		Null
	};

	enum class InputMode
	{
		String, File
	};

	template<
		template<typename Key, typename Value, typename... Args>
	typename Object_type = std::unordered_map,
		template<typename Value, typename... Args>
	typename Array_type = std::vector,
		typename String_type = std::string,
		typename Interger_type = std::int64_t,
		typename Float_type = double,
		typename Boolean_type = bool,
		template<typename T> typename Allocator_type = std::allocator>
	class Basic_json
	{
	public:
		using value_type = Basic_json;

		using reference = value_type&;

		using const_reference = const reference;

		using size_type = size_t;

		using object_t = Object_type<String_type,
			Basic_json,
			std::hash<String_type>,
			std::equal_to<String_type>,
			Allocator_type<std::pair<const String_type, Basic_json>>>;

		using object_ptr = std::unique_ptr<object_t>; //used in varaint

		using array_t = Array_type<Basic_json, Allocator_type<Basic_json>>;

		using array_ptr = std::unique_ptr<array_t>; //used in varaint

		using string_t = String_type;

		using string_ptr = std::unique_ptr<string_t>; //used in varaint

		using interger_t = Interger_type;

		using float_t = Float_type;

		using boolean_t = Boolean_type;

		using Json_value = std::variant<
			object_ptr,
			array_ptr,
			string_ptr,
			interger_t,
			float_t,
			boolean_t>;
	public:

		bool is_object() const noexcept
		{
			return type == Json_type::Object;
		}

		bool is_array() const noexcept
		{
			return type == Json_type::Array;
		}

		bool is_string() const noexcept
		{
			return type == Json_type::String;
		}

		bool is_interger() const noexcept
		{
			return type == Json_type::Interger;
		}

		bool is_float() const noexcept
		{
			return type == Json_type::Float;
		}

		bool is_boolean() const noexcept
		{
			return type == Json_type::Boolean;
		}

		bool is_null() const noexcept
		{
			return type == Json_type::Null;
		}

		template<typename T>
		operator T() const //implicit cast operation
		{
			if constexpr(std::is_same_v<T, object_t>)
				return *std::get<object_ptr>(value);
			else if constexpr(std::is_same_v<T, array_t>)
				return *std::get<array_ptr>(value);
			else if constexpr(std::is_same_v<T, string_t>)
				return *std::get<string_ptr>(value);
			else
				return std::get<T>(value);
		}

	public:
		void push_back(const_reference element) const
		{
			if (is_object() 
				&& element.is_array() 
				&& element.size() == 2
				&& element[0].is_string())
				std::get<object_ptr>(value)
				->emplace(
					*std::get<string_ptr>(element[0].value), element[1]);
			else
				std::get<array_ptr>(value)->push_back(element);
		}
		void push_back(value_type&& element)
		{
			if (is_object()
				&& element.is_array()
				&& element.size() == 2
				&& element[0].is_string())
				std::get<object_ptr>(value)
				->emplace(
					std::move(*std::get<string_ptr>(element[0].value)), std::move(element[1]));
			else
				std::get<array_ptr>(value)->push_back(std::move(element));
		}
		size_type size() const
		{
			if (is_object())
				return std::get<object_ptr>(value)->size();
			if (is_array())
				return std::get<array_ptr>(value)->size();
		}
	private:
		class Lexer
		{
		public:

			Token getToken()
			{
				while (std::isspace(last_char)) //skip space
					last_char = stream->get();
				switch (last_char)
				{
				case'{':
				{
					last_char = stream->get();
					return Token::Object_begin;
				}
				case'}':
				{
					last_char = stream->get();
					return Token::Object_end;
				}
				case'[':
				{
					last_char = stream->get();
					return Token::Array_begin;
				}
				case']':
				{
					last_char = stream->get();
					return Token::Array_end;
				}
				case':':
				{
					last_char = stream->get();
					return Token::Name_separator;
				}
				case',':
				{
					last_char = stream->get();
					return Token::Value_separator;
				}
				case'"':
					return scanString();
				case'-':
				case'0':
				case'1':
				case'2':
				case'3':
				case'4':
				case'5':
				case'6':
				case'7':
				case'8':
				case'9':
					return scanNumber();
				case't': //true
				case'f': //false
					return scanBoolean();
				case'n': //null
					return scanNull();
				default:
					break;
				}
			}

			template<typename T>
			decltype(auto) getValue() const
			{
				return std::get<T>(value);
			}

			void setStream(const string_t& s, InputMode mode)
			{
				if (mode == InputMode::String)
					stream = std::make_unique<std::istringstream>(s);
				else //mode == InputMode::File
					stream = std::make_unique<std::ifstream>(s);
			}

		private:
			std::unique_ptr<std::istream> stream;
			int last_char = ' ';
			std::variant<string_t, interger_t, float_t> value;

			Token scanString()
			{
				string_t s;
				bool escape = false;
				last_char = stream->get();
				while (escape || last_char != '"')
				{
					if (!escape && last_char == '\\')
					{
						escape = true;
					}
					else
					{
						escape = false;
						s += last_char;
					}
					last_char = stream->get();
				}
				last_char = stream->get();
				value = s;
				return Token::String;
			}

			Token scanNumber()
			{
				string_t num;
				bool is_float = false;
				while (std::isdigit(last_char)
					|| last_char == '.'
					|| last_char == '+'
					|| last_char == '-'
					|| last_char == 'e'
					|| last_char == 'E')
				{
					if (last_char == '.')
						is_float = true;
					num += last_char;
					last_char = stream->get();
				}
				if (is_float)
				{
					if constexpr(std::is_same_v<float_t, double>)
						value = std::stod(num);
					else if constexpr(std::is_same_v<float_t, float>)
						value = std::stof(nums);
					return Token::Float;
				}
				else
				{
					value = std::stoll(num);
					return Token::Interger;
				}
			}

			Token scanBoolean()
			{
				using namespace std::string_view_literals;
				static constexpr auto true_literal = "true"sv;
				static constexpr auto false_literal = "false"sv;
				if (last_char == 't')
				{
					for (const auto& c : true_literal)
					{
						if (last_char == c)
							last_char = stream->get();
						else
						{
							std::cerr << "invalid true literal";
							throw;
						}
					}
					return Token::True;
				}
				else //last_char == 'f'
				{
					for (const auto& c : false_literal)
					{
						if (last_char == c)
							last_char = stream->get();
						else
						{
							std::cerr << "invalid false literal";
							throw;
						}
					}
					return Token::False;
				}
			}

			Token scanNull()
			{
				using namespace std::string_view_literals;
				static constexpr auto null_literal = "null"sv;
				for (const auto& c : null_literal)
				{
					if (last_char == c)
						last_char = stream->get();
					else
					{
						std::cerr << "invalid null literal";
						throw;
					}
				}
				return Token::Null;
			}
		};

		class Parser
		{
		public:
			Basic_json parse(string_t s, InputMode mode)
			{
				lexer.setStream(s, mode);
				const auto token = lexer.getToken();
				if (token == Token::Array_begin)
					return parseArray();
				else if (token == Token::Object_begin)
					return parseObject();
				else
				{
					std::cerr << "invalid input";
					throw;
				}
			}
		private:
			Basic_json parseObject()
			{
				Basic_json object(Json_type::Object); //empty json object
				auto token = lexer.getToken();
				string_t name;
				bool is_name = true;
				while (token != Token::Object_end)
				{
					switch (token)
					{
					case Token::Object_begin:
						object.push_back({ name,parseObject() });
						break;
					case Token::Array_begin:
						object.push_back({ name,parseArray() });
						break;
					case Token::Name_separator:
						is_name = false;
						break;
					case Token::Value_separator:
						is_name = true;
						break;
					case Token::String:
					{
						if (is_name)
							name = lexer.getValue<string_t>();
						else
							object.push_back({ name,lexer.getValue<string_t>() });
						break;
					}
					case Token::Interger:
						object.push_back({ name,lexer.getValue<interger_t>() });
						break;
					case Token::Float:
						object.push_back({ name,lexer.getValue<float_t>() });
						break;
					case Token::True:
						object.push_back({ name,true });
						break;
					case Token::False:
						object.push_back({ name,false });
						break;
					case Token::Null:
						object.push_back({ name,nullptr });
						break;
					case Token::Array_end:
					default:
						break;
					}
					token = lexer.getToken();
				}
				return object;
			}
			Basic_json parseArray()
			{
				Basic_json array(Json_type::Array); //empty json array
				auto token = lexer.getToken();
				while (token != Token::Array_end)
				{
					switch (token)
					{
					case Token::Object_begin:
						array.push_back(parseObject());
						break;
					case Token::Array_begin:
						array.push_back(parseArray());
						break;
					case Token::Value_separator:
						break;
					case Token::String:
						array.push_back(lexer.getValue<string_t>());
						break;
					case Token::Interger:
						array.push_back(lexer.getValue<interger_t>());
						break;
					case Token::Float:
						array.push_back(lexer.getValue<float_t>());
						break;
					case Token::True:
						array.push_back(true);
						break;
					case Token::False:
						array.push_back(false);
						break;
					case Token::Null:
						array.push_back(nullptr);
						break;
					case Token::Object_end:
					case Token::Name_separator:
					default:
						break;
					}
					token = lexer.getToken();
				}
				return array;
			}
			Lexer lexer;
		};

		static Parser parser;

	public:
		template<typename T>
		reference operator[](T index)
		{
			if constexpr(std::is_integral_v<T>)
			{
				if (is_array())
					return std::get<array_ptr>(value)->operator[](index);
			}
			else if constexpr(std::is_convertible_v<T, string_t>) //T can be char* , std::string ...
			{
				if (is_object())
					return std::get<object_ptr>(value)->operator[](index);
			}
		}

		template<typename T>
		const_reference operator[](T index) const
		{
			if constexpr(std::is_integral_v<T>)
			{
				if (is_array())
					return std::get<array_ptr>(value)->operator[](index);
			}
			else if constexpr(std::is_convertible_v<T, string_t>)
			{
				if (is_object())
					return std::get<object_ptr>(value)->operator[](index);
			}
		}

	private:

		Json_type type;

		Json_value value;

	public:
		Basic_json() :type(Json_type::Null) {}

		Basic_json(interger_t i) :type(Json_type::Interger), value(i) {}

		Basic_json(float_t f) :type(Json_type::Float), value(f) {}

		Basic_json(boolean_t b) :type(Json_type::Boolean), value(b) {}

		Basic_json(std::nullptr_t n) :type(Json_type::Null) {}

		Basic_json(const std::initializer_list<Basic_json>& list)
		{
			const auto is_object = std::all_of(list.begin(), list.end(), [](const auto& element)
			{
				return element.is_array() && element.size() == 2 && element[0].is_string();
			});

			if (is_object)
			{
				type = Json_type::Object;
				value = std::make_unique<object_t>();
				for (const auto& element : list)
					std::get<object_ptr>(value)
					->emplace(*std::get<string_ptr>(element[0].value), element[1]);
			}
			else
			{
				type = Json_type::Array;
				value = std::make_unique<array_t>(list.begin(), list.end());
			}
		}

		Basic_json(const object_t& o) :type(Json_type::Object), value(std::make_unique<object_t>(o)) {}

		Basic_json(const array_t& a) :type(Json_type::Array), value(std::make_unique<array_t>(a)) {}

		Basic_json(const string_t& s) :type(Json_type::String), value(std::make_unique<string_t>(s)) {}

		Basic_json(Json_type t) :type(t)
		{
			switch (type)
			{
			case Json_type::Object:
				value = std::make_unique<object_t>();
				break;
			case Json_type::Array:
				value = std::make_unique<array_t>();
				break;
			case Json_type::String:
				value = std::make_unique<string_t>();
				break;
			case Json_type::Interger:
				value = interger_t(0);
				break;
			case Json_type::Float:
				value = float_t(0.0);
				break;
			case Json_type::Boolean:
				value = false;
				break;
			case Json_type::Null:
				break;
			default:
				break;
			}
		}
		Basic_json(const Basic_json& other)
		{
			*this = other;
		}
		reference operator=(const Basic_json& other)
		{
			if (this != &other)
			{
				type = other.type;
				switch (type)
				{
				case Json_type::Object:
					value = std::make_unique<object_t>(*std::get<object_ptr>(other.value));
					break;
				case Json_type::Array:
					value = std::make_unique<array_t>(*std::get<array_ptr>(other.value));
					break;
				case Json_type::String:
					value = std::make_unique<string_t>(*std::get<string_ptr>(other.value));
					break;
				case Json_type::Interger:
					value = std::get<interger_t>(other.value);
					break;
				case Json_type::Float:
					value = std::get<float_t>(other.value);
					break;
				case Json_type::Boolean:
					value = std::get<boolean_t>(other.value);
					break;
				case Json_type::Null:
					break;
				default:
					break;
				}
			}
			return *this;
		}
		Basic_json(Basic_json&& other) noexcept
		{
			*this = std::move(other);
		}
		reference operator=(Basic_json&& other) noexcept
		{
			if (this != &other)
			{
				type = other.type;
				switch (type)
				{
				case Json_type::Object:
					value = std::move(std::get<object_ptr>(other.value));
					break;
				case Json_type::Array:
					value = std::move(std::get<array_ptr>(other.value));
					break;
				case Json_type::String:
					value = std::move(std::get<string_ptr>(other.value));
					break;
				case Json_type::Interger:
					value = std::get<interger_t>(other.value);
					break;
				case Json_type::Float:
					value = std::get<float_t>(other.value);
					break;
				case Json_type::Boolean:
					value = std::get<boolean_t>(other.value);
					break;
				case Json_type::Null:
					break;
				default:
					break;
				}
			}
			return *this;
		}

		~Basic_json() = default;

	public:
		static value_type parse(const string_t& s, InputMode mode = InputMode::String)
		{
			return parser.parse(s, mode);
		}
	};

	using Json = Basic_json<>;

	Json::Parser Json::parser;

}
