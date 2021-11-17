// cbstoretest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "application.h"
#include "queue.h"
#include "function.h"
#include "jobject.h"
#include "sorted_index.h"

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
    countrybit::database::schema_tests();
    countrybit::database::collection_tests();
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
    countrybit::database::istring<5> test1 = "1234567";

    assert_if([test1]() { return test1.size() == 4; }, "Size incorrect.");
    assert_if([test1]() { return test1 == "1234"; }, "truncation incorrect.");

}

void table_tests()
{
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
    assert_if([s]() { return s == 5; }, "size isn't 5");

    using box_type = static_box<10000>;

    box_type box;

    table<test_item> basic;

    auto location = table<test_item>::reserve_table(&box, 20);
    basic = table<test_item>::get_table(&box, location);

    test_item* ti = &items[0];

    for (int i = 0; i < s; i++) {
        row_range rr;
        auto nr = basic.insert(items[i],rr);
        assert_if([nr, i, rr]() { return rr.stop - rr.start == 1; }, "size isn't 1");
        assert_if([nr, i, ti]() { return ti[i].id == nr.id && ti[i].description == nr.description && ti[i].name == nr.name; }, "item not stored correctly");
    }

    for (countrybit::database::row_id_type i = 0; i < basic.size(); i++) {
        auto nr = basic[i];
        assert_if([nr, i, ti]() { return ti[i].id == nr.id && ti[i].description == nr.description && ti[i].name == nr.name; }, "item not stored correctly");
    }

}

void index_tests()
{
    countrybit::database::test_index();
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

