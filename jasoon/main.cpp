#include "json.h"

using namespace jasoon;

int main()
{
	auto j = Json::parse("{ \"happy\": true, \"pi\": 3.141 }");
	auto j2 = Json::parse("data.json", InputMode::File);
	std::string s = j2["web-app"]["servlet"][0]["servlet-name"];
	std::cout << s;
	std::cout << std::boolalpha << j["pi"].is_float();
	bool b = j["happy"];
	std::cout << b;
	std::cin.get();
}