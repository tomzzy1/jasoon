#include "json.h"
#include <chrono>

using namespace jasoon;
using namespace std::chrono;

void test()
{
	std::ifstream f("citm_catalog.json");
	char c;
	std::string s;
	while (!f.eof())
	{
		f >> c;
		s += c;
	}
	std::cout << static_cast<int>(s[0]);
	auto start = system_clock::now();
	for (int i = 0; i < 10000; ++i)
	auto j = Json::parse(s);
	auto end = system_clock::now();
	std::cout << duration_cast<milliseconds>(end - start).count();
	std::cout << j.stringify();
}

int main()
{
	//test();
	auto j = Json::parse("{ \"happy\": true, \"pi\": 3.141 }"); 
	std::cout << std::boolalpha << j["pi"].is_float() << '\n';
	bool b = j["happy"];
	std::cout << b << '\n';
	auto j2 = Json::parse("data.json", InputMode::File);
	std::string s = j2["web-app"]["servlet"][0]["servlet-name"];
	std::cout << s << std::endl;
	std::cout << j2.stringify() << '\n';
	auto j3 = "{ \"sad\": false, \"e\": 2.718 }"_json;
	std::cout << j3["sad"].is_boolean() << '\n';
	Json j4 =
	{
		{ "pi", 3.141 },
		{ "happy", true },
		{ "name", "Niels" },
		{ "nothing", nullptr },
		{ "answer",
			{
				{ "everything", 42 }
			} 
		},
		{ "list",{ 1, 0, 2 } },
		{ "object",{
			{ "currency", "USD" },
			{ "value", 42.99 }
		} }
	};
	std::cout << static_cast<int>(j4["list"][1]) << '\n';
	auto j5 = j4;
	std::cout << static_cast<int>(j5["list"][2]);
	std::cin.get();
}