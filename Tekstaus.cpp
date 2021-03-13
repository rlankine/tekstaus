
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
	virtual String::data const* append(String::data const*) const { TODO; }
	virtual String::data const* head(int) const { TODO; }
	virtual String::data const* tail(int) const { TODO; }

	virtual bool get(char*, size_t) const = 0;
	virtual int length() const = 0;
	virtual size_t size() const = 0;
};

/***********************************************************************************************************************
*** StrEmpty
***********************************************************************************************************************/

struct StrEmpty final : public String::data, private ObjectGuard<StrEmpty>
{
	bool get(char* p, size_t n) const override final { if (p && n) *p = '\0'; return (p && n); }
	int length() const override final { return 0; }
	size_t size() const override final { return 1; }
};

/***********************************************************************************************************************
*** StrBuffer
***********************************************************************************************************************/

struct StrBuffer final : public String::data, private ObjectGuard<StrBuffer>
{
	bool get(char* p, size_t n) const override final { TODO; }
	int length() const override final { TODO; }
	size_t size() const override final { TODO; }
};

/***********************************************************************************************************************
*** StrHead
***********************************************************************************************************************/

struct StrHead final : public String::data, private ObjectGuard<StrHead>
{
	bool get(char* p, size_t n) const override final { TODO; }
	int length() const override final { TODO; }
	size_t size() const override final { TODO; }
};

/***********************************************************************************************************************
*** StrTail
***********************************************************************************************************************/

struct StrTail final : public String::data, private ObjectGuard<StrTail>
{
	bool get(char* p, size_t n) const override final { TODO; }
	int length() const override final { TODO; }
	size_t size() const override final { TODO; }
};

/***********************************************************************************************************************
*** StrSum
***********************************************************************************************************************/

struct StrSum final : public String::data, private ObjectGuard<StrSum>
{
	bool get(char* p, size_t n) const override final { TODO; }
	int length() const override final { TODO; }
	size_t size() const override final { TODO; }
};

/***********************************************************************************************************************
*** String
***********************************************************************************************************************/

String::String()
{
	TODO;
}

String::String(String const& r) : pData(Shared::Clone(r.pData))
{
}

String::String(String const& r, String const& s) : pData(r.pData->append(s.pData))
{
}

String::String(String const& r, int n) : pData(r.pData->head(n))
{
}

String::String(int n, String const& r) : pData(r.pData->tail(n))
{
}

String::String(char const* p)
{
	TODO;
}

String::String(data const* p) : pData(p)
{
}

String::~String()
{
	Shared::Erase(pData);
}

String& String::operator=(String const& r)
{
	Shared::Clone(r.pData);
	Shared::Erase(pData);
	pData = r.pData;
	return *this;
}

String::operator char const* () const
{
	TODO;
}

bool String::Get(char*p, size_t n) const
{
	return pData->get(p, n);
}

int String::Length() const
{
	return pData->length();
}

size_t String::Size() const
{
	return pData->size();
}

//**********************************************************************************************************************
