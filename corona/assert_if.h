#pragma once

#include <string>
#include <functional>
#include <iostream>

bool assert_if(std::function<bool()> test, const char* fail);
bool assert_if(std::function<bool()> test, const wchar_t* fail);
