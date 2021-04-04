
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
#define noexcept

#include <assert.h>
#include <string>

#pragma intrinsic(memcmp, memcpy, memset, strcmp, strlen)

//**********************************************************************************************************************

int UTF8_length(char const* p, size_t n)
{
    assert(p);
    return int(n);
}

size_t UTF8_size(char const* p, int n)
{
    assert(p);
    return size_t(n);
}

Char_t UTF8_char(char const* p)
{
    assert(p);
    return Char_t(*p);
}

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

    virtual bool isASCII() const noexcept = 0;
    virtual char const* buffer() const noexcept = 0;
    virtual char const* extent() const noexcept = 0;
    virtual char const* origin() const noexcept = 0;
    virtual int depth() const noexcept = 0;

    static char const* evaluate(String::data const*&);

protected:
    size_t const BUFFER_LIMIT = 1;
    size_t const STACK_LIMIT = 10000;
};

/***********************************************************************************************************************
*** StringBuffer
***********************************************************************************************************************/

struct StringBuffer final : public String::data, private ObjectGuard<StringBuffer>
{
    StringBuffer(char const* p, size_t n) : nSize(n), nLength(0)
    {
        assert(p && n && p[n] == '\0');
        memcpy(cBuffer, p, n);
        cBuffer[nSize] = '\0';
    }

    StringBuffer(String::data const* p) : nSize(p->size()), nLength(0)
    {
        assert(p);
        p->get(cBuffer, nSize);
        cBuffer[nSize] = '\0';
    }

    StringBuffer(String::data const* p, String::data const* q) : nSize(p->size() + q->size()), nLength(0)
    {
        assert(p && q);
        p->get(cBuffer, p->size());
        q->get(cBuffer + p->size(), q->size());
        cBuffer[nSize] = '\0';
    }

    void* operator new(size_t n, size_t k, bool = false) { return ::operator new(n + k); }
    void operator delete(void* p, size_t, bool) { ::operator delete(p); }

private:
    ~StringBuffer() { }

    void* operator new(size_t) = delete;

    String::data const* append(String::data const* p) const override final;
    String::data const* head(int n) const override final;
    String::data const* tail(int n) const override final;
    String::data const* prepend(String::data const* p) const override final;
    String::data const* stretch(int n) const override final;

    void get(char*, size_t) const noexcept override final;
    Char_t at(int n) const noexcept override final;
    size_t size() const noexcept override final { return nSize; }
    size_t size(int n) const noexcept override final;
    int length() const noexcept override final { if (nLength) return nLength; nLength = UTF8_length(cBuffer, nSize); return nLength; }

    bool isASCII() const noexcept override final { return length() == nSize; }
    char const* buffer() const noexcept override final { return cBuffer; }
    char const* extent() const noexcept override final { return cBuffer + nSize; }
    char const* origin() const noexcept override final { return cBuffer; }
    int depth() const noexcept override final { return 1; }

    size_t const nSize;
    mutable int nLength;
    char cBuffer[1];  // <---- This must be the last data item!
};

/***********************************************************************************************************************
*** StringTail
***********************************************************************************************************************/

struct StringTail final : public String::data, private ObjectGuard<StringTail>
{
    StringTail(String::data const* p, int n) : pData(p), nCursor(n)
    {
        assert(p && n > 0 && n < p->length());
        assert(dynamic_cast<StringBuffer const*>(p));
    }

private:
    ~StringTail() { Erase(pData); }

    String::data const* append(String::data const* p) const override final;
    String::data const* head(int n) const override final;
    String::data const* tail(int n) const override final;
    String::data const* prepend(String::data const* p) const override final;
    String::data const* stretch(int n) const override final;

    void get(char*, size_t) const noexcept override final;
    Char_t at(int n) const noexcept override final;
    size_t size() const noexcept override final { return pData->size() - size(nCursor); }
    size_t size(int n) const noexcept override final;
    int length() const noexcept override final { return pData->length() - nCursor; }

    bool isASCII() const noexcept override final { return length() == size(); }
    char const* buffer() const noexcept override final { return pData->buffer() + size(nCursor); }
    char const* extent() const noexcept override final { return pData->extent(); }
    char const* origin() const noexcept override final { return pData->origin() + size(nCursor); }
    int depth() const noexcept override final { return 2; }

    String::data const* const pData;
    int nCursor;
};

/***********************************************************************************************************************
*** StringHead
***********************************************************************************************************************/

struct StringHead final : public String::data, private ObjectGuard<StringHead>
{
    StringHead(String::data const* p, int n) noexcept : pData(p), nLength(n)
    {
        assert(p && n > 0 && n < p->length());
        assert(dynamic_cast<StringBuffer const*>(p) || dynamic_cast<StringTail const*>(p));
    }

private:
    ~StringHead() { Erase(pData); }

    String::data const* append(String::data const* p) const override final;
    String::data const* head(int n) const override final;
    String::data const* tail(int n) const override final;
    String::data const* prepend(String::data const* p) const override final;
    String::data const* stretch(int n) const override final;

    void get(char*, size_t) const noexcept override final;
    Char_t at(int n) const noexcept override final;
    size_t size() const noexcept override final { return pData->size(nLength); }
    size_t size(int n) const noexcept override final;
    int length() const noexcept override final { return nLength; }

    bool isASCII() const noexcept override final { return length() == size(); }
    char const* buffer() const noexcept override final { return nullptr; }
    char const* extent() const noexcept override final { return pData->origin() + size(); }
    char const* origin() const noexcept override final { return pData->origin(); }
    int depth() const noexcept override final { return pData->depth() + 1; }

    String::data const* const pData;
    int nLength;
};

/***********************************************************************************************************************
*** StringAppend
***********************************************************************************************************************/

struct StringAppend final : public String::data, private ObjectGuard<StringAppend>
{
    StringAppend(String::data const* p, String::data const* q) noexcept :
        pHead(p),
        pTail(q),
        nCursor(p->length()),
        nLength(nCursor + q->length()),
        nSize(p->size() + q->size()),
        nDepth(std::max(pHead->depth(), pTail->depth()) + 1)
    {
        assert(p && q);
        assert(nCursor > 0 && nLength > nCursor);
    }

private:
    ~StringAppend() { Erase(pHead); Erase(pTail); }

    String::data const* append(String::data const*) const override final;
    String::data const* head(int n) const override final;
    String::data const* tail(int n) const override final;
    String::data const* prepend(String::data const*) const override final;
    String::data const* stretch(int n) const override final;

    void get(char*, size_t) const noexcept override final;
    Char_t at(int n) const noexcept override final;
    size_t size() const noexcept override final { return nSize; }
    size_t size(int n) const noexcept override final;
    int length() const noexcept override final { return nLength; }

    bool isASCII() const noexcept override final { return nLength == nSize; }
    char const* buffer() const noexcept override final { return nullptr; }
    char const* extent() const noexcept override final { return pTail->extent(); }
    char const* origin() const noexcept override final { return pHead->origin(); }
    int depth() const noexcept override final { return nDepth; }

    String::data const* const pHead;
    String::data const* const pTail;
    int nCursor;
    int nLength;
    size_t const nSize;
    int nDepth;
};

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
    String::data const* append(String::data const* p) const override final;
    String::data const* head(int n) const override final;
    String::data const* tail(int n) const override final;
    String::data const* prepend(String::data const* p) const override final;
    String::data const* stretch(int n) const override final;

    void get(char*, size_t) const noexcept override final;
    Char_t at(int n) const noexcept override final;
    size_t size() const noexcept override final { return nSize; }
    size_t size(int n) const noexcept override final;
    int length() const noexcept override final { return nLength; }

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

/***********************************************************************************************************************
*** StringBuffer
***********************************************************************************************************************/

String::data const* StringBuffer::append(String::data const* p) const
{
    assert(p);
    return p->prepend(this);
}

String::data const* StringBuffer::head(int n) const
{
    if (n <= 0) return create();
    if (n < length()) return new StringHead(Clone(this), n);
    return Clone(this);
}

String::data const* StringBuffer::tail(int n) const
{
    if (n <= 0) return Clone(this);
    if (n < length()) return new StringTail(Clone(this), n);
    return create();
}

String::data const* StringBuffer::prepend(String::data const* p) const
{
    assert(p);
    if (p->size() + size() <= BUFFER_LIMIT || p->depth() >= STACK_LIMIT) return new(p->size() + size()) StringBuffer(p, this);
    return new StringAppend(Clone(p), Clone(this));
}

String::data const* StringBuffer::stretch(int n) const
{
    assert(n == 0);
    return Clone(this);
}

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

Char_t StringBuffer::at(int n) const noexcept
{
    if (n < 0) return '\0';
    if (n < length()) return UTF8_char(cBuffer + size(n));
    return '\0';
}

size_t StringBuffer::size(int n) const noexcept
{
    if (n <= 0) return 0;
    if (n < length()) return UTF8_size(cBuffer, n);
    return size();
}

/***********************************************************************************************************************
*** StringTail
***********************************************************************************************************************/

String::data const* StringTail::append(String::data const* p) const
{
    assert(p);
    return p->prepend(this);
}

String::data const* StringTail::head(int n) const
{
    if (n <= 0) return create();
    if (n < length()) return new StringHead(Clone(this), n);
    return Clone(this);
}

String::data const* StringTail::tail(int n) const
{
    if (n <= 0) return Clone(this);
    if (n < length()) return pData->tail(nCursor + n);
    return create();
}

String::data const* StringTail::prepend(String::data const* p) const
{
    assert(p);
    if (p->size() + size() <= BUFFER_LIMIT || p->depth() >= STACK_LIMIT) return new(p->size() + size()) StringBuffer(p, this);
    return new StringAppend(Clone(p), Clone(this));
}

String::data const* StringTail::stretch(int n) const
{
    assert(n >= 0 && n <= nCursor);
    return pData->tail(nCursor - n);
}

void StringTail::get(char* p, size_t n) const noexcept
{
    assert(p && n);

    if (n > size())
    {
        memcpy(p, buffer(), size());
        memset(p + size(), '\0', n - size());
    }
    else
    {
        memcpy(p, buffer(), n);
    }
}

Char_t StringTail::at(int n) const noexcept
{
    if (n < 0) return '\0';
    if (n < length()) return pData->at(nCursor + n);
    return '\0';
}

size_t StringTail::size(int n) const noexcept
{
    if (n <= 0) return 0;
    if (n < length()) return pData->size(nCursor + n) - pData->size(nCursor);
    return pData->size() - pData->size(nCursor);
}

/***********************************************************************************************************************
*** StringHead
***********************************************************************************************************************/

String::data const* StringHead::append(String::data const* p) const
{
    assert(p);
    if (p->origin() == extent()) return p->stretch(nLength);
    return p->prepend(this);
}

String::data const* StringHead::head(int n) const
{
    if (n <= 0) return create();
    if (n < length()) return pData->head(n);
    return Clone(this);
}

String::data const* StringHead::tail(int n) const
{
    if (n <= 0) return Clone(this);

    if (n < length())
    {
        auto step0 = pData->tail(n);
        auto step1 = step0->head(nLength - n);
        Erase(step0);
        return step1;
    }

    return create();
}

String::data const* StringHead::prepend(String::data const* p) const
{
    assert(p);
    if (p->size() + size() <= BUFFER_LIMIT || p->depth() >= STACK_LIMIT) return new(p->size() + size()) StringBuffer(p, this);
    return new StringAppend(Clone(p), Clone(this));
}

String::data const* StringHead::stretch(int n) const
{
    assert(n > 0);

    auto step0 = pData->stretch(n);
    auto step1 = step0->head(nLength + n);
    
    Erase(step0);
    
    return step1;
}

void StringHead::get(char* p, size_t n) const noexcept
{
    assert(p && n);

    if (n > UTF8_size(pData->buffer(), nLength))
    {
        pData->get(p, size());
        memset(p + size(), '\0', n - size());
    }
    else
    {
        pData->get(p, n);
    }
}

Char_t StringHead::at(int n) const noexcept
{
    if (n < length()) return pData->at(n);
    return '\0';
}

size_t StringHead::size(int n) const noexcept
{
    if (n < length()) return pData->size(n);
    return pData->size(length());
}

/***********************************************************************************************************************
*** StringAppend
***********************************************************************************************************************/

String::data const* StringAppend::append(String::data const* p) const
{
    assert(p);

    if (pHead->depth() > std::max(p->depth(), pTail->depth()))
    {
        auto step0 = pTail->append(p);
        auto step1 = pHead->append(step0);

        Erase(step0);

        return step1;
    }

    return p->prepend(this);
}

String::data const* StringAppend::head(int n) const
{
    if (n <= 0) return create();
    if (n < pHead->length()) return pHead->head(n);
    if (n == pHead->length()) return Clone(pHead);

    if (n < length())
    {
        auto step0 = pTail->head(n - nCursor);
        auto step1 = pHead->append(step0);

        Erase(step0);

        return step1;
    }

    return Clone(this);
}

String::data const* StringAppend::tail(int n) const
{
    if (n <= 0) return Clone(this);

    if (n < pHead->length())
    {
        auto step0 = pHead->tail(n);
        auto step1 = step0->append(pTail);
        Erase(step0);
        return step1;
    }

    if (n == pHead->length()) return Clone(pTail);
    if (n < length()) return pTail->tail(n - pHead->length());
    return create();
}

String::data const* StringAppend::prepend(String::data const* p) const
{
    assert(p);

    if (pTail->depth() > std::max(p->depth(), pHead->depth()))
    {
        auto step0 = p->append(pHead);
        auto step1 = step0->append(pTail);
        Erase(step0);
        return step1;
    }

    if (p->size() + size() <= BUFFER_LIMIT && std::max(p->depth(), depth()) >= STACK_LIMIT) return new(p->size() + size()) StringBuffer(p, this);
    return new StringAppend(Clone(p), Clone(this));
}

String::data const* StringAppend::stretch(int n) const
{
    auto step0 = pHead->stretch(n);
    auto step1 = step0->append(pTail);
    
    Erase(step0);
    
    return step1;
}

void StringAppend::get(char* p, size_t n) const noexcept
{
    assert(p && n);

    auto nHead = pHead->size();

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

Char_t StringAppend::at(int n) const noexcept
{
    return n < nCursor ? pHead->at(n) : pTail->at(n - nCursor);
}

size_t StringAppend::size(int n) const noexcept
{
    if (n <= 0) return 0;
    if (n < nCursor) return pHead->size(n);
    if (n == nCursor) return pHead->size();
    if (n < nLength) return pHead->size() + pTail->size(n - nCursor);
    return size();
}

/***********************************************************************************************************************
*** StringRepeat
***********************************************************************************************************************/

String::data const* StringRepeat::append(String::data const* p) const
{
    assert(p);
    return p->origin() == extent() ? p->stretch(length()) : p->prepend(this);
}

String::data const* StringRepeat::head(int n) const
{
    return n < nLength ? create(cData, n) : Clone(this);
}

String::data const* StringRepeat::tail(int n) const
{
    return create(cData, nLength - n);
}

String::data const* StringRepeat::prepend(String::data const* p) const
{
    assert(p);
    if (p->size() + size() <= BUFFER_LIMIT && p->depth() >= STACK_LIMIT) return new(p->size() + size()) StringBuffer(p, this);
    return new StringAppend(Clone(p), Clone(this));
}

String::data const* StringRepeat::stretch(int n) const
{
    assert(n > 0); return create(cData, nLength + n);
}

void StringRepeat::get(char* p, size_t n) const noexcept
{
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

Char_t StringRepeat::at(int n) const noexcept
{
    if (n < 0)
    {
        return 0;
    }

    if (n < nLength)
    {
        return cData;
    }

    return 0;
}

size_t StringRepeat::size(int n) const noexcept
{
    TODO;
}

/***********************************************************************************************************************
*** String::data
***********************************************************************************************************************/

String::data const* String::data::create()
{
    static struct : public String::data
    {
    private:
        String::data const* append(String::data const* p) const override final { assert(p); return Clone(p); }
        String::data const* head(int) const override final { return Clone(this); }
        String::data const* tail(int) const override final { return Clone(this); }
        String::data const* prepend(String::data const* p) const override final { assert(p); return Clone(p); }
        String::data const* stretch(int n) const override final { if (n) FAIL("An attempt was made to stretch an unstretchable string data object."); return Clone(this); }

        void get(char* p, size_t n) const noexcept override final { assert(p && n); memset(p, '\0', n); }
        Char_t at(int) const noexcept override final { return 0; }
        size_t size() const noexcept override final { return 0; }
        size_t size(int) const noexcept override final { return 0; }
        int length() const noexcept override final { return 0; }

        bool isASCII() const noexcept override final { return true; }
        char const* buffer() const noexcept override final { return &cData; }
        char const* extent() const noexcept override final { return &cData; }
        char const* origin() const noexcept override final { return &cData; }
        int depth() const noexcept override final { return 0; }

        char const cData = '\0';
    } const instance;

    return Clone(instance);
}

String::data const* String::data::create(char const* p)
{
    assert(p);

    auto n = strlen(p);

    if (n > 0)
    {
        return new(n) StringBuffer(p, n);
    }

    return create();
}

String::data const* String::data::create(Char_t c, int n)
{
    assert(n >= 0);

    if (c != '\0' && n > 0)
    {
        return new StringRepeat(c, n);
    }

    return create();
}

char const* String::data::evaluate(String::data const*& p)
{
    assert(p);

    auto result = p->buffer();

    if (!result)
    {
        auto pData = new(p->size()) StringBuffer(p);

        Erase(p);
        p = pData;
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

String::String(Char_t c, int n) : pData(c != 0 && n > 0 ? String::data::create(c, n) : String::data::create())
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
