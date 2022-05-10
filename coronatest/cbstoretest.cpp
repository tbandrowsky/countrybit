// cbstoretest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "application.h"
#include "queue.h"
#include "function.h"
#include "jobject.h"
#include "sorted_index.h"
#include "array_box.h"
#include "extractor.h"
#include "assert_if.h"

corona::system::sync<int> test_queue();
corona::system::task<int> calc();

void queue_tests();

int main()
{

    corona::system::application aw;

    if (corona::database::box_tests()) std::cout << "boxes passed" << std::endl;
    if (corona::database::string_tests()) std::cout << "string passed" << std::endl;
    if (corona::database::table_tests()) std::cout << "table passed" << std::endl;
    if (corona::database::test_index()) std::cout << "index passed" << std::endl;
    if (corona::database::array_box_tests()) std::cout << "array box passed" << std::endl;
    if (corona::database::list_box_tests()) std::cout << "list box passed" << std::endl;
    if (corona::database::schema_tests()) std::cout << "schema passed" << std::endl;
    if (corona::database::collection_tests()) std::cout << "collection passed" << std::endl;
    if (corona::database::array_tests()) std::cout << "array object passed" << std::endl;
    if (corona::system::string_extractor::test_basics()) std::cout << "parser basics passed" << std::endl;
    if (corona::system::string_extractor::test_json()) std::cout << "parser json passed" << std::endl;
    if (corona::database::model_tests()) std::cout << "model passed" << std::endl;
//    queue_tests();
}

void queue_tests()
{
    std::cout << "Hello World!\n";

    int t = test_queue();

    std::cout << "Good bye:" << GetCurrentThreadId() << std::endl;

    std::cout << "done:" << GetCurrentThreadId() << std::endl;
}

corona::system::task<int> calc()
{
    std::cout << "calc start:" << GetCurrentThreadId() << std::endl;
    std::cout << "calc sleeps:" << GetCurrentThreadId() << std::endl;
    ::Sleep(1000);
    std::cout << "calc returns:" << GetCurrentThreadId() << std::endl;
    co_return 10;
}

corona::system::sync<int> test_queue()
{
    std::cout << "test start:" << GetCurrentThreadId() << std::endl;

    int x = 0;

    for (int i = 0; i < 5; i++) 
    {
        auto temp = co_await calc();
        x += temp;
    }

    std::cout << "all done:" << x << " " << GetCurrentThreadId() << std::endl;

    ::Sleep(1000);

    co_return x;
}
