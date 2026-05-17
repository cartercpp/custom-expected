#include <iostream>
#include <string>
#include "expected.hpp"

static constexpr expected<int, const char*> Foo(int num)
{
	if (num > 0)
		return num;

	return unexpected{ "67" };
}

int main()
{
	constexpr auto e = Foo(-1)
		.transform([](int n) {return n / 2; })
		.and_then([](int n)
			-> expected<int, const char*> {
				return n * 2;
			});

	std::cout << e.error();
}