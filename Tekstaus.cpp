
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
#define VERBOSE
#include "Tools.h"
#if defined(PROFILER)
#undef PROFILER
#endif
#define PROFILER

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
    static String::data const* create();
    static String::data const* create(char const*);
    static String::data const* create(Char_t, int);

    virtual String::data const* append(String::data const*) const = 0;
    virtual String::data const* head(int) const = 0;
    virtual String::data const* tail(int) const = 0;
    virtual String::data const* prepend(String::data const*) const = 0;
    virtual String::data const* stretch(int) const = 0;

    virtual void get(char*, size_t) const noexcept = 0;
    virtual Char_t at(int) const noexcept = 0;
    virtual size_t size() const noexcept = 0;
    virtual size_t size(int) const noexcept = 0;
    virtual int length() const noexcept = 0;
    virtual int length(size_t) const noexcept = 0;

    virtual bool isASCII() const noexcept = 0;
    virtual char const* buffer() const noexcept = 0;
    virtual char const* extent() const noexcept = 0;
    virtual char const* origin() const noexcept = 0;
    virtual int depth() const noexcept = 0;

    static char const* evaluate(String::data const*&);

private:
    void* operator new(size_t n) { return ::operator new(n); }
    void* operator new[](size_t) = delete;
};

//----------------------------------------------------------------------------------------------------------------------

String::data const* String::data::create()
{
    static struct : public String::data
    {
    private:
        String::data const* append(String::data const* p) const override final { assert(p); return Clone(p); }
        String::data const* head(int) const override final { return Clone(this); }
        String::data const* tail(int) const override final { return Clone(this); }
        String::data const* prepend(String::data const* p) const override final { assert(p); return Clone(p); }
        String::data const* stretch(int n) const override final { if (n) FAIL("An attempt was made to stretch an unstretchable string data object."); else return Clone(this); }

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
    } const instance;

    return Clone(instance);
}

String::data const* String::data::append(String::data const* p) const
{
    assert(p);
    return p->prepend(this);
}

String::data const* String::data::stretch(int n) const
{
    if (n) FAIL("An attempt was made to stretch an unstretchable string data object."); else return Clone(this);
}

/***********************************************************************************************************************
*** StringBuffer
***********************************************************************************************************************/

struct StringBuffer final : public String::data, private ObjectGuard<StringBuffer>
{
    StringBuffer(char const*, size_t);
    StringBuffer(String::data const*, size_t);
    StringBuffer(String::data const*, size_t, String::data const*, size_t);

    void* operator new(size_t n, size_t k, bool = false) { return ::operator new(n + k); }
    void operator delete(void* p, size_t, bool) { ::operator delete(p); }

private:
    ~StringBuffer() { }

    void* operator new(size_t) = delete;

    String::data const* append(String::data const* p) const override final { return p->prepend(this); }
    String::data const* head(int n) const override final { return String::data::head(n); }
    String::data const* tail(int n) const override final { return String::data::tail(n); }
    String::data const* prepend(String::data const* p) const override final { return String::data::prepend(p); }
    String::data const* stretch(int n) const override final { return String::data::stretch(n); }

    void get(char*, size_t) const noexcept override final;
    Char_t at(int) const noexcept override final;
    size_t size() const noexcept override final { return nSize; }
    size_t size(int) const noexcept override final;
    int length() const noexcept override final { return nLength; }
    int length(size_t) const noexcept override final;

    bool isASCII() const noexcept override final { return nLength == nSize; }
    char const* buffer() const noexcept override final { return cBuffer; }
    char const* extent() const noexcept override final { return cBuffer + nSize; }
    char const* origin() const noexcept override final { return cBuffer; }
    int depth() const noexcept override final { return 1; }

    size_t const nSize;
    int nLength;
    char cBuffer[1];  // <---- This must be the last data item!
};

//----------------------------------------------------------------------------------------------------------------------

StringBuffer::StringBuffer(char const* p, size_t n) : nSize(n), nLength(int(n))
{
    assert(p && n);
    memcpy(cBuffer, p, n);
    cBuffer[n] = '\0';
}

StringBuffer::StringBuffer(String::data const* p, size_t n) : nSize(n), nLength(p->length())
{
    assert(p && n && n == p->size());
    p->get(cBuffer, n);
    cBuffer[n] = '\0';
}

StringBuffer::StringBuffer(String::data const* p, size_t n, String::data const* q, size_t k) : nSize(n + k), nLength(p->length() + q->length())
{
    assert(p && n && n == p->size() && q && k && k == p->size());
    p->get(cBuffer, n);
    q->get(cBuffer + n, k);
    cBuffer[nSize] = '\0';
}

String::data const* String::data::create(char const* p)
{
    assert(p && *p);
    auto n = strlen(p);
    return new(n) StringBuffer(p, n);
}

/***********************************************************************************************************************
*** StringTail
***********************************************************************************************************************/

struct StringTail final : public String::data, private ObjectGuard<StringTail>
{
    StringTail(String::data const* p, int n) : pString(p), nCursor(n), nOffset(n)
    {
        assert(p && n > 0 && n < p->length());
        assert(dynamic_cast<StringBuffer const*>(p));
    }

private:
    ~StringTail() { Erase(pString); }

    String::data const* append(String::data const* p) const override final { return p->prepend(this); }
    String::data const* head(int n) const override final { return n < length() ? String::data::head(n) : Clone(this); }
    String::data const* tail(int n) const override final { return n > 0 ? pString->tail(nCursor + n) : Clone(this); }
    String::data const* prepend(String::data const* p) const override final { return String::data::prepend(p); }
    String::data const* stretch(int n) const override final { assert(n > 0 && n <= nCursor); return pString->tail(nCursor - n); }

    void get(char*, size_t) const noexcept override final;
    Char_t at(int n) const noexcept override final { return n < 0 ? 0 : pString->at(nCursor + n); }
    size_t size() const noexcept override final { return pString->size() - nOffset; }
    size_t size(int) const noexcept override final;
    int length() const noexcept override final { return pString->length() - nCursor; }
    int length(size_t) const noexcept override final;

    bool isASCII() const noexcept override final { return length() == size(); }
    char const* buffer() const noexcept override final { return pString->buffer() + nOffset; }
    char const* extent() const noexcept override final { return pString->extent(); }
    char const* origin() const noexcept override final { return pString->origin() + nOffset; }
    int depth() const noexcept override final { return 2; }

    String::data const* const pString;
    int nCursor;
    size_t nOffset;
};

//----------------------------------------------------------------------------------------------------------------------

String::data const* String::data::tail(int n) const
{
    assert(n > 0 && n < length());
    return new StringTail(Clone(this), n);
}

/***********************************************************************************************************************
*** StringHead
***********************************************************************************************************************/

struct StringHead final : public String::data, private ObjectGuard<StringHead>
{
    StringHead(String::data const* p, int n) noexcept : pString(p), nCursor(n), nSize(n)
    {
        assert(p && n > 0 && n < p->length());
        assert(dynamic_cast<StringBuffer const*>(p) || dynamic_cast<StringTail const*>(p));
    }

private:
    ~StringHead() { Erase(pString); }

    String::data const* append(String::data const* p) const override final { assert(p); return p->origin() == extent() ? p->stretch(nCursor) : p->prepend(this); }
    String::data const* head(int n) const override final { return n < nCursor ? pString->head(n) : Clone(this); }
    String::data const* tail(int) const override final;
    String::data const* prepend(String::data const* p) const override final { return String::data::prepend(p); }
    String::data const* stretch(int n) const override final { assert(n > 0); auto step0 = pString->stretch(n); auto step1 = step0->head(nCursor + n); Erase(step0); return step1; }

    void get(char*, size_t) const noexcept override final;
    Char_t at(int n) const noexcept override final { return n < nCursor ? pString->at(n) : 0; }
    size_t size() const noexcept override final { return nSize; }
    size_t size(int) const noexcept override final;
    int length() const noexcept override final { return nCursor; }
    int length(size_t) const noexcept override final;

    bool isASCII() const noexcept override final { return nCursor == nSize; }
    char const* buffer() const noexcept override final { return nullptr; }
    char const* extent() const noexcept override final { return pString->origin() + nSize; }
    char const* origin() const noexcept override final { return pString->origin(); }
    int depth() const noexcept override final { return pString->depth() + 1; }

    String::data const* const pString;
    int nCursor;
    size_t nSize;
};

//----------------------------------------------------------------------------------------------------------------------

String::data const* String::data::head(int n) const
{
    assert(n > 0 && n < length());
    return new StringHead(Clone(this), n);
}

/***********************************************************************************************************************
*** StringAppend
***********************************************************************************************************************/

struct StringAppend final : public String::data, private ObjectGuard<StringAppend>
{
    StringAppend(String::data const* p, String::data const* q) noexcept : pHead(p), pTail(q), nCursor(pHead->length()), nHead(pHead->size())
    {
        assert(!!p && p->length() > 0 && !!q && q->length() > 0);
    }

private:
    ~StringAppend() { Erase(pHead); Erase(pTail); }

    String::data const* append(String::data const*) const override final;
    String::data const* head(int n) const override final { if (n > nCursor) { auto step0 = pTail->head(n - nCursor); auto step1 = pHead->append(step0); Erase(step0); return step1; } return pHead->head(n); }
    String::data const* tail(int) const override final;
    String::data const* prepend(String::data const*) const override final;
    String::data const* stretch(int n) const override final { auto step0 = pHead->stretch(n); auto step1 = step0->append(pTail); Erase(step0); return step1; }

    void get(char*, size_t) const noexcept override final;
    Char_t at(int n) const noexcept override final { return n < nCursor ? pHead->at(n) : pTail->at(n - nCursor); }
    size_t size() const noexcept override final { return pHead->size() + pTail->size(); }
    size_t size(int) const noexcept override final;
    int length() const noexcept override final { return nCursor + pTail->length(); }
    int length(size_t n) const noexcept override final;

    bool isASCII() const noexcept override final { return length() == size(); }
    char const* buffer() const noexcept override final { return nullptr; }
    char const* extent() const noexcept override final { return pTail->extent(); }
    char const* origin() const noexcept override final { return pHead->origin(); }
    int depth() const noexcept override final { return std::max(pHead->depth(), pTail->depth()) + 1; }

    String::data const* const pHead;
    String::data const* const pTail;
    int nCursor;
    size_t const nHead;
};

//----------------------------------------------------------------------------------------------------------------------

String::data const* StringAppend::append(String::data const* p) const
{
    if (pHead->depth() > std::max(p->depth(), pTail->depth()))
    {
        auto step0 = pTail->append(p);
        auto step1 = pHead->append(step0);

        Erase(step0);

        return step1;
    }

    return p->prepend(this);
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

String::data const* String::data::prepend(String::data const* p) const
{
    assert(p);

    auto n = p->size();
    auto k = size();

    if (n + k > BUFFER_LIMIT && std::max(p->depth(), depth()) < STACK_LIMIT)
    {
        return new StringAppend(Clone(p), Clone(this));
    }

    return new(n + k) StringBuffer(p, n, this, k);
}

/***********************************************************************************************************************
*** StringRepeat
***********************************************************************************************************************/

struct StringRepeat final : public String::data, private ObjectGuard<StringRepeat>
{
    StringRepeat(Char_t c, int n) : cData(c), nSize(n), nLength(n)
    {
        assert(c && n);
    }

private:
    String::data const* append(String::data const* p) const override final { assert(p); return p->origin() == extent() ? p->stretch(length()) : p->prepend(this); }
    String::data const* head(int n) const override final { assert(n > 0); return n < nLength ? create(cData, n) : Clone(this); }
    String::data const* tail(int n) const override final { return create(cData, nLength - n); }
    String::data const* prepend(String::data const* p) const override final { return String::data::prepend(p); }
    String::data const* stretch(int n) const override final { assert(n > 0); return create(cData, nLength + n); }

    void get(char*, size_t) const noexcept override final;
    Char_t at(int n) const noexcept override final { return n < 0 ? 0 : n < nLength ? cData : 0; }
    size_t size() const noexcept override final { return nSize; }
    size_t size(int) const noexcept override final;
    int length() const noexcept override final { return nLength; }
    int length(size_t) const noexcept override final;

    bool isASCII() const noexcept override final { return !(cData & 0xFFFFFF80); }
    char const* buffer() const noexcept override final { return nullptr; }
    char const* extent() const noexcept override final { return cookie - cData; }
    char const* origin() const noexcept override final { return cookie - cData; }
    int depth() const noexcept override final { return 1; }

    char const* const cookie = nullptr;

    Char_t const cData;
    size_t nSize;
    int const nLength;
};

//----------------------------------------------------------------------------------------------------------------------

String::data const* String::data::create(Char_t c, int n)
{
    assert(c && n);
    return new StringRepeat(c, n);
}

/***********************************************************************************************************************
*** tail()
***********************************************************************************************************************/

String::data const* StringHead::tail(int n) const
{
    auto step0 = pString->tail(n);
    auto step1 = step0->head(nCursor - n);

    Erase(step0);

    return step1;
}

String::data const* StringAppend::tail(int n) const
{
    if (n < nCursor)
    {
        auto step0 = pHead->tail(n);
        auto step1 = step0->append(pTail);

        Erase(step0);

        return step1;
    }

    if (n > nCursor)
    {
        return pTail->tail(n - nCursor);
    }

    return Clone(pTail);
}

/***********************************************************************************************************************
*** get()
***********************************************************************************************************************/

void StringBuffer::get(char* p, size_t n) const noexcept
{
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
    assert(p && n);

    auto pBuffer = buffer();
    auto nSize = pString->size() - nOffset;

    if (n > nSize)
    {
        memcpy(p, pBuffer, nSize);
        memset(p + nSize, '\0', n - nSize);
    }
    else
    {
        memcpy(p, pBuffer, n);
    }
}

void StringHead::get(char* p, size_t n) const noexcept
{
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

Char_t StringBuffer::at(int n) const noexcept
{
    TODO;
}

/***********************************************************************************************************************
*** size(int)
***********************************************************************************************************************/

size_t StringBuffer::size(int n) const noexcept
{
    assert(n > 0 && n < nLength);

    if (nLength == nSize) return n;

    PROFILER; TODO;
}

size_t StringTail::size(int) const noexcept
{
    PROFILER; TODO;
}

size_t StringHead::size(int) const noexcept
{
    PROFILER; TODO;
}

size_t StringAppend::size(int n) const noexcept
{
    PROFILER; TODO;
    return n < nCursor ? pHead->size(n) : pHead->size() + pTail->size(n - nCursor);
}

size_t StringRepeat::size(int) const noexcept
{
    PROFILER; TODO;
}

/***********************************************************************************************************************
*** length(size_t)
***********************************************************************************************************************/

int StringBuffer::length(size_t) const noexcept
{
    PROFILER; TODO;
}

int StringTail::length(size_t) const noexcept
{
    PROFILER; TODO;
}

int StringHead::length(size_t) const noexcept
{
    PROFILER; TODO;
}

int StringAppend::length(size_t) const noexcept
{
    PROFILER; TODO;
}

int StringRepeat::length(size_t) const noexcept
{
    PROFILER; TODO;
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

String::String() : pData(String::data::create())
{
}

String::String(String const& r) : pData(Shared::Clone(r.pData))
{
}

String::String(String const& r, String const& s) : pData(r.pData->append(s.pData))
{
}

String::String(String const& r, int n) : pData(n > 0 ? r.pData->head(n) : String::data::create())
{
}

String::String(int n, String const& r) : pData(n > 0 ? r.pData->tail(n) : Shared::Clone(r.pData))
{
}

String::String(char const* p) : pData(p != 0 && *p != '\0' ? String::data::create(p) : String::data::create())
{
}

String::String(Char_t c, int n) : pData(c != 0 && n != 0 ? String::data::create(c, n) : String::data::create())
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
