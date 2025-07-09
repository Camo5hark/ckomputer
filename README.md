# ckomputer

A Vulkan 1.0 compute wrapper library written in C for general-purpose GPU parallel computing.

## Building

### 1. Clone repository with submodules

<code>git clone --recursive-submodules https://github.com/Camo5hark/ckomputer.git</code>

<code>cd ckomputer</code>

### 2. Configure

<code>mkdir build</code>

<code>cmake -S . -B build -G "Your Preferred Generator"</code>

### 3. Build

Just the library: <code>cmake --build build --target ckomputer</code>

Library and test: <code>cmake --build build --target test-default</code>

## Usage

### 1. Include header(s)

Add <code>ckomputer/include</code> as an include directory to your application. Then, <code>#include <ckomputer/ckomputer.h></code>.

### 2. Link library

Add <code>ckomputer/build</code> as a link directory to your application. Then, link <code>ckomputer</code> shared object/DLL to your application.
