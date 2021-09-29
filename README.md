# DemoApps
This repository contains examples of simple applications using Direct3D, WinRT/C++, and XAML. So far, there are two 
demos in the repository:

* `HelloWorld` - A colored rotating cube -- the most basic 3D application.
* `TexturedCube` - Multiple renderers drawing a single cube with materials and textures, lit by directional lights.

## Hello World

A colored cube rotating along its axes is a canonical example of 3D animation. As simple as it is, the application
requires some setup to work properly with XAML and Direct3D:

* `DeviceResources` creates both a Direct3D Device and DeviceContext as well as the swap chain. It also manages 
the device lost event. 
* `DemoMain` loads and executes rudimentary pixel and vertex shaders. It also creates a position/color vertex layout,
vertex and index buffers, a projection matrix.
* The application runs a rendering loop on a separate thread.

<img src="./Docs/Images/HelloWorld.png"/>

## Textured Cube

The `TexturedCube` project extends the Hello World example significatly. It introduces the following features:

* Multiple renderers created using a renderer factory:
    + `TextureRenderer` - draws a textured cube lit by two directional lights.
    + `MaterialRenderer` - draws a solid cube lit by a single light.
    + `ColorRenderer` - draws a colored cube whose vertices are transformed dynamically in a vertex shader.
* XAML control panel allows the user to turn cube rotation on and off as well as select a renderer.
* `IndependentInput` provides means for the user to control the point of view using a mouse.
* Directional lights include diffuse, ambient, and specular components.
* Multiple constant buffers minimize the CPU/GPU traffic.

<img src="./Docs/Images/TexturedCube.png"/>

# References

Many pieces of code in DemoApps are based on or inspired by the following sources:

* "Introduction to 3D Game Programming with DirectX 11" by Frank Luna
* Microsoft samples
