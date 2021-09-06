// cbstoretest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "application.h"
#include "queue.h"
#include "function.h"
#include "jobject.h"
#include "index.h"

countrybit::system::sync<int> test_queue();
countrybit::system::task<int> calc();

void queue_tests();
void jstring_tests();
void table_tests();
void index_tests();

int main()
{

    countrybit::system::application aw;

    jstring_tests();
    table_tests();
    index_tests();
    queue_tests();
}

bool assert_if(std::function<bool()> test, std::string fail)
{
    if (!test()) {
        std::cout << fail << std::endl;
        return false;
    }
    return true;
}

void jstring_tests()
{
    countrybit::database::jstring<5> test1 = "1234567";

    assert_if([test1]() { return test1.size() == 4; }, "Size incorrect.");
    assert_if([test1]() { return test1 == "1234"; }, "truncation incorrect.");

}

void table_tests()
{
    using countrybit::database::jstring;
    using countrybit::database::table;
    using countrybit::database::row_range;

    struct test_item {
        int id;
        jstring<10> name;
        jstring<50> description;
    };

    test_item items[5] = {
        { 0, "hello", "long hello" },
        { 1, "goodbye", "long goodbye"},
        { 2, "yes", "you say yes"},
        { 3, "no", "i say no"},
        { 4, "don't", "i don't why you say goodbye"}
    };

    int s = sizeof(items) / sizeof(test_item);
    assert_if([s]() { return s == 5; }, "size isn't 5");

    table<test_item, 5> basic;
    basic.init();

    test_item* ti = &items[0];

    for (int i = 0; i < s; i++) {
        row_range rr;
        auto nr = basic.insert(items[i],rr);
        assert_if([nr, i, rr]() { return rr.stop - rr.start == 0; }, "size isn't 1");
        assert_if([nr, i, ti]() { return ti[i].id = nr.id && ti[i].description == nr.description && ti[i].name == nr.name; }, "Names are not the same size");
    }

    for (int i = 0; i < s; i++) {
        row_range rr;
        auto nr = basic.insert(items[i], rr);
        assert_if([nr, i, rr]() { return rr.stop - rr.start == 0; }, "size isn't 1");
        assert_if([nr, i, ti]() { return ti[i].id = nr.id && ti[i].description == nr.description && ti[i].name == nr.name; }, "Names are not the same size");
    }

}

void index_tests()
{
    ;
}

void queue_tests()
{
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

