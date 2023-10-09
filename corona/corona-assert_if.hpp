
#ifndef CORONA_ASSERT_IF_H
#define CORONA_ASSERT_IF_H

#include "corona-windows-lite.h"

#include <iostream>
#include <format>
#include <functional>

namespace corona
{

   bool assert_if(int test, const char* fail)
    {
        if (!test) {
            std::cout << fail << std::endl;
            return false;
        }
        return true;
    }

    bool assert_if(std::function<int()> test, const char* fail)
    {
        if (!test()) {
            std::cout << fail << std::endl;
            return false;
        }
        return true;
    }

    bool assert_if(int test, const wchar_t* fail)
    {
        if (!test) {
            std::wcout << fail << std::endl;
            return false;
        }
        return true;
    }

    bool assert_if(std::function<int()> test, const wchar_t* fail)
    {
        if (!test()) {
            std::wcout << fail << std::endl;
            return false;
        }
        return true;
    }

    void throwOnFail(HRESULT hr, const char* _message)
    {
        if (!SUCCEEDED(hr)) {
            throw std::exception(std::format("COM failure:{0}", _message).c_str());
        }
    }

    void throwOnNull(void* _ptr, const char* _message)
    {
        if (!_ptr) {
            throw std::exception(std::format("null reference failure:{0}", _message).c_str());
        }
    }

    void throwOnFalse(bool _ptr, const char* _message)
    {
        if (!_ptr) {
            throw std::exception(std::format("object reference failure:{0}", _message).c_str());
        }
    }

}

#endif
