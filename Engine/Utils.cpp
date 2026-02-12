#include "pch.h"
#include "Utils.h"
#include <thread>

std::wstring util::ReadAppxTextFileSync(const std::wstring& uriString)
{
    using namespace winrt;
    using namespace Windows::Storage;
    using namespace Windows::Foundation;

    std::wstring result;

    // Run the async WinRT calls on a background thread
    winrt::handle event{ CreateEvent(nullptr, false, false, nullptr) };

    std::thread worker([&]()
        {
            try
            {
                auto file = winrt::Windows::Storage::StorageFile::GetFileFromApplicationUriAsync(Uri(uriString)).get();
                auto text = winrt::Windows::Storage::FileIO::ReadTextAsync(file).get();
                result = text.c_str();
            }
            catch (...)
            {
                result.clear();
            }

            SetEvent(event.get());
        });

    WaitForSingleObject(event.get(), INFINITE);
    worker.join();

    return result;
}
