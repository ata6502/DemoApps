# DemoApps
This repository contains examples of simple UWP (Universal Windows Platform) applications 
using Direct3D, [WinRT/C++](https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/), 
and XAML. So far, there are two demos in the repository:

* `HelloWorld` - A colored rotating cube -- the most basic 3D application.
* `TexturedCube` - A cube with materials and textures, lit by directional lights.

## Hello World

A colored cube rotating along its axes is a canonical example of 3D animation. As simple as it is, the application
requires a little bit of setup to work properly with XAML and Direct3D:

* The `DeviceResources` class creates both the Direct3D device and the device context as well as the swap chain. It also manages 
the device lost event. 
* The `DemoMain` class loads and executes rudimentary pixel and vertex shaders. It also creates a position/color vertex layout,
vertex and index buffers, a projection matrix.
* The application runs a rendering loop on a worker thread.

<img src="./Docs/Images/HelloWorld.png"/>

## Textured Cube

The `TexturedCube` project extends the Hello World example by introducing the following features:

* Multiple renderers created using a renderer factory:
    + `TextureRenderer` - draws a textured cube lit by two directional lights.
    + `MaterialRenderer` - draws a solid cube lit by a single light.
    + `ColorRenderer` - draws a colored cube whose vertices are transformed dynamically in a vertex shader.
* XAML control panel allows the user to turn cube rotation on and off as well as select a renderer.
* `IndependentInput` provides means for the user to control the point of view using a mouse.
* Directional lights include diffuse, ambient, and specular components.
* Multiple constant buffers minimize the CPU/GPU traffic.

<img src="./Docs/Images/TexturedCube.png"/>

## Simple Waves

This app is under construction. For now, it just renders a colored cube.

# References

Many pieces of code in DemoApps are based on or inspired by the following sources:

* [Luna] "Introduction to 3D Game Programming with DirectX 11" by Frank Luna
* Microsoft sample applications - [Simple3DGameDX](https://github.com/microsoft/Windows-universal-samples/tree/main/Samples/Simple3DGameDX/cppwinrt)
