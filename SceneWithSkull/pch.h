#pragma once
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
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Windows.UI.Xaml.Media.Animation.h>
#include <winrt/Windows.UI.Xaml.Navigation.h>

#include <d3d11_3.h>
#include <windows.ui.xaml.media.dxinterop.h>
#include <DirectXColors.h>
#include <dcommon.h>

#include <concrt.h>
#include <ppltasks.h>
#include <crtdbg.h> // Microsoft's implementation of CRT; defines _ASSERTE

// Run-time assertion. ASSERT is evaluated only in Debug builds.
#define ASSERT _ASSERTE

