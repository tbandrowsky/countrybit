module;

export module corona:assert_if;

import "corona-windows-lite.h";

import <iostream>;
import <format>;
import <functional>;

export bool assert_if(int test, const char* fail)
{
    if (!test) {
        std::cout << fail << std::endl;
        return false;
    }
    return true;
}

export bool assert_if(std::function<int()> test, const char* fail)
{
    if (!test()) {
        std::cout << fail << std::endl;
        return false;
    }
    return true;
}

export bool assert_if(int test, const wchar_t* fail)
{
    if (!test) {
        std::wcout << fail << std::endl;
        return false;
    }
    return true;
}

export bool assert_if(std::function<int()> test, const wchar_t* fail)
{
    if (!test()) {
        std::wcout << fail << std::endl;
        return false;
    }
    return true;
}

export void throwOnFail(HRESULT hr, const char* _message)
{
    if (!SUCCEEDED(hr)) {
        throw std::exception(std::format("COM failure:{0}", _message).c_str());
    }
}

export void throwOnNull(void* _ptr, const char* _message)
{
    if (!_ptr) {
        throw std::exception(std::format("null reference failure:{0}", _message).c_str());
    }
}

export void throwOnFalse(bool _ptr, const char* _message)
{
    if (!_ptr) {
        throw std::exception(std::format("object reference failure:{0}", _message).c_str());
    }
}

