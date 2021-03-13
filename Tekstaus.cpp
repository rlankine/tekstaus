
/*
MIT License

Copyright (c) 2020 Risto Lankinen

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

#include "Tekstaus.h"
// #define VERBOSE
#include "Tools.h"

/***********************************************************************************************************************
*** String::data
***********************************************************************************************************************/

struct String::data : public Shared
{
};

/***********************************************************************************************************************
*** StrHead
***********************************************************************************************************************/

struct StrHead final : public String::data, private ObjectGuard<StrHead>
{
};

/***********************************************************************************************************************
*** StrTail
***********************************************************************************************************************/

struct StrTail final : public String::data, private ObjectGuard<StrTail>
{
};

/***********************************************************************************************************************
*** StrSum
***********************************************************************************************************************/

struct StrSum final : public String::data, private ObjectGuard<StrSum>
{
};

/***********************************************************************************************************************
*** String
***********************************************************************************************************************/

String::String()
{
	TODO;
}

String::String(String const&)
{
	TODO;
}

String::String(String const&, String const&)
{
	TODO;
}

String::String(String const&, size_t)
{
	TODO;
}

String::String(size_t, String const&)
{
	TODO;
}

String::~String()
{
}

//**********************************************************************************************************************
