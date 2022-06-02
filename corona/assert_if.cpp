#pragma once

#include "corona.h"

bool assert_if(std::function<bool()> test, const char* fail)
{
    if (!test()) {
        std::cout << fail << std::endl;
        return false;
    }
    return true;
}

bool assert_if(std::function<bool()> test, const wchar_t* fail)
{
    if (!test()) {
        std::wcout << fail << std::endl;
        return false;
    }
    return true;
}
