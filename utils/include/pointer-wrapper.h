#pragma once

namespace Utils
{
    template<typename RawPtr>
    class PointerWrapper
    {
    public:
        explicit PointerWrapper(RawPtr* ptr) : _ptr(ptr) { assert(_ptr && "nullptr / NULL assigned to PointerWrapper instance!"); }

        RawPtr* operator->()
        {
            return _ptr;
        }

        ~PointerWrapper()
        {
            if(_ptr)
            {
                delete _ptr;
            }
        }
    
    private:
        RawPtr* _ptr;
    };
}