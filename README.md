# ckomputer

A Vulkan 1.0 compute wrapper library written in C for general-purpose GPU parallel computing. This library is currently in the developmental stage.

## Preparation

### Building

This library requires CMake, a C/C++ toolchain, and a Vulkan SDK to be built. Currently, this project has only been built successfully on Windows using Ninja and LunarG Vulkan SDK. However, it should build successfully on other platforms and using different toolchains.

<b>The <code>VULKAN_SDK_DIR</code> definition must be defined during CMake configuration with the directory of your Vulkan SDK.</b>

Configure CMake:

<code>cmake -DVULKAN_SDK_DIR=C:/VulkanSDK/X.X.X.X -G Ninja -S . -B ./build</code>

Build library:

<code>cmake --build build --target ckomputer</code>

OR build library with test:

<code>cmake --build build --target test-default</code>

### Linking

The resulting shared library from the build process above must then be linked to your application. Include the <code>ckomputer.h</code> header in your application.

### Usage

See usage example in <code>test/default/main.c</code>. Documentation not complete as of now.
