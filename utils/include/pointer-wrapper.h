#pragma once

namespace Utils
{
    template<typename RawPtr>
    class PointerWrapper
    {
    public:
        explicit PointerWrapper(RawPtr* ptr) : _ptr(ptr) { assert(_ptr && "PointerWrapper instance initialized with a null pointer!"); }
        PointerWrapper(const PointerWrapper&) = delete;
        PointerWrapper& operator=(const PointerWrapper&) = delete;
        PointerWrapper& operator=(PointerWrapper&& rhs)
        {
            if(this != &rhs)
            {
                delete _ptr;
                _ptr = rhs._ptr;
                rhs._ptr = nullptr;
            }
            return *this;
        }

        RawPtr* operator->()
        {
            return _ptr;
        }

        ~PointerWrapper()
        {
            delete _ptr;
        }
    
    private:
        RawPtr* _ptr;
    };
}