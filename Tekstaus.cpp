
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

#define BUFFER_LIMIT 1
#define STACK_LIMIT 10000

/***********************************************************************************************************************
*** String::data
***********************************************************************************************************************/

struct String::data : public Shared
{
    static String::data const* create(char const*);
    static String::data const* create(Char_t, int) { TODO; }

    virtual String::data const* append(String::data const*) const;
    virtual String::data const* head(int) const;
    virtual String::data const* tail(int) const;
    virtual String::data const* prepend(String::data const*) const;
    virtual String::data const* stretch(int) const { UNREACHABLE; }

    virtual void get(char*, size_t) const noexcept = 0;
    virtual Char_t at(int) const noexcept = 0;
    virtual size_t size() const noexcept = 0;
    virtual size_t size(int) const noexcept = 0;
    virtual int length() const noexcept = 0;
    virtual int length(size_t) const noexcept = 0;

    virtual bool connects(char const*) const noexcept { return false; }
    virtual bool isASCII() const noexcept { return length() == size(); }
    virtual char const* buffer() const noexcept { return nullptr; }
    virtual char const* extent() const noexcept = 0;
    virtual char const* origin() const noexcept = 0;
    virtual int depth() const noexcept { return 1; }

    static char const* evaluate(String::data const*&);

protected:


private:
    void* operator new(size_t n) { return ::operator new(n); }
};

//**********************************************************************************************************************

static struct final : public String::data
{
private:
    String::data const* append(String::data const* p) const override final { return Clone(p); }
    String::data const* head(int) const override final { return Clone(this); }
    String::data const* tail(int) const override final { return Clone(this); }
    String::data const* prepend(String::data const* p) const override final { return Clone(p); }
    String::data const* stretch(int) const override final { TODO; }

    void get(char* p, size_t n) const noexcept override final { assert(p && n); memset(p, '\0', n); }
    Char_t at(int) const noexcept override final { return 0; }
    size_t size() const noexcept override final { return 0; }
    size_t size(int) const noexcept override final { return 0; }
    int length() const noexcept override final { return 0; }
    int length(size_t) const noexcept override final { return 0; }

    bool connects(char const*) const noexcept override final { TODO; }
    bool isASCII() const noexcept override final { return true; }
    char const* buffer() const noexcept override final { return &cBuffer; }
    char const* extent() const noexcept override final { return &cBuffer; }
    char const* origin() const noexcept override final { return &cBuffer; }
    int depth() const noexcept override final { return 0; }

    char const cBuffer = '\0';
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

    void* operator new(size_t n, size_t x, bool = false) { return ::operator new(n + x); }
    void operator delete(void* p, size_t, bool) { ::operator delete(p); }

private:
    ~StringBuffer() { }

    void* operator new(size_t) = delete;
    void* operator new[](size_t) = delete;

    // String::data const* append(String::data const*) const override final { TODO; }
    // String::data const* head(int) const override final { TODO; }
    // String::data const* tail(int) const override final { TODO; }
    // String::data const* prepend(String::data const*) const override final { TODO; }
    String::data const* stretch(int) const override final { TODO; }

    void get(char*, size_t) const noexcept override final;
    Char_t at(int) const noexcept override final { TODO; }
    size_t size() const noexcept override final;
    size_t size(int) const noexcept override final { TODO; }
    int length() const noexcept override final { if (nLength) return nLength; return nSize; TODO; /* !!!! */ }
    int length(size_t) const noexcept override final { TODO; }

    bool connects(char const*) const noexcept override final { TODO; }
    bool isASCII() const noexcept override final { TODO; }
    char const* buffer() const noexcept override final { return cBuffer; }
    char const* extent() const noexcept override final;
    char const* origin() const noexcept override final;
    // int depth() const noexcept override final { TODO; }

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
        assert(p && n && n < p->length());
        assert(dynamic_cast<StringBuffer const*>(p));
    }

private:
    ~StringTail()
    {
        Erase(pString);
    }

    // String::data const* append(String::data const*) const override final { TODO; }
    // String::data const* head(int) const override final { TODO; }
    String::data const* tail(int) const override final;
    // String::data const* prepend(String::data const*) const override final { TODO; }
    String::data const* stretch(int) const override final { TODO; }

    void get(char*, size_t) const noexcept override final;
    Char_t at(int) const noexcept override final { TODO; }
    size_t size() const noexcept override final;
    size_t size(int) const noexcept override final { TODO; }
    int length() const noexcept override final;
    int length(size_t) const noexcept override final { TODO; }

    bool connects(char const*) const noexcept override final;
    bool isASCII() const noexcept override final { TODO; }
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
        assert(p && n > 0 && n < p->length());
        assert(dynamic_cast<StringBuffer const*>(p) || dynamic_cast<StringTail const*>(p));
    }

private:
    ~StringHead()
    {
        Erase(pString);
    }

    // String::data const* append(String::data const*) const override final { TODO; }
    String::data const* head(int) const override final;
    String::data const* tail(int) const override final;
    // String::data const* prepend(String::data const*) const override final { TODO; }
    String::data const* stretch(int) const override final { TODO; }

    void get(char*, size_t) const noexcept override final;
    Char_t at(int) const noexcept override final { TODO; }
    size_t size() const noexcept override final;
    size_t size(int) const noexcept override final { TODO; }
    int length() const noexcept override final;
    int length(size_t) const noexcept override final { TODO; }

    bool connects(char const*) const noexcept override final;
    bool isASCII() const noexcept override final { TODO; }
    char const* buffer() const noexcept override final { TODO; }
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
        assert(!!pPrefix && pPrefix->length() > 0 && !!pSuffix && pSuffix->length() > 0);
    }

private:
    ~StringAppend()
    {
        Erase(pHead);
        Erase(pTail);
    }

    String::data const* append(String::data const*) const override final;
    String::data const* head(int) const override final { TODO; }
    String::data const* tail(int) const override final { TODO; }
    String::data const* prepend(String::data const*) const override final;
    String::data const* stretch(int) const override final { TODO; }

    void get(char*, size_t) const noexcept override final;
    Char_t at(int) const noexcept override final { TODO; }
    size_t size() const noexcept override final;
    size_t size(int) const noexcept override final { TODO; }
    int length() const noexcept override final;
    int length(size_t n) const noexcept override final { return n; TODO; /* !!!! */ }

    bool connects(char const*) const noexcept override final;
    bool isASCII() const noexcept override final { TODO; }
    // char const* buffer() const noexcept override final { TODO; }
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
};

/***********************************************************************************************************************
*** create()
***********************************************************************************************************************/

String::data const* String::data::create(char const* p)
{
    assert(p && *p);
    auto n = strlen(p);
    return new(n) StringBuffer(p, n);
}

/***********************************************************************************************************************
*** append()
***********************************************************************************************************************/

String::data const* String::data::append(String::data const* p) const
{
    assert(p);
    return p->prepend(this);
}

String::data const* StringAppend::append(String::data const* p) const
{
    if (pHead->depth() > std::max(p->depth(), pTail->depth()) || p->connects(extent()))
    {
        auto step0 = pTail->append(p);
        auto step1 = pHead->append(step0);

        Erase(step0);

        return step1;
    }

    return p->prepend(this);
}

/***********************************************************************************************************************
*** head()
***********************************************************************************************************************/

String::data const* String::data::head(int n) const
{
    assert(n > 0 && n < length());
    return new StringHead(Clone(this), n);
}

String::data const* StringHead::head(int n) const
{
    assert(n > 0 && n < length());
    return pString->head(n);
}

/***********************************************************************************************************************
*** tail()
***********************************************************************************************************************/

String::data const* String::data::tail(int n) const
{
    assert(n > 0 && n < length());
    return new StringTail(Clone(this), n);
}

String::data const* StringTail::tail(int n) const
{
    assert(n > 0 && n < length());
    return pString->tail(n + nReduceLength);
}

String::data const* StringHead::tail(int n) const
{
    assert(n > 0 && n < length());

    auto step0 = pString->tail(n);
    auto step1 = step0->head(nLength - n);

    Erase(step0);

    return step1;
}

/***********************************************************************************************************************
*** prepend()
***********************************************************************************************************************/

String::data const* String::data::prepend(String::data const* p) const
{
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

/***********************************************************************************************************************
*** at()
***********************************************************************************************************************/

/***********************************************************************************************************************
*** size()
***********************************************************************************************************************/

size_t StringBuffer::size() const noexcept
{
    return nSize;
}

size_t StringTail::size() const noexcept
{
    return pString->size() - nReduceSize;
}

size_t StringHead::size() const noexcept
{
    return nSize;
}

size_t StringAppend::size() const noexcept
{
    return pHead->size() + pTail->size();
}

/***********************************************************************************************************************
*** length()
***********************************************************************************************************************/

int StringTail::length() const noexcept
{
    return pString->length() - nReduceLength;
}

int StringHead::length() const noexcept
{
    return nLength;
}

int StringAppend::length() const noexcept
{
    return pHead->length() + pTail->length();
}

/***********************************************************************************************************************
*** connects()
***********************************************************************************************************************/

bool StringTail::connects(char const* p) const noexcept
{
    return p == buffer();
}

bool StringHead::connects(char const* p) const noexcept
{
    return p == pString->buffer();
}

bool StringAppend::connects(char const* p) const noexcept
{
    return p!= nullptr && p == origin();
}

/***********************************************************************************************************************
*** isASCII()
***********************************************************************************************************************/

/***********************************************************************************************************************
*** buffer()
***********************************************************************************************************************/

char const* StringTail::buffer() const noexcept
{
    return pString->buffer() + nReduceSize;
}

/***********************************************************************************************************************
*** extent()
***********************************************************************************************************************/

char const* StringBuffer::extent() const noexcept
{
    return cBuffer + nSize;
}

char const* StringTail::extent() const noexcept
{
    return pString->extent();
}

char const* StringHead::extent() const noexcept
{
    return pString->origin() + nSize;
}

char const* StringAppend::extent() const noexcept
{
    return pTail->extent();
}

/***********************************************************************************************************************
*** origin()
***********************************************************************************************************************/

char const* StringBuffer::origin() const noexcept
{
    return cBuffer;
}

char const* StringTail::origin() const noexcept
{
    return pString->buffer() + nReduceSize;
}

char const* StringHead::origin() const noexcept
{
    return pString->origin();
}

char const* StringAppend::origin() const noexcept
{
    return pHead->origin();
}

/***********************************************************************************************************************
*** depth()
***********************************************************************************************************************/

int StringTail::depth() const noexcept
{
    return 2;
}

int StringHead::depth() const noexcept
{
    return pString->depth() + 1;
}

int StringAppend::depth() const noexcept
{
    return std::max(pHead->depth(), pTail->depth()) + 1;
}

/***********************************************************************************************************************
*** evaluate()
***********************************************************************************************************************/

char const* String::data::evaluate(String::data const*& p)
{
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
