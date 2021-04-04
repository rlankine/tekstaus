
#include "Tekstaus.h"

#include <iostream>

using std::cout;
using std::endl;

void evaluate(char const*p)
{
	// cout << p << endl;
}

int main() try
{
	String a;
	String b("");
	String c("HelloWorld");
	String d(c, 5);
	String e(5, c);
	String f(c, c);

	String s[] = { a, b, c, d, e, f };

	for (auto& item : s)
	{
		evaluate(item);
	}

	for (auto& head : s) for(auto & tail : s)
	{
		evaluate(head + tail);

		for (int i = -1; i < 12; ++i)
		{
			auto data = head + tail;

			evaluate(data.Head(i));
			evaluate(data.Tail(i));
		}
	}

	return EXIT_SUCCESS;
}
catch (char const* p)
{
	cout << endl << p << endl;
}
catch (...)
{
	cout << endl << "Diva tantrum!!!" << endl;
}
