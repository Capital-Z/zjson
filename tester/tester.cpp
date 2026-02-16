#include "zjson.h"
#include <iostream>

int main()
{
	std::cout << "Hello Every One\n";
	json::Object jObj{
		{"field1", 1}
		, {"field2", "val2" }
		, {"feld3", json::Array{1, 2, 3}}
	};
	std::cout << jObj << std::endl;
	std::cout << json::setOStreamIdent(2) << json::setOStreamSorted << jObj << std::endl;
	const auto s = R"({"field1":1,"field2":"val2","feld3":[1,2,3]})";
	std::istringstream iss{ s };
	json::Object jObj2;
	iss >> jObj2;
	std::cout << "serialized obj is " << (jObj == jObj2 ? "the same" : " NOT the same") << std::endl;
}
