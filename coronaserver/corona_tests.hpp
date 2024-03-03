#ifndef CORONA_TESTS_HPP
#define CORONA_TESTS_HPP

#define CORONA_CONSOLE 1
#include "corona.hpp"

corona::file_batch test_file_awaitable(corona::application& app);
int test_file_straight(corona::application& app);

void corona_tests()
{
    corona::application app;

    try
    {
        std::cout << "\nmain::begin,thread:" << GetCurrentThreadId() << std::endl;

        std::cout << "\n\n=============================================" << std::endl;
        std::cout << "\nmain::test_file_straight,thread:" << GetCurrentThreadId() << std::endl;
        test_file_straight(app);

        std::cout << "\n\n=============================================" << std::endl;
        std::cout << "main::test_file_awaitable,thread:" << GetCurrentThreadId() << std::endl;

        corona::file_batch fb = test_file_awaitable(app);
        std::cout << "   (notice the contents of test_file_awaitable have not run yet):" << GetCurrentThreadId() << std::endl;
        auto result = fb.wait();
        std::cout << "   (the call to wait produced them.):" << GetCurrentThreadId() << std::endl;
        std::cout << "test_file_awaitable::result " << result << ", thread:" << GetCurrentThreadId() << std::endl;

        std::cout << "\n\n=============================================" << std::endl;
        std::cout << "main:test_data_block, thread:" << GetCurrentThreadId() << std::endl;

        corona::file_transaction fb2 = corona::test_data_block(app);
        auto result2 = fb2.wait();
        std::cout << "test_data_block::result " << result << ", thread:" << GetCurrentThreadId() << std::endl;

        std::cout << "\n\n=============================================" << std::endl;
        std::cout << "main:test_json_node, thread:" << GetCurrentThreadId() << std::endl;

        corona::file_transaction fb3 = corona::test_json_node(app);
        auto result3 = fb3.wait();

        corona::user_transaction fb4 = corona::test_json_table(app);
        auto result4 = fb4.wait();

        std::cout << "\nmain::end,thread:" << GetCurrentThreadId() << std::endl;
    }
    catch (std::exception exc)
    {
        std::cout << exc.what() << std::endl;
    }
}

corona::file_batch test_file_nested_awaitable(corona::application& app)
{
    auto fbr = test_file_awaitable(app);
    return fbr;
}

corona::file_batch test_file_awaitable(corona::application& app)
{
    std::cout << "test_file_awaitable::entry, thread:" << GetCurrentThreadId() << std::endl;
    auto fb = corona::test_file(app);
    std::cout << "test_file_awaitable::exit, thread:" << GetCurrentThreadId() << std::endl;
    return fb;
}

int test_file_straight(corona::application& app)
{
    std::cout << "test_file_straight::test, thread:" << GetCurrentThreadId() << std::endl;
    auto batch_result = corona::test_file(app);
    std::cout << "\ntest_file_straight::wait, thread:" << std::endl;
    int result = batch_result.wait();
    std::cout << "\ntest_file_straight::result " << result << ", thread:" << GetCurrentThreadId() << std::endl;
    return result;
}

#endif
