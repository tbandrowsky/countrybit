/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT License

About this File
Provides asserts for critical system flags.

Notes
Corona needs to use these more often.

For Future Consideration
*/

#ifndef CORONA_ASSERT_IF_H
#define CORONA_ASSERT_IF_H

namespace corona
{

   bool assert_if(int test, const char* fail)
    {
        if (!test) {
            log_warning(fail);
            return false;
        }
        return true;
    }

    bool assert_if(std::function<int()> test, const char* fail)
    {
        if (!test()) {
            log_warning(fail);
            return false;
        }
        return true;
    }

    bool assert_if(int test, const wchar_t* fail)
    {
        if (!test) {
            std::wcout << fail << std::endl;
            return false;
        }
        return true;
    }

    bool assert_if(std::function<int()> test, const wchar_t* fail)
    {
        if (!test()) {
            std::wcout << fail << std::endl;
            return false;
        }
        return true;
    }

    void throwOnFail(HRESULT hr, const char* _message)
    {
        if (!SUCCEEDED(hr)) {
            throw std::exception(std::format("COM failure:{0}", _message).c_str());
        }
    }

    void throwOnNull(void* _ptr, const char* _message)
    {
        if (!_ptr) {
            throw std::exception(std::format("null reference failure:{0}", _message).c_str());
        }
    }

    void throwOnFalse(bool _ptr, const char* _message)
    {
        if (!_ptr) {
            throw std::exception(std::format("object reference failure:{0}", _message).c_str());
        }
    }

}

#endif
