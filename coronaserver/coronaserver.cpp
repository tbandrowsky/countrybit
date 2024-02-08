// coronaserver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "corona.hpp"

int main()
{
    corona::application app;

    try 
    {
        std::cout << "Before Thread Id:" << GetCurrentThreadId() << std::endl;
        corona::test_file(app);
        std::cout << "After Thread Id:" << GetCurrentThreadId() << std::endl;
    }
    catch (std::exception exc)
    {
        std::cout << exc.what() << std::endl;
    }
}

