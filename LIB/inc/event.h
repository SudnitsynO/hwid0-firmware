#pragma once

#include "delegate.h"
#include <algorithm>
#include <list>


#define EVENT_LOCK

#define EVENT_UNLOCK

template <typename... Params> class Event
{
  public:
    typedef Delegate<void(Params...)> SubscriberT;

    Event()
    {
        _current = _subscribers.end();
        _next = _subscribers.end();
    }

    void operator+=(const SubscriberT &subscriber)
    {
        EVENT_LOCK;
        auto fr = std::find(_subscribers.begin(), _subscribers.end(), subscriber);
        if (fr == _subscribers.end())
            _subscribers.push_back(subscriber);
        EVENT_UNLOCK;
    }

    void operator-=(const SubscriberT &subscriber)
    {
        EVENT_LOCK;
        auto fr = std::find(_subscribers.begin(), _subscribers.end(), subscriber);
        if (fr != _subscribers.end())
        {
            if (fr == _current) //если подписчик удаляет сам себя
            {
                _next = _subscribers.erase(fr);
            }
            else
            {
                _subscribers.erase(fr);
            }
        }
        EVENT_UNLOCK;
    }

    void operator()(Params... args)
    {
        EVENT_LOCK;
        _current = _subscribers.begin();
        while (_current != _subscribers.end())
        {
            _next = _current;
            _next++;
            EVENT_UNLOCK;
            (*_current)(args...);
            EVENT_LOCK;
            _current = _next;
        }
        EVENT_UNLOCK;
    }

  private:
    std::list<SubscriberT> _subscribers;
    typename std::list<SubscriberT>::iterator _current, _next;
};