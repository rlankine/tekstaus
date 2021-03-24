
#include "Tekstaus.h"

#include <iostream>

using std::cout;
using std::endl;

String Reverse1(String const& r)
{
	auto mid = r.Length() / 2;

	return mid ? Reverse1(r.Tail(mid)) + Reverse1(r.Head(mid)) : r;
}

String Reverse2(String const& r)
{
	return r.Length() > 1 ? Reverse2(r.Tail(1)) + r.Head(1) : r;
}

int main() try
{
	String s('x', 99);
	cout << s << endl;

#if 1
	String a = String('<', 1);
	String b = String('<', 4);

	cout << (a + b).Head(3) << "|" << (a + b).Tail(3) << endl;

	String HW = "HelloWorld";
	String greeting = String('>',5) + HW.Head(5) + ", " + HW.Tail(5) + "!" + String('<',5);
	cout << greeting << endl;
	greeting = Reverse1(greeting);
	cout << greeting << endl;
	greeting = Reverse2(greeting);
	cout << greeting << endl;
#endif
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
