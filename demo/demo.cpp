#include "zjson.h"
#include <iostream>
#include <vector>
#include <set>

int main()
{
	std::cout << "Hello Every One\n";
	//Construction and output to stream:
	json::Object jObj{
		{"field1", 1}
		, {"field2", "val2" }
		, {"feld3", json::Array{1, 2, 3}}
	};
	std::cout << "Staticly constructed object: " << jObj << std::endl;
	std::cout << "Sorted and indented object: " << json::setOStreamIdent(2) << json::setOStreamSorted << jObj << std::endl;
	//deserialization from stream:
	const auto s = R"({"field1":1,"field2":"val2","feld3":[1,2,3]})";
	std::istringstream iss{ s };
	json::Object jObj2;
	iss >> jObj2;
	std::cout << "Deserialized obj is " << (jObj == jObj2 ? "the same" : " NOT the same") << std::endl;
	//Array specific:
	json::Array jArr{ 1, 2, 3 };
	auto setInt{ jArr.toStdSet<std::set<int>>(&json::Value::asInt32) };
	std::cout << json::Array::fromStdArray(setInt) << std::endl;
	std::cout << json::Array::fromStdArray(std::vector{"a", "b", "c"}) << std::endl;

	//Serialization and deserialization in user defined types:

	class A
	{
		int64_t i;
		const std::string s;
	public:
		A(const json::Object& jObj)
			: i(jObj["i"].asIntSafe()), s(jObj["s"].asStringSafe())
		{}
		A(int64_t i_, std::string s_)
			: i(i_), s(std::move(s_))
		{}
		json::Object serialize() const
		{
			return { {"i", i}, {"s", s} };
		}
	}a{ 42, "The Answer" };
	using VecA = std::vector<A>;
	VecA vecA{ a, a, a };
	const auto jVecA = json::Array::fromStdArraySer(vecA);
	const auto vecA2 = jVecA.toStdObjArray<VecA>();
	std::cout << json::Array::fromStdArraySer(vecA2) << std::endl;

	class B
	{
		int64_t i{ 0 };
		int64_t j{ 0 };
		std::string s;
	public:
		B(const json::Value& jV)
		{
			if (jV.isObject())
			{
				const auto jObj = jV.asObject();
				i = jObj["i"].asInt();//will throw if "i" field is missing or not an int
				j = jObj["j"].asIntSafe();
				s = jObj["s"].asStringSafe();
			}
		}
		B(int64_t i_, int64_t j_, const std::string& s_)
			: i(i_), j(j_), s(s_)
		{}
		json::Value serialize() const
		{
			return json::Object{ {"i", i}, {"j", j}, {"s", s} };
		}
	} b{ 6, 7, "The Question" };
	const auto jB = b.serialize();
	const B bb{ jB };
	std::cout << jB.asObjectSafe() << std::endl;
	std::cout << bb.serialize().asObjectSafe() << std::endl;
}
