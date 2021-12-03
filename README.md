# DemoApps
This repository contains examples of simple UWP (Universal Windows Platform) applications 
using [Direct3D 11](https://docs.microsoft.com/en-us/windows/win32/direct3d11/atoc-dx-graphics-direct3d-11), 
[WinRT/C++](https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/), 
and XAML. So far, there are four demos in the repository:

* `HelloWorld` - A colored rotating cube -- the most basic 3D application.
* `TexturedCube` - A cube with materials and textures, lit by directional lights.
* `SimpleWaves` - A wireframe grid with a height function.
* `SimpleScene` - A scene built from meshes.

## Hello World

A rotating cube is a canonical example of 3D animation. As simple as it is, the application
requires quite a bit of setup to work properly with XAML and Direct3D:

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
    + `ColorRenderer` - draws a colored cube whose vertices are transformed dynamically in a vertex shader. Also, it uses 32-bit color values
      (`DirectX::PackedVector::XMCOLOR`) instead of a 128-bit colors.
* XAML control panel allows the user to turn cube rotation on and off as well as select a renderer.
* `IndependentInput` provides means for the user to control the point of view using a mouse.
* Directional lights include diffuse, ambient, and specular components.
* Multiple constant buffers minimize the CPU/GPU traffic.

<img src="./Docs/Images/TexturedCube.png"/>

## Simple Waves

This is the first version of the Simple Waves app. For now, it applies a height function to a wireframe grid.

<img src="./Docs/Images/SimpleWaves.png"/>

## Simple Scene

It's the first version of the Simple Scene app. In the source code, it's called SceneWithSkull because 
in the future we will load a skull mesh from a file. 

* The `DrawIndexed` method draws multiple objects by using a separate world matrix for each object.
* The `MeshGenerator` class keeps vertices of all the objects in a single vertex buffer and all indices in a single index buffer.
* The `MeshGenerator` creates the following meshes: Cube, Pyramid, Cylinder, Sphere, Grid, and Geosphere.
* The application allows the user to enable a scissor test that culls pixels that are outside of a scissor rectangle.

<img src="./Docs/Images/SceneWithSkull.png"/>

# References

Many pieces of code in DemoApps are based on or inspired by the following sources:

* [Luna] "Introduction to 3D Game Programming with DirectX 11" by Frank Luna
* Microsoft sample applications - [Simple3DGameDX](https://github.com/microsoft/Windows-universal-samples/tree/main/Samples/Simple3DGameDX/cppwinrt)
