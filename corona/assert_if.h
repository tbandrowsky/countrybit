#pragma once

#include <string>
#include <functional>
#include <iostream>

template <typename string_type> bool assert_if(std::function<bool()> test, string_type fail)
{
    if (!test()) {
        std::cout << fail << std::endl;
        return false;
    }
    return true;
}
