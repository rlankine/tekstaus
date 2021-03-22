

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
// #define PROFILER

#include <assert.h>
#include <string>

//**********************************************************************************************************************

#pragma intrinsic(memcmp, memcpy, memset, strcmp, strlen)
#define noexcept

#define BUFFER_LIMIT 1
#define STACK_LIMIT 10000

/***********************************************************************************************************************
*** String::data
***********************************************************************************************************************/

struct String::data : public Shared
{
    static String::data const* create(char const*);
    static String::data const* create(Char_t, int);

    virtual String::data const* append(String::data const*) const;
    virtual String::data const* head(int) const;
    virtual String::data const* tail(int) const;
    virtual String::data const* prepend(String::data const*) const;
    virtual String::data const* stretch(int) const;

    virtual void get(char*, size_t) const noexcept = 0;
    virtual Char_t at(int) const noexcept = 0;
    virtual size_t size() const noexcept = 0;
    virtual size_t size(int) const noexcept = 0;
    virtual int length() const noexcept = 0;
    virtual int length(size_t) const noexcept = 0;

    virtual bool isASCII() const noexcept;
    virtual char const* buffer() const noexcept;
    virtual char const* extent() const noexcept = 0;
    virtual char const* origin() const noexcept = 0;
    virtual int depth() const noexcept;

    static char const* evaluate(String::data const*&);

private:
    void* operator new(size_t n) { return ::operator new(n); }
};

//**********************************************************************************************************************

static struct : public String::data
{
private:
    String::data const* append(String::data const* p) const override final { return Clone(p); }
    String::data const* head(int) const override final { return Clone(this); }
    String::data const* tail(int) const override final { return Clone(this); }
    String::data const* prepend(String::data const* p) const override final { return Clone(p); }
    String::data const* stretch(int) const override final { UNREACHABLE; return Clone(this); }

    void get(char* p, size_t n) const noexcept override final { assert(p && n); memset(p, '\0', n); }
    Char_t at(int) const noexcept override final { return 0; }
    size_t size() const noexcept override final { return 0; }
    size_t size(int) const noexcept override final { return 0; }
    int length() const noexcept override final { return 0; }
    int length(size_t) const noexcept override final { return 0; }

    bool isASCII() const noexcept override final { return true; }
    char const* buffer() const noexcept override final { return &cData; }
    char const* extent() const noexcept override final { return &cData; }
    char const* origin() const noexcept override final { return &cData; }
    int depth() const noexcept override final { return 0; }

    char const cData = '\0';
} const empty;

/***********************************************************************************************************************
*** StringBuffer
***********************************************************************************************************************/

struct StringBuffer final : public String::data, private ObjectGuard<StringBuffer>
{
    StringBuffer(char const* p, size_t n) : nSize(n), nLength(n)
    {
        assert(p && n);
        memcpy(cBuffer, p, n);
        cBuffer[nSize] = '\0';
    }

    StringBuffer(String::data const* p, size_t n) : nSize(n), nLength(p->length())
    {
        assert(p && n && n == p->size());
        p->get(cBuffer, nSize);
        cBuffer[nSize] = '\0';
    }

    StringBuffer(String::data const* pHead, size_t nHead, String::data const* pTail, size_t nTail) : nSize(nHead + nTail), nLength(pHead->length() + pTail->length())
    {
        assert(pHead && pTail && nHead && nTail && nHead == pHead->size() && nTail == pTail->size());
        pHead->get(cBuffer, nHead);
        pTail->get(cBuffer + nHead, nTail);
        cBuffer[nSize] = '\0';
    }

private:
    ~StringBuffer() { }

    void* operator new(size_t) = delete;
    void* operator new[](size_t) = delete;

    friend String::data const* String::data::create(char const*);                 // For initial creation of string objects
    friend char const* String::data::evaluate(String::data const*&);              // For converting a string expression into a contiguous buffer
    friend String::data const* String::data::prepend(String::data const*) const;  // For string expression complexity reduction and small string optimization

    void* operator new(size_t n, size_t x, bool = false) { return ::operator new(n + x); }
    void operator delete(void* p, size_t, bool) { ::operator delete(p); }

    void get(char*, size_t) const noexcept override final;
    Char_t at(int) const noexcept override final { PROFILER; TODO; }
    size_t size() const noexcept override final;
    size_t size(int) const noexcept override final { PROFILER; TODO; }
    int length() const noexcept override final;
    int length(size_t) const noexcept override final { PROFILER; TODO; }

    bool isASCII() const noexcept override final { PROFILER; TODO; }
    char const* buffer() const noexcept override final;
    char const* extent() const noexcept override final;
    char const* origin() const noexcept override final;
    // int depth() const noexcept override final { PROFILER; TODO; }

    size_t const nSize;
    mutable int nLength;
    char cBuffer[1];  // <---- This must be the last data item!
};

/***********************************************************************************************************************
*** StringTail
***********************************************************************************************************************/

struct StringTail final : public String::data, private ObjectGuard<StringTail>
{
    StringTail(String::data const* p, int n) : pString(p), nReduceLength(n), nReduceSize(n)
    {
        PROFILER;

        assert(p && n && n < p->length());
        assert(dynamic_cast<StringBuffer const*>(p));
    }

private:
    ~StringTail()
    {
        Erase(pString);
    }

    // String::data const* append(String::data const*) const override final { PROFILER; TODO; }
    // String::data const* head(int) const override final { PROFILER; TODO; }
    String::data const* tail(int) const override final;
    // String::data const* prepend(String::data const*) const override final { PROFILER; TODO; }
    String::data const* stretch(int) const override final;

    void get(char*, size_t) const noexcept override final;
    Char_t at(int) const noexcept override final { PROFILER; TODO; }
    size_t size() const noexcept override final;
    size_t size(int) const noexcept override final { PROFILER; TODO; }
    int length() const noexcept override final;
    int length(size_t) const noexcept override final { PROFILER; TODO; }

    bool isASCII() const noexcept override final { PROFILER; TODO; }
    char const* buffer() const noexcept override final;
    char const* extent() const noexcept override final;
    char const* origin() const noexcept override final;
    int depth() const noexcept override final;

    String::data const* const pString;
    int nReduceLength;
    size_t nReduceSize;
};

/***********************************************************************************************************************
*** StringHead
***********************************************************************************************************************/

struct StringHead final : public String::data, private ObjectGuard<StringHead>
{
    StringHead(String::data const* p, int n) noexcept : pString(p), nLength(n), nSize(n)
    {
        PROFILER;

        assert(p && n > 0 && n < p->length());
        assert(dynamic_cast<StringBuffer const*>(p) || dynamic_cast<StringTail const*>(p));
    }

private:
    ~StringHead()
    {
        Erase(pString);
    }

    // String::data const* append(String::data const*) const override final { PROFILER; TODO; }
    String::data const* head(int) const override final;
    String::data const* tail(int) const override final;
    // String::data const* prepend(String::data const*) const override final { PROFILER; TODO; }
    String::data const* stretch(int) const override final;

    void get(char*, size_t) const noexcept override final;
    Char_t at(int) const noexcept override final { PROFILER; TODO; }
    size_t size() const noexcept override final;
    size_t size(int) const noexcept override final { PROFILER; TODO; }
    int length() const noexcept override final;
    int length(size_t) const noexcept override final { PROFILER; TODO; }

    bool isASCII() const noexcept override final { PROFILER; TODO; }
    char const* buffer() const noexcept override final { PROFILER; TODO; }
    char const* extent() const noexcept override final;
    char const* origin() const noexcept override final;
    int depth() const noexcept override final;

    String::data const* const pString;
    int nLength;
    size_t nSize;
};

/***********************************************************************************************************************
*** StringAppend
***********************************************************************************************************************/

struct StringAppend final : public String::data, private ObjectGuard<StringAppend>
{
    StringAppend(String::data const* pPrefix, String::data const* pSuffix) noexcept :
        pHead(pPrefix),
        pTail(pSuffix),
        nHead(pHead->size())
    {
        PROFILER;

        assert(!!pPrefix && pPrefix->length() > 0 && !!pSuffix && pSuffix->length() > 0);
    }

private:
    ~StringAppend()
    {
        Erase(pHead);
        Erase(pTail);
    }

    String::data const* append(String::data const*) const override final;
    String::data const* head(int) const override final;
    String::data const* tail(int) const override final;
    String::data const* prepend(String::data const*) const override final;
    String::data const* stretch(int) const override final;

    void get(char*, size_t) const noexcept override final;
    Char_t at(int) const noexcept override final { PROFILER; TODO; }
    size_t size() const noexcept override final;
    size_t size(int) const noexcept override final { PROFILER; TODO; }
    int length() const noexcept override final;
    int length(size_t n) const noexcept override final { PROFILER; return n; TODO; /* !!!! */ }

    bool isASCII() const noexcept override final { PROFILER; TODO; }
    // char const* buffer() const noexcept override final { PROFILER; TODO; }
    char const* extent() const noexcept override final;
    char const* origin() const noexcept override final;
    int depth() const noexcept override final;

    String::data const* const pHead;
    String::data const* const pTail;
    size_t const nHead;
};

/***********************************************************************************************************************
*** StringRepeat
***********************************************************************************************************************/

struct StringRepeat final : public String::data, private ObjectGuard<StringRepeat>
{
    StringRepeat(Char_t c, int n) : cData(c), nSize(n), nLength(n)
    {
        PROFILER;

        assert(c && n);
    }

private:
    String::data const* append(String::data const*) const override final;
    String::data const* head(int) const override final;
    String::data const* tail(int) const override final;
    // String::data const* prepend(String::data const*) const override final { PROFILER; TODO; }
    String::data const* stretch(int) const override final;

    void get(char*, size_t) const noexcept override final;
    Char_t at(int) const noexcept override final { PROFILER; TODO; }
    size_t size() const noexcept override final;
    size_t size(int) const noexcept override final { PROFILER; TODO; }
    int length() const noexcept override final;
    int length(size_t) const noexcept override final { PROFILER; TODO; }

    bool isASCII() const noexcept override final { PROFILER; TODO; }
    char const* extent() const noexcept override final;
    char const* origin() const noexcept override final;
    int depth() const noexcept override final;

    char const* const cookie = nullptr;

    Char_t const cData;
    size_t nSize;
    int const nLength;
};

/***********************************************************************************************************************
*** create()
***********************************************************************************************************************/

String::data const* String::data::create(char const* p)
{
    PROFILER;

    assert(p && *p);
    auto n = strlen(p);
    return new(n) StringBuffer(p, n);
}

String::data const* String::data::create(Char_t c, int n)
{
    PROFILER;

    assert(c && n);
    return new StringRepeat(c, n);
}

/***********************************************************************************************************************
*** append()
***********************************************************************************************************************/

String::data const* String::data::append(String::data const* p) const
{
    PROFILER;

    assert(p);
    return p->prepend(this);
}

String::data const* StringAppend::append(String::data const* p) const
{
    PROFILER;

    if (pHead->depth() > std::max(p->depth(), pTail->depth()) || p->origin() == extent())
    {
        auto step0 = pTail->append(p);
        auto step1 = pHead->append(step0);

        Erase(step0);

        return step1;
    }

    return p->prepend(this);
}

String::data const* StringRepeat::append(String::data const* p) const
{
    PROFILER;

    return p->origin() == extent() ? p->stretch(length()) : p->prepend(this);
}

/***********************************************************************************************************************
*** head()
***********************************************************************************************************************/

String::data const* String::data::head(int n) const
{
    PROFILER;

    assert(n > 0 && n < length());
    return new StringHead(Clone(this), n);
}

String::data const* StringHead::head(int n) const
{
    PROFILER;

    assert(n > 0 && n < length());
    return pString->head(n);
}

String::data const* StringAppend::head(int n) const
{
    PROFILER;

    assert(n > 0 && n < length());

    if (n > pHead->length())
    {
        auto step0 = pTail->head(n - pHead->length());
        auto step1 = pHead->append(step0);

        Erase(step0);

        return step1;
    }

    if (n < pHead->length())
    {
        return pHead->head(n);
    }

    return Clone(pHead);
}

String::data const* StringRepeat::head(int n) const
{
    PROFILER;

    assert(n > 0 && n < length());
    return create(cData, n);
}

/***********************************************************************************************************************
*** tail()
***********************************************************************************************************************/

String::data const* String::data::tail(int n) const
{
    PROFILER;

    assert(n > 0 && n < length());
    return new StringTail(Clone(this), n);
}

String::data const* StringTail::tail(int n) const
{
    PROFILER;

    assert(n > 0 && n < length());
    return pString->tail(n + nReduceLength);
}

String::data const* StringHead::tail(int n) const
{
    PROFILER;

    assert(n > 0 && n < length());

    auto step0 = pString->tail(n);
    auto step1 = step0->head(nLength - n);

    Erase(step0);

    return step1;
}

String::data const* StringAppend::tail(int n) const
{
    PROFILER;

    assert(n > 0 && n < length());

    if (n < pHead->length())
    {
        auto step0 = pHead->tail(n);
        auto step1 = step0->append(pTail);

        Erase(step0);

        return step1;
    }

    if (n > pHead->length())
    {
        return pTail->tail(n - pHead->length());
    }

    return Clone(pTail);
}

String::data const* StringRepeat::tail(int n) const
{
    PROFILER;

    assert(n > 0 && n < length());
    return create(cData, nLength - n);
}

/***********************************************************************************************************************
*** prepend()
***********************************************************************************************************************/

String::data const* String::data::prepend(String::data const* p) const
{
    PROFILER;

    assert(p);

    auto nHead = p->size();
    auto nTail = size();

    if (nHead + nTail > BUFFER_LIMIT && std::max(p->depth(), depth()) < STACK_LIMIT)
    {
        return new StringAppend(Clone(p), Clone(this));
    }

    return new(nHead + nTail) StringBuffer(p, nHead, this, nTail);
}

String::data const* StringAppend::prepend(String::data const* p) const
{
    PROFILER;

    if (pTail->depth() > std::max(p->depth(), pHead->depth()))
    {
        auto step0 = p->append(pHead);
        auto step1 = step0->append(pTail);

        Erase(step0);

        return step1;
    }

    return String::data::prepend(p);
}

/***********************************************************************************************************************
*** stretch()
***********************************************************************************************************************/

String::data const* String::data::stretch(int) const
{
    PROFILER;

    UNREACHABLE;

    return Clone(this);
}

String::data const* StringTail::stretch(int n) const
{
    PROFILER;

    assert(n > 0 && n <= pString->length());

    return pString->tail(nReduceLength - n);
}

String::data const* StringHead::stretch(int n) const
{
    PROFILER;

    assert(n > 0);

    auto step0 = pString->stretch(n);
    auto step1 = step0->head(nLength + n);

    Erase(step0);

    return step1;
}

String::data const* StringAppend::stretch(int n) const
{
    PROFILER;

    assert(n > 0);

    auto step0 = pHead->stretch(n);
    auto step1 = step0->append(pTail);

    Erase(step0);

    return step1;
}

String::data const* StringRepeat::stretch(int n) const
{
    PROFILER;

    assert(n > 0);

    return create(cData, nLength + n);
}

/***********************************************************************************************************************
*** get()
***********************************************************************************************************************/

void StringBuffer::get(char* p, size_t n) const noexcept
{
    PROFILER;

    assert(p && n);

    if (n > nSize)
    {
        memcpy(p, cBuffer, nSize);
        memset(p + nSize, '\0', n - nSize);
    }
    else
    {
        memcpy(p, cBuffer, n);
    }
}

void StringTail::get(char* p, size_t n) const noexcept
{
    PROFILER;

    assert(p && n);

    auto pBuffer = pString->buffer() + nReduceSize;
    auto nSize = pString->size() - nReduceSize;

    if (n > nSize)
    {
        memcpy(p, pBuffer, nSize);
        memset(p + nSize, 0, n - nSize);
    }
    else
    {
        memcpy(p, pBuffer, n);
    }
}

void StringHead::get(char* p, size_t n) const noexcept
{
    PROFILER;

    assert(p && n);

    if (n > nSize)
    {
        pString->get(p, nSize);
        memset(p + nSize, '\0', n - nSize);
    }
    else
    {
        pString->get(p, n);
    }
}

void StringAppend::get(char* p, size_t n) const noexcept
{
    PROFILER;

    assert(p && n);

    if (n > nHead)
    {
        pHead->get(p, nHead);
        pTail->get(p + nHead, n - nHead);
    }
    else
    {
        pHead->get(p, n);
    }
}

void StringRepeat::get(char* p, size_t n) const noexcept
{
    PROFILER;

    assert(p && n);

    // TODO: Fix!!!! To support multibytes as well!!!!

    if (n > nSize)
    {
        memset(p, cData, nSize);
        memset(p + nSize, '\0', n - nSize);
    }
    else
    {
        memset(p, cData, n);
    }
}

/***********************************************************************************************************************
*** at()
***********************************************************************************************************************/

/***********************************************************************************************************************
*** size()
***********************************************************************************************************************/

size_t StringBuffer::size() const noexcept
{
    PROFILER;

    return nSize;
}

size_t StringTail::size() const noexcept
{
    PROFILER;

    return pString->size() - nReduceSize;
}

size_t StringHead::size() const noexcept
{
    PROFILER;

    return nSize;
}

size_t StringAppend::size() const noexcept
{
    PROFILER;

    return pHead->size() + pTail->size();
}

size_t StringRepeat::size() const noexcept
{
    PROFILER;

    return nSize;
}

/***********************************************************************************************************************
*** size(int)
***********************************************************************************************************************/

/***********************************************************************************************************************
*** length()
***********************************************************************************************************************/

int StringBuffer::length() const noexcept
{
    PROFILER;

    return nLength;
}

int StringTail::length() const noexcept
{
    PROFILER;

    return pString->length() - nReduceLength;
}

int StringHead::length() const noexcept
{
    PROFILER;

    return nLength;
}

int StringAppend::length() const noexcept
{
    PROFILER;

    return pHead->length() + pTail->length();
}

int StringRepeat::length() const noexcept
{
    PROFILER;

    return nLength;
}

/***********************************************************************************************************************
*** length(size_t)
***********************************************************************************************************************/

/***********************************************************************************************************************
*** isASCII()
***********************************************************************************************************************/

bool String::data::isASCII() const noexcept
{
    PROFILER;

    return length() == size();
}

/***********************************************************************************************************************
*** buffer()
***********************************************************************************************************************/

char const* String::data::buffer() const noexcept
{
    PROFILER;

    return nullptr;
}

char const* StringBuffer::buffer() const noexcept
{
    PROFILER;

    return cBuffer;
}

char const* StringTail::buffer() const noexcept
{
    PROFILER;

    return pString->buffer() + nReduceSize;
}

/***********************************************************************************************************************
*** extent()
***********************************************************************************************************************/

char const* StringBuffer::extent() const noexcept
{
    PROFILER;

    return cBuffer + nSize;
}

char const* StringTail::extent() const noexcept
{
    PROFILER;

    return pString->extent();
}

char const* StringHead::extent() const noexcept
{
    PROFILER;

    return pString->origin() + nSize;
}

char const* StringAppend::extent() const noexcept
{
    PROFILER;

    return pTail->extent();
}

char const* StringRepeat::extent() const noexcept
{
    PROFILER;

    return cookie - cData;
}

/***********************************************************************************************************************
*** origin()
***********************************************************************************************************************/

char const* StringBuffer::origin() const noexcept
{
    PROFILER;

    return cBuffer;
}

char const* StringTail::origin() const noexcept
{
    PROFILER;

    return pString->origin() + nReduceSize;
}

char const* StringHead::origin() const noexcept
{
    PROFILER;

    return pString->origin();
}

char const* StringAppend::origin() const noexcept
{
    PROFILER;

    return pHead->origin();
}

char const* StringRepeat::origin() const noexcept
{
    PROFILER;

    return cookie - cData;
}

/***********************************************************************************************************************
*** depth()
***********************************************************************************************************************/

int String::data::depth() const noexcept
{
    PROFILER;

    return 1;
}

int StringTail::depth() const noexcept
{
    PROFILER;

    return 2;
}

int StringHead::depth() const noexcept
{
    PROFILER;

    return pString->depth() + 1;
}

int StringAppend::depth() const noexcept
{
    PROFILER;

    return std::max(pHead->depth(), pTail->depth()) + 1;
}

int StringRepeat::depth() const noexcept
{
    PROFILER;

    return 1;
}

/***********************************************************************************************************************
*** evaluate()
***********************************************************************************************************************/

char const* String::data::evaluate(String::data const*& p)
{
    PROFILER;

    assert(p);

    auto result = p->buffer();

    if (!result)
    {
        auto nSize = p->size();
        auto pString = new(nSize) StringBuffer(p, nSize);

        Erase(p);
        p = pString;
        result = p->buffer();
    }

    return result;
}

/***********************************************************************************************************************
*** String
***********************************************************************************************************************/

String::String() : pData(Shared::Clone(empty))
{
}

String::String(String const& r) : pData(Shared::Clone(r.pData))
{
}

String::String(String const& r, String const& s) : pData(r.pData->append(s.pData))
{
}

String::String(String const& r, int n) : pData(n < 1 ? Shared::Clone(empty) : n < r.pData->length() ? r.pData->head(n) : Shared::Clone(r.pData))
{
}

String::String(int n, String const& r) : pData(n < 1 ? Shared::Clone(r.pData) : n < r.pData->length() ? r.pData->tail(n) : Shared::Clone(empty))
{
}

String::String(char const* p) : pData(p != nullptr && *p != '\0' ? String::data::create(p) : Shared::Clone(empty))
{
}

String::String(Char_t c, int n) : pData(c&& n ? String::data::create(c, n) : Shared::Clone(empty))
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
