
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

#include "Tekstaus.h"
// #define VERBOSE
#include "Tools.h"

#include <assert.h>
#include <string>
#include <vector>

#pragma intrinsic(memcmp, memcpy, memset, strcmp, strlen)

//**********************************************************************************************************************

int UTF8_length(char const* p, size_t n) noexcept
{
    assert(p);
    return int(n);
}

size_t UTF8_size(char const* p, int n) noexcept
{
    assert(p);
    if (n <= 0) { PROFILER; return 0; }
    return size_t(n);
}

Char_t UTF8_char(char const* p) noexcept
{
    assert(p);
    PROFILER; return *p;
}

size_t char_size(Char_t c)
{
    if (!(c & 0xFFFFFF80)) return 1;  // 0XXXXXXX
    if (!(c & 0xFFFFF800)) return 2;  // 110XXXXx 10xxxxxx
    if (!(c & 0xFFFF0000)) return 3;  // 1110XXXX 10Xxxxxx 10xxxxxx
    if (!(c & 0xFFE00000)) return 4;  // 11110XXX 10XXxxxx 10xxxxxx 10xxxxxx
    if (!(c & 0xFC000000)) return 5;  // 111110XX 10XXXxxx 10xxxxxx 10xxxxxx 10xxxxxx
    if (!(c & 0x80000000)) return 6;  // 1111110X 10XXXXxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    return 7;                         // 11111110 100000Xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx

    assert((c & 0xFFE00000) == 0);
    return (c & 0xFFFFF800) ? (c & 0xFFFF0000) ? 4 : 3 : (c & 0xFFFFFF80) ? 2 : 1;
}

/***********************************************************************************************************************
*** String::data
***********************************************************************************************************************/

struct String::data : public Shared
{
    static data const* create() noexcept;
    static data const* create(char const*);
    static data const* create(Char_t, int);

    virtual data const* append(data const*) const = 0;
    virtual data const* head(int) const = 0;
    virtual data const* tail(int) const = 0;
    virtual data const* prepend(data const*) const = 0;
    virtual data const* stretch(int) const = 0;

    virtual void get(char*, size_t) const noexcept = 0;
    virtual Char_t at(int) const noexcept = 0;
    virtual size_t size(int) const noexcept = 0;
    virtual size_t size() const noexcept = 0;
    virtual int length() const noexcept = 0;

    virtual bool isASCII() const noexcept = 0;
    virtual char const* buffer() const noexcept = 0;
    virtual char const* extent() const noexcept = 0;
    virtual char const* origin() const noexcept = 0;
    virtual int depth() const noexcept = 0;

    static char const* evaluate(data const*&);

protected:
    size_t const BUFFER_LIMIT = 0;
    size_t const STACK_LIMIT = 33554432;
};

/***********************************************************************************************************************
*** StrBuf
***********************************************************************************************************************/

struct StrBuf final : public String::data, private ObjectGuard<StrBuf>
{
    StrBuf(char const* p, size_t n) noexcept : nSize(n)
    {
        assert(p && n && p[n] == '\0');
        memcpy(cBuffer, p, n);
        cBuffer[nSize] = '\0';
    }

    StrBuf(String::data const* p) noexcept : nSize(p->size())
    {
        assert(p);
        p->get(cBuffer, nSize);
        cBuffer[nSize] = '\0';
    }

    StrBuf(String::data const* p, String::data const* q) noexcept : nSize(p->size() + q->size())
    {
        assert(p && q);
        p->get(cBuffer, p->size());
        q->get(cBuffer + p->size(), q->size());
        cBuffer[nSize] = '\0';
    }

    void* operator new(size_t n, size_t k, bool = false) { return ::operator new(n + k); }
    void operator delete(void* p, size_t, bool) noexcept { ::operator delete(p); }

private:
    ~StrBuf() { }

    void* operator new(size_t) = delete;

    String::data const* append(String::data const* p) const override final;
    String::data const* head(int n) const override final;
    String::data const* tail(int n) const override final;
    String::data const* prepend(String::data const* p) const override final;
    String::data const* stretch(int n) const override final;

    void get(char*, size_t) const noexcept override final;
    Char_t at(int n) const noexcept override final;
    size_t size(int n) const noexcept override final;
    size_t size() const noexcept override final { return nSize; }
    int length() const noexcept override final { return nLength ? nLength : nLength = UTF8_length(cBuffer, nSize); }

    bool isASCII() const noexcept override final { PROFILER; return length() == nSize; }
    char const* buffer() const noexcept override final { return cBuffer; }
    char const* extent() const noexcept override final { PROFILER; return cBuffer + nSize; }
    char const* origin() const noexcept override final { return cBuffer; }
    int depth() const noexcept override final { return 1; }

    size_t const nSize;
    mutable int nLength = 0;
    mutable char const* pExtent = nullptr;
    char cBuffer[1];  // <---- This must be the last data item!
};

/***********************************************************************************************************************
*** StrTail
***********************************************************************************************************************/

struct StrTail final : public String::data, private ObjectGuard<StrTail>
{
    StrTail(String::data const* p, int n) : pSource(p), nCursor(n)
    {
        assert(p && n > 0 && n < p->length());
        assert(dynamic_cast<StrBuf const*>(p));
    }

private:
    ~StrTail() { Erase(pSource); }

    String::data const* append(String::data const* p) const override final;
    String::data const* head(int n) const override final;
    String::data const* tail(int n) const override final;
    String::data const* prepend(String::data const* p) const override final;
    String::data const* stretch(int n) const override final;

    void get(char*, size_t) const noexcept override final;
    Char_t at(int n) const noexcept override final;
    size_t size(int n) const noexcept override final;
    size_t size() const noexcept override final { return nSize ? nSize : nSize = pSource->size() - skipsize(); }
    int length() const noexcept override final { return nLength ? nLength : nLength = pSource->length() - nCursor; }

    bool isASCII() const noexcept override final { PROFILER; return length() == size(); }
    char const* buffer() const noexcept override final { return pBuffer ? pBuffer : pBuffer = pSource->buffer() + skipsize(); }
    char const* extent() const noexcept override final { PROFILER; return pExtent ? pExtent : pExtent = pSource->extent(); }
    char const* origin() const noexcept override final { return pOrigin ? pOrigin : pOrigin = pSource->origin() + skipsize(); }
    int depth() const noexcept override final { return 2; }

    size_t skipsize() const noexcept { return nSkip ? nSkip : nSkip = pSource->size(nCursor); }

    String::data const* const pSource;
    int const nCursor;
    mutable int nLength = 0;
    mutable size_t nSize = 0;
    mutable size_t nSkip = 0;
    mutable char const* pBuffer = nullptr;
    mutable char const* pExtent = nullptr;
    mutable char const* pOrigin = nullptr;
};

/***********************************************************************************************************************
*** StrHead
***********************************************************************************************************************/

struct StrHead final : public String::data, private ObjectGuard<StrHead>
{
    StrHead(String::data const* p, int n) noexcept : pSource(p), nCursor(n)
    {
        assert(p && n > 0 && n < p->length());
        assert(dynamic_cast<StrBuf const*>(p) || dynamic_cast<StrTail const*>(p));
    }

private:
    ~StrHead() { Erase(pSource); }

    String::data const* append(String::data const* p) const override final;
    String::data const* head(int n) const override final;
    String::data const* tail(int n) const override final;
    String::data const* prepend(String::data const* p) const override final;
    String::data const* stretch(int n) const override final;

    void get(char*, size_t) const noexcept override final;
    Char_t at(int n) const noexcept override final;
    size_t size(int n) const noexcept override final;
    size_t size() const noexcept override final { return nSize ? nSize : nSize = pSource->size(nCursor); }
    int length() const noexcept override final { return nCursor; }

    bool isASCII() const noexcept override final { PROFILER; return length() == size(); }
    char const* buffer() const noexcept override final { return nullptr; }
    char const* extent() const noexcept override final { return pExtent ? pExtent : pExtent = pSource->origin() + size(); }
    char const* origin() const noexcept override final { return pOrigin ? pOrigin : pOrigin = pSource->origin(); }
    int depth() const noexcept override final { return nDepth ? nDepth : nDepth = pSource->depth() + 1; }

    String::data const* const pSource;
    int const nCursor;
    mutable size_t nSize = 0;
    mutable char const* pExtent = nullptr;
    mutable char const* pOrigin = nullptr;
    mutable int nDepth = 0;
};

/***********************************************************************************************************************
*** StrCat
***********************************************************************************************************************/

struct StrCat final : public String::data, private ObjectGuard<StrCat>
{
    StrCat(String::data const* p, String::data const* q) noexcept : pHead(p), pTail(q)
    {
        assert(p && q);
        assert(pHead->length() > 0);
        assert(length() > pHead->length());
    }

private:
    ~StrCat() { Erase(pHead); Erase(pTail); }

    String::data const* append(String::data const*) const override final;
    String::data const* head(int n) const override final;
    String::data const* tail(int n) const override final;
    String::data const* prepend(String::data const*) const override final;
    String::data const* stretch(int n) const override final;

    void get(char*, size_t) const noexcept override final;
    Char_t at(int n) const noexcept override final;
    size_t size(int n) const noexcept override final;
    size_t size() const noexcept override final { return nSize ? nSize : nSize = pHead->size() + pTail->size(); }
    int length() const noexcept override final { return nLength ? nLength : nLength = pHead->length() + pTail->length(); }

    bool isASCII() const noexcept override final { PROFILER; return length() == size(); }
    char const* buffer() const noexcept override final { return nullptr; }
    char const* extent() const noexcept override final { PROFILER; return pExtent ? pExtent : pExtent = pTail->extent(); }
    char const* origin() const noexcept override final { return pOrigin ? pOrigin : pOrigin = pHead->origin(); }
    int depth() const noexcept override final { return nDepth ? nDepth : nDepth = std::max(pHead->depth(), pTail->depth()) + 1; }

    String::data const* const pHead;
    String::data const* const pTail;
    mutable size_t nSize = 0;
    mutable int nLength = 0;
    mutable char const* pExtent = nullptr;
    mutable char const* pOrigin = nullptr;
    mutable int nDepth = 0;
};

/***********************************************************************************************************************
*** StrSum
***********************************************************************************************************************/

struct StrSum final : public String::data, private ObjectGuard<StrSum>
{
    StrSum(String::data const* p, String::data const* q) noexcept : source({ p, q })
    {
        // source.emplace_back(p);
        // source.emplace_back(q);
    }

private:
    ~StrSum() { for (auto const& pSource : source) Erase(pSource); }

    String::data const* append(String::data const* p) const override final;
    String::data const* head(int n) const override final;
    String::data const* tail(int n) const override final;
    String::data const* prepend(String::data const*) const override final;
    String::data const* stretch(int n) const override final;

    void get(char*, size_t) const noexcept override final;
    Char_t at(int n) const noexcept override final;
    size_t size(int n) const noexcept override final;
    size_t size() const noexcept override final;
    int length() const noexcept override final;

    bool isASCII() const noexcept override final { PROFILER; return length() == size(); }
    char const* buffer() const noexcept override final { PROFILER; return nullptr; }
    char const* extent() const noexcept override final { PROFILER; return source.back()->extent(); }
    char const* origin() const noexcept override final { PROFILER; return source.front()->origin(); }
    int depth() const noexcept override final;

    mutable std::vector<String::data const*> source;
    mutable size_t nSize = 0;
    mutable int nLength = 0;
    mutable char const* pExtent = nullptr;
    mutable char const* pOrigin = nullptr;
    mutable int nDepth = 0;
};

/***********************************************************************************************************************
*** StrRep
***********************************************************************************************************************/

struct StrRep final : public String::data, private ObjectGuard<StrRep>
{
    StrRep(Char_t c, int n) : cData(c), nLength(n)
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
    size_t size(int n) const noexcept override final;
    size_t size() const noexcept override final { return nSize ? nSize : nSize = char_size(cData) * nLength; }
    int length() const noexcept override final { return nLength; }

    bool isASCII() const noexcept override final { PROFILER; return !(cData & 0xFFFFFF80); }
    char const* buffer() const noexcept override final { return nullptr; }
    char const* extent() const noexcept override final { return cookie - cData; }
    char const* origin() const noexcept override final { return cookie - cData; }
    int depth() const noexcept override final { return 1; }

    char const* const cookie = nullptr;

    Char_t const cData;
    int const nLength;
    mutable size_t nSize = 0;
};

/***********************************************************************************************************************
*** StrBuf
***********************************************************************************************************************/

String::data const* StrBuf::append(String::data const* p) const
{
    assert(p);
    return p->prepend(this);
}

String::data const* StrBuf::head(int n) const
{
    if (n <= 0) { PROFILER; return create(); }
    if (n < length()) { return new StrHead(Clone(this), n); }
    return Clone(this);
}

String::data const* StrBuf::tail(int n) const
{
    if (n <= 0) { PROFILER; return Clone(this); }
    if (n < length()) { return new StrTail(Clone(this), n); }
    return create();
}

String::data const* StrBuf::prepend(String::data const* p) const
{
    assert(p);
    if (p->size() + size() <= BUFFER_LIMIT || p->depth() >= STACK_LIMIT) { return new(p->size() + size()) StrBuf(p, this); }
    return new StrCat(Clone(p), Clone(this));
}

String::data const* StrBuf::stretch(int n) const
{
    assert(n == 0);
    PROFILER; return Clone(this);
}

void StrBuf::get(char* p, size_t n) const noexcept
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

inline Char_t StrBuf::at(int n) const noexcept
{
    if (n < 0) PROFILER; return '\0';
    if (n < length()) { PROFILER; return UTF8_char(cBuffer + size(n)); }
    PROFILER; return '\0';
}

size_t StrBuf::size(int n) const noexcept
{
    if (n <= 0) { PROFILER; return 0; }
    if (n < length()) { return UTF8_size(cBuffer, n); }
    PROFILER; return nSize;
}

/***********************************************************************************************************************
*** StrTail
***********************************************************************************************************************/

String::data const* StrTail::append(String::data const* p) const
{
    assert(p);
    return p->prepend(this);
}

String::data const* StrTail::head(int n) const
{
    if (n <= 0) { PROFILER; return create(); }
    if (n < length()) { return new StrHead(Clone(this), n); }
    return Clone(this);
}

String::data const* StrTail::tail(int n) const
{
    if (n <= 0) { PROFILER; return Clone(this); }
    if (n < length()) { return pSource->tail(nCursor + n); }
    return create();
}

String::data const* StrTail::prepend(String::data const* p) const
{
    assert(p);
    if (p->size() + size() <= BUFFER_LIMIT || p->depth() >= STACK_LIMIT) { return new(p->size() + size()) StrBuf(p, this); }
    return new StrCat(Clone(p), Clone(this));
}

String::data const* StrTail::stretch(int n) const
{
    assert(n >= 0 && n <= nCursor);
    PROFILER; return pSource->tail(nCursor - n);
}

void StrTail::get(char* p, size_t n) const noexcept
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

inline Char_t StrTail::at(int n) const noexcept
{
    if (n < 0) { PROFILER; return '\0'; }
    if (n < length()) { PROFILER; return pSource->at(nCursor + n); }
    PROFILER; return '\0';
}

inline size_t StrTail::size(int n) const noexcept
{
    if (n <= 0) { PROFILER; return 0; }
    if (n < length()) { return pSource->size(nCursor + n) - skipsize(); }
    PROFILER; return size();
}

/***********************************************************************************************************************
*** StrHead
***********************************************************************************************************************/

String::data const* StrHead::append(String::data const* p) const
{
    assert(p);
    if (p->origin() == extent()) { PROFILER; return p->stretch(length()); }
    return p->prepend(this);
}

String::data const* StrHead::head(int n) const
{
    if (n <= 0) { PROFILER; return create(); }
    if (n < length()) { PROFILER; return pSource->head(n); }
    return Clone(this);
}

String::data const* StrHead::tail(int n) const
{
    if (n <= 0) { PROFILER; return Clone(this); }

    if (n < length())
    {
        auto step0 = pSource->tail(n);
        auto step1 = step0->head(nCursor - n);
        Erase(step0);
        PROFILER; return step1;
    }

    return create();
}

String::data const* StrHead::prepend(String::data const* p) const
{
    assert(p);
    if (p->size() + size() <= BUFFER_LIMIT || p->depth() >= STACK_LIMIT) { return new(p->size() + size()) StrBuf(p, this); }
    return new StrCat(Clone(p), Clone(this));
}

String::data const* StrHead::stretch(int n) const
{
    assert(n > 0);

    auto step0 = pSource->stretch(n);
    auto step1 = step0->head(nCursor + n);

    Erase(step0);

    PROFILER; return step1;
}

void StrHead::get(char* p, size_t n) const noexcept
{
    assert(p && n);

    if (n > pSource->size(nCursor))
    {
        pSource->get(p, size());
        memset(p + size(), '\0', n - size());
    }
    else
    {
        pSource->get(p, n);
    }
}

inline Char_t StrHead::at(int n) const noexcept
{
    if (n < length()) { PROFILER; return pSource->at(n); }
    PROFILER; return '\0';
}

inline size_t StrHead::size(int n) const noexcept
{
    if (n < length()) { PROFILER; return pSource->size(n); }
    PROFILER; return pSource->size(length());
}

/***********************************************************************************************************************
*** StrCat
***********************************************************************************************************************/

String::data const* StrCat::append(String::data const* p) const
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

String::data const* StrCat::head(int n) const
{
    if (n <= 0) { PROFILER; return create(); }
    if (n < pHead->length()) { return pHead->head(n); }
    if (n == pHead->length()) { return Clone(pHead); }

    if (n < length())
    {
        auto step0 = pTail->head(n - pHead->length());
        auto step1 = pHead->append(step0);

        Erase(step0);

        return step1;
    }

    return Clone(this);
}

String::data const* StrCat::tail(int n) const
{
    if (n <= 0) { PROFILER; return Clone(this); }

    if (n < pHead->length())
    {
        auto step0 = pHead->tail(n);
        auto step1 = step0->append(pTail);
        Erase(step0);
        return step1;
    }

    if (n == pHead->length()) { return Clone(pTail); }
    if (n < length()) { return pTail->tail(n - pHead->length()); }
    return create();
}

String::data const* StrCat::prepend(String::data const* p) const
{
    assert(p);

    if (pTail->depth() > std::max(p->depth(), pHead->depth()))
    {
        auto step0 = p->append(pHead);
        auto step1 = step0->append(pTail);

        Erase(step0);

        return step1;
    }

    if (p->size() + size() <= BUFFER_LIMIT && std::max(p->depth(), depth()) >= STACK_LIMIT) { PROFILER; return new(p->size() + size()) StrBuf(p, this); }
    return new StrCat(Clone(p), Clone(this));
}

String::data const* StrCat::stretch(int n) const
{
    auto step0 = pHead->stretch(n);
    auto step1 = step0->append(pTail);

    Erase(step0);

    PROFILER; return step1;
}

void StrCat::get(char* p, size_t n) const noexcept
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

Char_t StrCat::at(int n) const noexcept
{
    PROFILER; return n < pHead->length() ? pHead->at(n) : pTail->at(n - pHead->length());
}

size_t StrCat::size(int n) const noexcept
{
    if (n <= 0) { PROFILER; return 0; }
    if (n < pHead->length()) { PROFILER; return pHead->size(n); }
    if (n == pHead->length()) { PROFILER; return pHead->size(); }
    if (n < length()) { PROFILER; return pHead->size() + pTail->size(n - pHead->length()); }
    PROFILER; return size();
}

/***********************************************************************************************************************
*** StrSum
***********************************************************************************************************************/

String::data const* StrSum::append(String::data const* p) const
{
    assert(p);

    if (IsShared()) return p->prepend(this);

    source.emplace_back(p);
    nSize += p->size();
    nLength += p->length();
    pExtent = p->extent();
    nDepth = std::max(nDepth, p->depth());

    return Clone(this);
}

/***********************************************************************************************************************
*** StrRep
***********************************************************************************************************************/

String::data const* StrRep::append(String::data const* p) const
{
    assert(p);
    if (p->origin() == extent()) { return p->stretch(length()); }
    return p->prepend(this);
}

String::data const* StrRep::head(int n) const
{
    if (n <= 0) { PROFILER; return create(); }
    if (n < length()) { return create(cData, n); }
    return Clone(this);
}

String::data const* StrRep::tail(int n) const
{
    if (n <= 0) { PROFILER; return Clone(this); }
    if (n < length()) { return create(cData, nLength - n); }
    return create();
}

String::data const* StrRep::prepend(String::data const* p) const
{
    assert(p);
    if (p->size() + size() <= BUFFER_LIMIT && p->depth() >= STACK_LIMIT) { PROFILER; return new(p->size() + size()) StrBuf(p, this); }
    return new StrCat(Clone(p), Clone(this));
}

String::data const* StrRep::stretch(int n) const
{
    assert(n > 0);
    return create(cData, nLength + n);
}

void StrRep::get(char* p, size_t n) const noexcept
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

Char_t StrRep::at(int n) const noexcept
{
    if (n < 0)
    {
        PROFILER; return 0;
    }

    if (n < nLength)
    {
        PROFILER; return cData;
    }

    PROFILER; return 0;
}

size_t StrRep::size(int n) const noexcept
{
    if (n <= 0) { PROFILER; return 0; }
    if (n < length()) { PROFILER; return char_size(cData) * n; }
    PROFILER; return size();
}

/***********************************************************************************************************************
*** String::data
***********************************************************************************************************************/

String::data const* String::data::create() noexcept
{
    static struct : public String::data
    {
    private:
        data const* append(data const* p) const override final { assert(p); return Clone(p); }
        data const* head(int) const override final { return Clone(this); }
        data const* tail(int) const override final { return Clone(this); }
        data const* prepend(data const* p) const override final { assert(p); return Clone(p); }
        data const* stretch(int n) const override final { if (n) FAIL("An attempt was made to stretch an unstretchable string data object."); PROFILER; return Clone(this); }

        void get(char* p, size_t n) const noexcept override final { assert(p && n); memset(p, '\0', n); }
        Char_t at(int) const noexcept override final { PROFILER; return 0; }
        size_t size(int) const noexcept override final { PROFILER; return 0; }
        size_t size() const noexcept override final { PROFILER; return 0; }
        int length() const noexcept override final { PROFILER; return 0; }

        bool isASCII() const noexcept override final { PROFILER; return true; }
        char const* buffer() const noexcept override final { return &cData; }
        char const* extent() const noexcept override final { PROFILER; return &cData; }
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
        return new(n) StrBuf(p, n);
    }

    PROFILER; return create();
}

String::data const* String::data::create(Char_t c, int n)
{
    assert(n >= 0);

    if (c != '\0' && n > 0)
    {
        return new StrRep(c, n);
    }

    PROFILER; return create();
}

char const* String::data::evaluate(data const*& p)
{
    assert(p);

    auto result = p->buffer();

    if (!result)
    {
        auto pData = new(p->size()) StrBuf(p);

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
