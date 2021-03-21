
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
	String HW = "HelloWorld";
	String greeting = HW.Head(5) + ", " + HW.Tail(5) + "!";
	cout << greeting << endl;
	greeting = Reverse1(greeting);
	cout << greeting << endl;
	greeting = Reverse2(greeting);
	cout << greeting << endl;
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
