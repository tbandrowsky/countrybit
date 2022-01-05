// cbstoretest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "application.h"
#include "queue.h"
#include "function.h"
#include "jobject.h"
#include "sorted_index.h"
#include "array_box.h"

countrybit::system::sync<int> test_queue();
countrybit::system::task<int> calc();

void queue_tests();
bool jstring_tests();
bool table_tests();

int main()
{

    countrybit::system::application aw;

    if (jstring_tests()) std::cout << "string passed" << std::endl;
    if (table_tests()) std::cout << "table passed" << std::endl;
    if (countrybit::database::test_index()) std::cout << "index passed" << std::endl;
    if (countrybit::database::schema_tests()) std::cout << "schema passed" << std::endl;
    if (countrybit::database::collection_tests()) std::cout << "collection passed" << std::endl;
    if (countrybit::database::array_tests()) std::cout << "array object passed" << std::endl;
    if (countrybit::database::array_box_tests()) std::cout << "array box passed" << std::endl;
//    queue_tests();
}

bool assert_if(std::function<bool()> test, std::string fail)
{
    if (!test()) {
        std::cout << fail << std::endl;
        return false;
    }
    return true;
}

bool jstring_tests()
{
    int r = true;
    countrybit::database::istring<5> test1 = "1234567";

    r = r && assert_if([test1]() { return test1.size() == 4; }, "Size incorrect.");
    r = r && assert_if([test1]() { return test1 == "1234"; }, "truncation incorrect.");
    return r;
}

bool table_tests()
{
    int r = true;
    using countrybit::database::istring;
    using countrybit::database::table;
    using countrybit::database::row_range;
    using countrybit::database::static_box;

    struct test_item {
        int id;
        istring<10> name;
        istring<50> description;
    };

    test_item items[5] = {
        { 0, "hello", "long hello" },
        { 1, "goodbye", "long goodbye"},
        { 2, "yes", "you say yes"},
        { 3, "no", "i say no"},
        { 4, "don't", "i don't why you say goodbye"}
    };

    int s = sizeof(items) / sizeof(test_item);
    r = r && assert_if([s]() { return s == 5; }, "size isn't 5");

    using box_type = static_box<10000>;

    box_type box;

    table<test_item> basic;

    auto location = table<test_item>::reserve_table(&box, 20);
    basic = table<test_item>::get_table(&box, location);

    test_item* ti = &items[0];

    for (int i = 0; i < s; i++) {
        row_range rr;
        auto nr = basic.insert(items[i],rr);
        r = r && assert_if([nr, i, rr]() { return rr.stop - rr.start == 1; }, "size isn't 1");
        r = r && assert_if([nr, i, ti]() { return ti[i].id == nr.id && ti[i].description == nr.description && ti[i].name == nr.name; }, "item not stored correctly");
    }

    for (countrybit::database::row_id_type i = 0; i < basic.size(); i++) {
        auto nr = basic[i];
        r = r && assert_if([nr, i, ti]() { return ti[i].id == nr.id && ti[i].description == nr.description && ti[i].name == nr.name; }, "item not stored correctly");
    }

    return r;
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

