
/*
MIT License

Copyright (c) 2022 Risto Lankinen

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

//**********************************************************************************************************************

using Byte_t = char;
using Char_t = char32_t;

/***********************************************************************************************************************
*** String
***********************************************************************************************************************/

struct String final
{
	String();
	String(String const&);
	String(String const&, String const&);
	String(String const&, int);
	String(int, String const&);
	String(char const*);
	String(Char_t, int);
	~String();

	String& operator=(String const&);

	operator char const* () const;

	String Head(int n) const { return String(*this, n); }
	String Tail(int n) const { return String(n, *this); }

	void Get(char*, size_t) const;
	int Length() const;
	size_t Size() const;

	struct data;

private:
	String(data const* p) : pData(p) { }
	mutable data const* pData;
};

//**********************************************************************************************************************

inline String operator+(String const& r, String const& s)
{
	return String(r, s);
}

//**********************************************************************************************************************
