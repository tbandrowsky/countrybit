#include <iostream>
#include <windows.h>
#include <atlbase.h>
#include <atlsafe.h>
#include <iismanager.h>

int main()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        std::cerr << "Failed to initialize COM library: " << hr << std::endl;
        return 1;
    }

    CComPtr<IIisManager> pIisManager;
    hr = CoCreateInstance(CLSID_IisManager, NULL, CLSCTX_INPROC_SERVER, IID_IIisManager, (void**)&pIisManager);
    if (FAILED(hr))
    {
        std::cerr << "Failed to create IIS Manager instance: " << hr << std::endl;
        CoUninitialize();
        return 1;
    }

    CComPtr<IIisServer> pIisServer;
    hr = pIisManager->get_Server(L"localhost", &pIisServer);
    if (FAILED(hr))
    {
        std::cerr << "Failed to get IIS server instance: " << hr << std::endl;
        CoUninitialize();
        return 1;
    }

    CComPtr<IIisSiteCollection> pSiteCollection;
    hr = pIisServer->get_Sites(&pSiteCollection);
    if (FAILED(hr))
    {
        std::cerr << "Failed to get site collection: " << hr << std::endl;
        CoUninitialize();
        return 1;
    }

    long count = 0;
    hr = pSiteCollection->get_Count(&count);
    if (FAILED(hr))
    {
        std::cerr << "Failed to get site count: " << hr << std::endl;
        CoUninitialize();
        return 1;
    }

    for (long i = 0; i < count; ++i)
    {
        CComPtr<IIisSite> pSite;
        hr = pSiteCollection->get_Item(i + 1, &pSite);
        if (FAILED(hr))
        {
            std::cerr << "Failed to get site item: " << hr << std::endl;
            CoUninitialize();
            return 1;
        }

        BSTR name = NULL;
        hr = pSite->get_Name(&name);
        if (FAILED(hr))
        {
            std::cerr << "Failed to get site name: " << hr << std::endl;
            CoUninitialize();
            return 1;
        }

        std::wcout << name << std::endl;

        SysFreeString(name);
    }

    CoUninitialize();

    return 0;
}
