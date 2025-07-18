cmake_minimum_required(VERSION 3.5)
project(OpenGLTests C)

# Find required libraries
find_package(glfw3 REQUIRED)

# The GLAD loader source file is needed by all tests.
set(COMMON_SOURCES src/glad.c)

# Define the main include directory.
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Use file(GLOB) to create a list of all .c files in the tests directory.
file(GLOB TEST_FILES "src/tests/*.c")

message(STATUS "Found test files: ${TEST_FILES}")

# Loop through each test file that was found.
foreach(TEST_FILE ${TEST_FILES})
    # Get the name of the file without the path or extension (e.g., "bufferdata").
    # This will be our executable's name.
    get_filename_component(EXECUTABLE_NAME ${TEST_FILE} NAME_WE)

    # Add an executable target for the current test.
    # It's built from its own .c file and the common GLAD source.
    add_executable(${EXECUTABLE_NAME} ${TEST_FILE} ${COMMON_SOURCES})

    # Set a custom output directory for the compiled executables.
    # This keeps things tidy. All tests will be in the 'build/bin' folder.
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    # Tell this executable where to find its header files (e.g., #include <glad/glad.h>)
    target_include_directories(${EXECUTABLE_NAME} PRIVATE ${INCLUDE_DIR})

    # Link the required libraries (GLFW and the math library 'm')
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE glfw m)

    message(STATUS "Configured test executable: ${EXECUTABLE_NAME}")
endforeach()