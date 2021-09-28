#pragma once

// Reads data from a binary file.
static winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::Streams::IBuffer> ReadDataAsync(winrt::hstring const& filename)
{
    using namespace winrt::Windows::ApplicationModel;
    using namespace winrt::Windows::Storage;

    auto folder = Package::Current().InstalledLocation();
    StorageFile file{ co_await folder.GetFileAsync(filename) };
    co_return co_await FileIO::ReadBufferAsync(file);
}
