// cbstoretest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "application.h"
#include "queue.h"
#include "function.h"
#include "jobject.h"

countrybit::system::sync<int> test_queue();
countrybit::system::task<int> calc();

int main()
{

    countrybit::system::application aw;

    std::cout << "Hello World!\n";

    int t = test_queue();

    std::cout << "Good bye:" << GetCurrentThreadId() << std::endl;

    std::cout << "done:" << GetCurrentThreadId() << std::endl;

}

countrybit::system::task<int> calc()
{
    std::cout << "calc start:" << GetCurrentThreadId() << std::endl;
    std::cout << "calc sleeps:" << GetCurrentThreadId() << std::endl;
    ::Sleep(1000);
    std::cout << "calc returns:" << GetCurrentThreadId() << std::endl;
    co_return 10;
}

countrybit::system::sync<int> test_queue()
{
    std::cout << "test start:" << GetCurrentThreadId() << std::endl;

    int x = 0;

    for (int i = 0; i < 5; i++) {
        auto temp = co_await calc();
        x += temp;
    }

    std::cout << "all done:" << x << " " << GetCurrentThreadId() << std::endl;

    ::Sleep(1000);

    co_return x;
}

