/*==============================================================================
Copyright (c) 2009, André Homeyer
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
==============================================================================*/ 

#ifndef common_h
#define common_h

#include <exception>
#include <assert.h>

#include <itkNumericTraits.h>
#include <itkSize.h>
#include <itkVector.h>

namespace PT 
{

class Exception : public std::exception
{
public:
    Exception() {}

    Exception(const char* what) : what_(what) { }

    virtual ~Exception() throw() {}

    virtual const char* what() const throw()
    {
        return what_.c_str(); 
    }

private:
    std::string what_;
};

class IOException : public Exception 
{
public:
    IOException() : Exception() {}
    IOException(const char* what) : Exception(what) {}
};

class InvalidArgumentException : public Exception 
{
public:
    InvalidArgumentException() : Exception() {}
    InvalidArgumentException(const char* what) : Exception(what) {}
};

class DuplicateElementException : public Exception 
{
public:
    DuplicateElementException() : Exception() {}
    DuplicateElementException(const char* what) : Exception(what) {}
};

class NoSuchElementException : public Exception 
{
public:
    NoSuchElementException() : Exception() {}
    NoSuchElementException(const char* what) : Exception(what) {}
};

template<typename T>
struct Range
{
    T min;
    T max;

    Range() : min(itk::NumericTraits<T>::max()), max(itk::NumericTraits<T>::NonpositiveMin()) {}

    template<class T2>
    Range(T2 mi, T2 ma) : min((T)mi), max((T)ma)
    {
        assert(min <= max);
    }

    template<class T2>
    Range(const Range<T2>& range) : min((T)range.min), max((T)range.max)
    {
    }

    T size() const
    {
        return max - min;
    }

    void update(T value)
    {
        if (value < min)
            min = value;
        if (value > max)
            max = value;

        assert(min <= max);
    }

    template <class T2>
    void update(const Range<T2>& range)
    {
        update((T)range.min);
        update((T)range.max);
    }

    bool contains(T value) const
    {
        return (value >= min) && (value <= max);
    }

    template<class T2>
    T map(T2 value, const Range<T2>& range) const
    {
        assert(range.size() != 0);
        return min + (T)( ((double)(value - range.min) / range.size()) * size() );
    }

    template<class T2>
    T mapReverse(T2 value, const Range<T2>& range) const
    {
        assert(range.size() != 0);
        return max - (T)( ((double)(value - range.min) / range.size()) * size() );
    }

    void extend(float percent)
    {
        float offset = (percent / 2) * size();
        min = min - offset;
        max = max + offset;
    }
};

typedef Range<int> IntRange;

template<typename T>
struct Point
{
    T x;
    T y;

    Point() : x(-1), y(-1) {}

    Point(T _x, T _y) : x(_x), y(_y) {}

    Point(const Point<T>& p) : x(p.x), y(p.y) {}

    bool operator==(const Point<T>& p) const
    {
        return x == p.x && y == p.y;
    }

    bool operator!=(const Point<T>& p) const
    {
        return ! operator==(p);
    }

    float dist(const Point<T>& p) const
    {
        float dx = x - p.x;
        float dy = y - p.y;
        return sqrt(dx * dx + dy * dy);
    }
};

typedef Point<int> IntPoint;

/**
 * With primitive types DEREF_RETURN_TYPE should generally be the same as
 * VALUE_TYPE.  For complex types DEREF_RETURN_TYPE sould generally be the
 * refence type of VALUE_TYPE;
 */
template<typename IT, typename VALUE_TYPE, typename DEREF_RETURN_TYPE, DEREF_RETURN_TYPE (*DEREF_FUN)(IT&)>
class IteratorWrapper : public std::iterator<std::forward_iterator_tag, VALUE_TYPE>
{
public:
    IteratorWrapper(const IT& it) : it_(it)
    {}
    ~IteratorWrapper()
    {}

    inline IteratorWrapper& operator=(const IteratorWrapper& other)
    {
        it_ = other.it_;
        return(*this);
    }

    inline bool operator==(const IteratorWrapper& other) const
    {
        return (it_ == other.it_);
    }

    inline bool operator!=(const IteratorWrapper& other) const
    {
        return(it_ != other.it_);
    }

    // it is important to us prefix increment here to prevent creating new
    // temporary instances
    inline IteratorWrapper& operator++()
    {
        ++it_;
        return(*this);
    }

    inline IteratorWrapper operator++(int)
    {
        IteratorWrapper tmp(*this);
        ++(*this);
        return(tmp);
    }

    inline VALUE_TYPE* operator->()
    {
        return &(*(*this)) ;
    }

    inline DEREF_RETURN_TYPE operator*()
    {
        return DEREF_FUN(it_);
    }

protected:
    IT it_;
};

template<class Event>
class EventHandler 
{
public:
    virtual ~EventHandler() {}

    virtual void handleEvent(const Event &event) { }
};

template<class Event>
class EventGenerator
{
public:

    EventGenerator() : eventHandler_(0) { }

    virtual ~EventGenerator() {};

    void setEventHandler(EventHandler<Event> *eventHandler)
    {
        eventHandler_ = eventHandler;
    }

protected:

    void notifyEventHandler(const Event& event)
    {
        if (eventHandler_ != 0)
        {
            eventHandler_->handleEvent(event);
        }
    }

private:

    EventHandler<Event> *eventHandler_;

};

}
#endif
