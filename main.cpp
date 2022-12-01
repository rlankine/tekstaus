
#include "Tekstaus.h"

#include <assert.h>
#include <iostream>

using std::cout;
using std::endl;

void evaluate(char const*p)
{
	assert(p);
	// cout << p << endl;
}

int test0()
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

	for (auto& head : s) for (auto& tail : s)
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

int main() try
{
	String none;
	String buffer("Mustan kissan paksut posket");
	String head = buffer.Head(18);
	String tail = buffer.Tail(9);
	String sum = head + tail;
	String repeat = String('#', 27);

	evaluate(none + buffer);
	evaluate(none + repeat);
	evaluate(repeat + buffer);
	evaluate(repeat + repeat);
	evaluate(none + tail);
	evaluate(none + head);
	evaluate(none + sum);
	evaluate(sum + none);
	evaluate(tail + none);
	evaluate(head + none);
	evaluate(sum + none);
	evaluate(sum + sum);

	evaluate(String(""));

	evaluate(buffer.Head(-5));
	evaluate(buffer.Tail(-5));
	evaluate(head.Head(-5));
	evaluate(head.Tail(-5));
	evaluate(tail.Head(-5));
	evaluate(tail.Tail(-5));
	evaluate(sum.Head(-5));
	evaluate(sum.Tail(-5));

	evaluate(buffer.Head(555));
	evaluate(buffer.Tail(555));
	evaluate(head.Head(555));
	evaluate(head.Tail(555));
	evaluate(tail.Head(555));
	evaluate(tail.Tail(555));
	evaluate(sum.Head(555));
	evaluate(sum.Tail(555));

	head = buffer.Head(3);
	tail = buffer.Tail(3);

	evaluate(head);
	evaluate(tail);
	evaluate(head+tail);
	evaluate(tail+head);

	for (int i = 0; i < 100; ++i) sum = "(" + sum;
	evaluate(sum);
	for (int i = 0; i < 100; ++i) sum = sum + ")";
	evaluate(sum);
	for (int i = 0; i < 100; ++i) sum = "("+ sum + ")";
	evaluate(sum);

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
