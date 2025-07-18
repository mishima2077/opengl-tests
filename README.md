# OpenGL Test Programs

A collection of standalone test programs for various OpenGL features and functions. This repository is for learning, testing, and creating minimal, reproducible examples. I have developed them for OpenGl ES2.0, but it should work on anything higher than it.

## Prerequisites

Before you can build and run these tests, you will need:
* A C++ compiler (g++, clang, MSVC)
* CMake 3.5+
* OpenGL ES2.0+ ,capable hardware and drivers (you may encounter some problems in higher OpenGl functions,
for example higher versions may require you to have Vertex Array Objects, but I haven't done extensive testing about that)
* [GLFW] (for window and input management)
* [GLAD] (included, for loading OpenGL functions)

## How to Build

This project uses CMake to generate platform-native build files.

1.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```
2.  **Run CMake and build:**
    ```bash
    cmake ..
    cmake --build . # Or just make, if you are in linux/macos
    ```
The compiled test executables will be located in the "build/bin" directory.

## Adding a New Test

Adding a new test is simple:

1.  Create a new C file (e.g., `mynew_test.c`) inside the `src/tests/` directory.
2.  Write your OpenGL code.
3.  Go back to your `build` directory and re-run the build command.
4.  Your new executable, `mynew_test`, will now be available in the `build/bin/` directory.

## Available Tests

Here is a list of the current test programs and what they demonstrate:

* **`bufferdata`**: Tests `glBufferData` with various data, specifically tests the difference of GL_STREAM_DRAW, GL_DYNAMIC_DRAW and GL_STATIC_DRAW hints.
* **`getprogramiv`**: Tests the `glGetProgramiv` function, to see if it performs correctly on different situations.
* **`getTexParameter`**: Tests `glGetTexParameter{if}v` functions, so see if it returns the expected values on different types of textures.
* **`samplecoverage`**: Tests `glSampleCoverage` function, specifically the inverse parameter. Sampling must be enabled.
* **`transform`**: Tests `glUniformMatrix{2|3|4}fv` functions, transforming color values with 2x2,3x3 and 4x4 matrices respectively.
* **`vertexAttrib`**: Tests `glVertexAttribPointer` function, specifically the normalized parameter and different data types.