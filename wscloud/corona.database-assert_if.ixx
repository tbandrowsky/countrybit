module;

#include <string>
#include <functional>
#include <iostream>
#include <compare>

export module corona.database:assert_if;

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

