#pragma once

#include <string>
#include <functional>
#include <iostream>

bool assert_if(std::function<bool()> test, std::string fail);

