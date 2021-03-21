
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

#include <assert.h>

//**********************************************************************************************************************

#pragma intrinsic(memcmp, memcpy, memset, strcmp, strlen)
#define noexcept

/***********************************************************************************************************************
*** String::data
***********************************************************************************************************************/

struct String::data : public Shared
{
    static String::data const* create(char const*);
    static String::data const* create(Char_t, int);

    virtual String::data const* append(String::data const*) const { TODO; }
    virtual String::data const* head(int) const { TODO; }
    virtual String::data const* tail(int) const { TODO; }
    virtual String::data const* prepend(String::data const*) const { TODO; }
    virtual String::data const* stretch(int) const { TODO; }

    virtual void get(char*, size_t) const noexcept = 0;
    virtual Char_t at(int) const noexcept = 0;
    virtual size_t size() const noexcept = 0;
    virtual size_t size(int) const noexcept = 0;
    virtual int length() const noexcept = 0;
    virtual int length(size_t) const noexcept = 0;

    virtual bool connects(char const*) const noexcept { return false; }
    virtual bool isASCII() const noexcept { return length() == size(); }
    virtual char const* buffer() const noexcept { return nullptr; }
    virtual char const* extent() const noexcept { return nullptr; }
    virtual char const* origin() const noexcept { return nullptr; }
    virtual int depth() const noexcept { return 1; }

    static char const* evaluate(String::data const*&);

private:
    void* operator new(size_t n) { return ::operator new(n); }
};

//**********************************************************************************************************************

static struct final : public String::data
{
    operator String::data const* () const { return Clone(this); }

private:
    String::data const* append(String::data const* p) const override final { return Clone(p); }
    String::data const* head(int) const override final { return Clone(this); }
    String::data const* tail(int) const override final { return Clone(this); }
    String::data const* prepend(String::data const* p) const override final { return Clone(p); }

    void get(char* p, size_t n) const noexcept override final { assert(p && n); memset(p, 0, n); }
    Char_t at(int) const noexcept override final { return 0; }
    size_t size() const noexcept override final { return 0; }
    size_t size(int) const noexcept override final { return 0; }
    int length() const noexcept override final { return 0; }
    int length(size_t) const noexcept override final { return 0; }

    bool connects(char const*) const noexcept override final { TODO; }
    bool isASCII() const noexcept override final { return true; }
    char const* buffer() const noexcept override final { return ""; }
    char const* extent() const noexcept override final { TODO; }
    char const* origin() const noexcept override final { TODO; }
    int depth() const noexcept override final { return 0; }
} const empty;

/***********************************************************************************************************************
*** Buffer
***********************************************************************************************************************/

struct Buffer final : public String::data, private ObjectGuard<Buffer>
{
    Buffer(char const* p, size_t n) : nSize(n), nLength(0)
    {
        assert(p && n);
        memcpy(cBuffer, p, n);
        cBuffer[nSize] = '\0';
    }

    Buffer(String::data const* p, size_t n) : nSize(n), nLength(p->length())
    {
        assert(p);
        p->get(cBuffer, nSize);
        cBuffer[nSize] = '\0';
    }

    void* operator new(size_t n, size_t x, bool = false) { return ::operator new(n + x); }
    void operator delete(void* p, size_t, bool) { ::operator delete(p); }

private:
    ~Buffer() { }

    void* operator new(size_t) = delete;
    void* operator new[](size_t) = delete;

    String::data const* append(String::data const*) const override final { TODO; }
    String::data const* head(int) const override final { TODO; }
    String::data const* tail(int) const override final { TODO; }
    String::data const* prepend(String::data const*) const override final { TODO; }
    String::data const* stretch(int) const override final { TODO; }

    void get(char*, size_t) const noexcept override final { TODO; }
    Char_t at(int) const noexcept override final { TODO; }
    size_t size() const noexcept override final { TODO; }
    size_t size(int) const noexcept override final { TODO; }
    int length() const noexcept override final { if (nLength) return nLength; TODO; }
    int length(size_t) const noexcept override final { TODO; }

    bool connects(char const*) const noexcept override final { TODO; }
    bool isASCII() const noexcept override final { TODO; }
    char const* buffer() const noexcept override final { return cBuffer; }
    char const* extent() const noexcept override final { TODO; }
    char const* origin() const noexcept override final { TODO; }
    int depth() const noexcept override final { TODO; }

    size_t const nSize;
    int const nLength;
    char cBuffer[1];  // <---- This must be the last data item!
};

/***********************************************************************************************************************
*** Head
***********************************************************************************************************************/

struct Head final : public String::data, private ObjectGuard<Head>
{
private:
    String::data const* append(String::data const*) const override final { TODO; }
    String::data const* head(int) const override final { TODO; }
    String::data const* tail(int) const override final { TODO; }
    String::data const* prepend(String::data const*) const override final { TODO; }
    String::data const* stretch(int) const override final { TODO; }

    void get(char*, size_t) const noexcept override final { TODO; }
    Char_t at(int) const noexcept override final { TODO; }
    size_t size() const noexcept override final { TODO; }
    size_t size(int) const noexcept override final { TODO; }
    int length() const noexcept override final { TODO; }
    int length(size_t) const noexcept override final { TODO; }

    bool connects(char const*) const noexcept override final { TODO; }
    bool isASCII() const noexcept override final { TODO; }
    char const* buffer() const noexcept override final { TODO; }
    char const* extent() const noexcept override final { TODO; }
    char const* origin() const noexcept override final { TODO; }
    int depth() const noexcept override final { TODO; }

    String::data const* const p;
    int n;
};

/***********************************************************************************************************************
*** Tail
***********************************************************************************************************************/

struct Tail final : public String::data, private ObjectGuard<Tail>
{
    // Tail(String::data const *p, int n) : pString(p), 

private:
    String::data const* append(String::data const*) const override final { TODO; }
    String::data const* head(int) const override final { TODO; }
    String::data const* tail(int) const override final { TODO; }
    String::data const* prepend(String::data const*) const override final { TODO; }
    String::data const* stretch(int) const override final { TODO; }

    void get(char*, size_t) const noexcept override final { TODO; }
    Char_t at(int) const noexcept override final { TODO; }
    size_t size() const noexcept override final { TODO; }
    size_t size(int) const noexcept override final { TODO; }
    int length() const noexcept override final { TODO; }
    int length(size_t) const noexcept override final { TODO; }

    bool connects(char const*) const noexcept override final { TODO; }
    bool isASCII() const noexcept override final { TODO; }
    char const* buffer() const noexcept override final { TODO; }
    char const* extent() const noexcept override final { TODO; }
    char const* origin() const noexcept override final { TODO; }
    int depth() const noexcept override final { TODO; }

    String::data const* const pString;
    int nLength;
    size_t nSize;
};

/***********************************************************************************************************************
*** Append
***********************************************************************************************************************/

struct Append final : public String::data, private ObjectGuard<Append>
{
private:
    String::data const* append(String::data const*) const override final { TODO; }
    String::data const* head(int) const override final { TODO; }
    String::data const* tail(int) const override final { TODO; }
    String::data const* prepend(String::data const*) const override final { TODO; }
    String::data const* stretch(int) const override final { TODO; }

    void get(char*, size_t) const noexcept override final { TODO; }
    Char_t at(int) const noexcept override final { TODO; }
    size_t size() const noexcept override final { TODO; }
    size_t size(int) const noexcept override final { TODO; }
    int length() const noexcept override final { TODO; }
    int length(size_t) const noexcept override final { TODO; }

    bool connects(char const*) const noexcept override final { TODO; }
    bool isASCII() const noexcept override final { TODO; }
    char const* buffer() const noexcept override final { TODO; }
    char const* extent() const noexcept override final { TODO; }
    char const* origin() const noexcept override final { TODO; }
    int depth() const noexcept override final { TODO; }

    String::data const* const pHead;
    String::data const* const pTail;
};

/***********************************************************************************************************************
*** Repeat
***********************************************************************************************************************/

struct Repeat final : public String::data, private ObjectGuard<Repeat>
{
private:
    String::data const* append(String::data const*) const override final { TODO; }
    String::data const* head(int) const override final { TODO; }
    String::data const* tail(int) const override final { TODO; }
    String::data const* prepend(String::data const*) const override final { TODO; }
    String::data const* stretch(int) const override final { TODO; }

    void get(char*, size_t) const noexcept override final { TODO; }
    Char_t at(int) const noexcept override final { TODO; }
    size_t size() const noexcept override final { TODO; }
    size_t size(int) const noexcept override final { TODO; }
    int length() const noexcept override final { TODO; }
    int length(size_t) const noexcept override final { TODO; }

    bool connects(char const*) const noexcept override final { TODO; }
    bool isASCII() const noexcept override final { TODO; }
    char const* buffer() const noexcept override final { TODO; }
    char const* extent() const noexcept override final { TODO; }
    char const* origin() const noexcept override final { TODO; }
    int depth() const noexcept override final { TODO; }

    Char_t c;
    int n;
};

/***********************************************************************************************************************
*** create()
***********************************************************************************************************************/

String::data const* String::data::create(char const* p)
{
    assert(p && *p);

    auto n = strlen(p);
    return new(n) Buffer(p, n);
}

String::data const* String::data::create(Char_t c, int n)
{
    TODO;
}

/***********************************************************************************************************************
*** evaluate()
***********************************************************************************************************************/

char const* String::data::evaluate(String::data const*& pData)
{
    assert(pData);

    auto result = pData->buffer();

    if (!result)
    {
        auto n = pData->size();
        auto p = new(n) Buffer(pData, n);

        Erase(pData);
        pData = p;
        result = pData->buffer();
    }

    return result;
}

/***********************************************************************************************************************
*** String
***********************************************************************************************************************/

String::String() : pData(empty)
{
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

String::String(char const* p) : pData((p) && (*p) ? String::data::create(p) : empty)
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
    return String::data::evaluate(pData);
}

void String::Get(char* p, size_t n) const
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

/***********************************************************************************************************************
*
* TODO's:
*
*  - Literals
*  - Translations
*
***********************************************************************************************************************/
