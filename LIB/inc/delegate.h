#pragma once

template <typename Signature>
class Delegate;

template <typename _Res, typename... _ArgTypes>
inline bool operator==(const Delegate<_Res(_ArgTypes...)> &lhs, const Delegate<_Res(_ArgTypes...)> &rhs)
{
    if (lhs.func_ptr)
        return lhs.func_ptr == rhs.func_ptr;
    else if (lhs.object_ptr)
        return (lhs.object_ptr == rhs.object_ptr) && (lhs.stub_ptr == rhs.stub_ptr);
    else
        return (rhs.func_ptr == 0) && (rhs.object_ptr == 0);
}

template <typename _Res, typename... _ArgTypes>
class Delegate<_Res(_ArgTypes...)>
{
public:
    typedef _Res (*func_type)(_ArgTypes...);
    Delegate()
        : object_ptr(0), stub_ptr(0), func_ptr(0)
    {
    }

    template <class T, _Res (T::*TMethod)(_ArgTypes...)>
    void init(T *object_ptr) volatile
    {
        //Delegate<_Res(_ArgTypes...)> d;
        func_ptr = nullptr;
        this->object_ptr = object_ptr;
        stub_ptr = &method_stub<T, TMethod>;
        //return d;
    }
/*
    static Delegate<_Res(_ArgTypes...)> create(func_type function)
    {
        Delegate<_Res(_ArgTypes...)> d;
        d.func_ptr = function;
        return d;
    }
    */
    void init(func_type function) volatile 
    {
        stub_ptr = nullptr;
        func_ptr = function;
    }

    _Res operator()(_ArgTypes... args)
    {
        if (func_ptr)
            return func_ptr(args...);
        else if (object_ptr)
            return (*stub_ptr)(object_ptr, args...);
        else
            return _Res();
    }
    template<typename... _ArgTypes1>
    void operator()(_ArgTypes1... args) const
    {
        if (func_ptr)
            func_ptr(args...);
        else if (object_ptr)
            (*stub_ptr)(object_ptr, args...);
    }

private:
    typedef _Res (*stub_type)(void *, _ArgTypes...);
    void *object_ptr;

    volatile stub_type stub_ptr;
    volatile func_type func_ptr;

    template <class T, _Res (T::*TMethod)(_ArgTypes...)>
    static _Res method_stub(void *object_ptr, _ArgTypes... args)
    {
        T *p = static_cast<T *>(object_ptr);
        return (p->*TMethod)(args...); // #2
    }

    friend bool operator==<_Res,_ArgTypes...>(const Delegate<_Res(_ArgTypes...)> &lhs, const Delegate<_Res(_ArgTypes...)> &rhs);
};

