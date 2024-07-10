#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define NOMINMAX

#include <windows.h>
#include <unknwn.h>
#include <hstring.h>

#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.System.Threading.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Input.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Windows.UI.Xaml.Media.Animation.h>
#include <winrt/Windows.UI.Xaml.Navigation.h>

#include <d3d11_3.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <dxgi1_3.h> // already included in d3d11_3.h; we also add dxgi.lib to Linker/Input for completness

#include <concrt.h> // Concurrency
#include <crtdbg.h> // Microsoft's implementation of CRT; contains _ASSERTE

// Run-time assertion. ASSERT is evaluated only in Debug builds.
#define ASSERT _ASSERTE

#if defined(_DEBUG)

// Disable Warning C4505: 'DebugTrace': unreferenced function with internal linkage has been removed
#pragma warning(push)
#pragma warning(disable : 4505)
// Examples: 
// DebugTrace(L"num = %4.2f\n", num);
// DebugTrace(L"%s\n", str.c_str());
// DebugTrace(L"%4.1f, %4.1f\n", x, y);
// DebugTrace(L"Eye: (%4.2f, %4.2f, %4.2f)\n", eye.x, eye.y, eye.z);
static void DebugTrace(const wchar_t* format, ...)
{
    // Generate the message string.
    va_list args;
    va_start(args, format); // initialize the argument list
    wchar_t buffer[1024];
    ASSERT(_vsnwprintf_s(buffer, _countof(buffer) - 1, format, args) != -1);
    va_end(args);

    OutputDebugStringW(buffer); // this is a Windows function
}
#pragma warning(pop)

#endif