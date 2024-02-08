// coronaserver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "corona.hpp"

int main()
{
    corona::application app;

    try 
    {
        std::cout << "main::Before Thread Id:" << GetCurrentThreadId() << std::endl;
        auto batch_result = corona::test_file(app);
        std::cout << "\nmain::About to wait:" << std::endl;
        int result = batch_result.wait();
        std::cout << "\nmain::After wait, result:" << result << std::endl;
        std::cout << "\nmain::After Thread Id:" << GetCurrentThreadId() << std::endl;
    }
    catch (std::exception exc)
    {
        std::cout << exc.what() << std::endl;
    }
}

