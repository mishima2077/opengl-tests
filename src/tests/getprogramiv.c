#include <stdio.h>
#include <stdlib.h>

#include "glad.h"
#include <GLFW/glfw3.h>

// Window size parameters (important for glViewport)
int width = 1920, height = 1080;

static const char* windowTitle = "glGetProgramiv Test";

// Flag to check if any of the asserts failed
int tests_failed = 0;

static const char* testVertexShaderSource =
    "#version 100\n"
    "attribute vec3 inPosition;\n" // Length 10
    "attribute vec3 inColor;\n"    // Length 7
    "varying vec3 v_color_unused;\n" // Add a varying to use inColor
    "void main()\n"
    "{\n"
    "   v_color_unused = inColor;\n"
    "   gl_Position = vec4(inPosition, 1.0);\n"
    "}\n";

static const char* testFragmentShaderSource =
    "#version 100\n"
    "precision mediump float;\n"
    "uniform vec3 uSomeUniform;\n" // A test uniform
    "varying vec3 v_color_unused;\n"
    "void main()\n"
    "{\n"
    "   vec3 ranVal = v_color_unused * uSomeUniform;\n"
    "   gl_FragColor = vec4(ranVal, 1.0);\n"
    "}\n";

// Shaders for final square
static const char* drawVertexShaderSource =
    "#version 100\n"
    "attribute vec2 aPos;\n"
    "void main() {\n"
    "   gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "}\n";

// Fragment shader now takes a uniform for the color
static const char* drawFragmentShaderSource =
    "#version 100\n"
    "precision mediump float;\n"
    "uniform vec4 uDrawColor;\n"
    "void main() {\n"
    "   gl_FragColor = uDrawColor;\n"
    "}\n";

// Checks a condition and exits with a message if it fails.
void check(int actual, int expected, const char* message);

static GLFWwindow *window;

void init();
void draw();

int main(){
    //GLFW and GLAD init
    if(!glfwInit())
        return -1;

    // Enforce OpenGl es2.0
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(width, height, windowTitle, NULL, NULL);

    if(window == NULL)
    {
        printf("Window couldnt be created.\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if(!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Glad failed.\n");
        glfwTerminate();
        return -1;
    }

    // OpenGl specific
    init();

    while(!glfwWindowShouldClose(window))
    {
        draw();

        // GLFW specific
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    // Ensure GLFW is cleaned up and return a status code based on test results.
    glfwTerminate();
    return tests_failed ? -1 : 0;
}

void init()
{
    GLuint test_vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(test_vs, 1, &testVertexShaderSource, NULL);
    glCompileShader(test_vs);

    GLuint test_fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(test_fs, 1, &testFragmentShaderSource, NULL);
    glCompileShader(test_fs);
    
    GLuint testProgram = glCreateProgram();
    glAttachShader(testProgram, test_vs);
    glAttachShader(testProgram, test_fs);
    glLinkProgram(testProgram);

    GLint result = 0;

    // Run all assertions
    printf("--- Running OpenGL Assertions ---\n");
    glGetProgramiv(testProgram, GL_LINK_STATUS, &result);
    check(result, GL_TRUE, "GL_LINK_STATUS");

    glGetProgramiv(testProgram, GL_ACTIVE_ATTRIBUTES, &result);
    check(result, 2, "GL_ACTIVE_ATTRIBUTES");

    glGetProgramiv(testProgram, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &result);
    check(result, 11, "GL_ACTIVE_ATTRIBUTE_MAX_LENGTH");

    glGetProgramiv(testProgram, GL_ACTIVE_UNIFORMS, &result);
    check(result, 1, "GL_ACTIVE_UNIFORMS");

    glGetProgramiv(testProgram, GL_ATTACHED_SHADERS, &result);
    check(result, 2, "GL_ATTACHED_SHADERS");

    glUseProgram(testProgram);

    glGetProgramiv(testProgram, GL_DELETE_STATUS, &result);
    check(result, GL_FALSE, "GL_DELETE_STATUS (before deletion)");

    glDeleteProgram(testProgram); // Flag for deletion

    glGetProgramiv(testProgram, GL_DELETE_STATUS, &result);
    check(result, GL_TRUE, "GL_DELETE_STATUS (after deletion)");
    printf("---------------------------------\n");

    glUseProgram(0);

    // The test program is no longer needed, clean up its shaders
    glDeleteShader(test_vs);
    glDeleteShader(test_fs);

    // Draw square for success/failure
    GLuint draw_vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(draw_vs, 1, &drawVertexShaderSource, NULL);
    glCompileShader(draw_vs);
    GLuint draw_fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(draw_fs, 1, &drawFragmentShaderSource, NULL);
    glCompileShader(draw_fs);
    GLuint drawProgram = glCreateProgram();
    glAttachShader(drawProgram, draw_vs);
    glAttachShader(drawProgram, draw_fs);
    glLinkProgram(drawProgram);
    glDeleteShader(draw_vs);
    glDeleteShader(draw_fs);

    GLint idx = glGetAttribLocation(drawProgram, "aPos");

    float square_vertices[] = {
        -0.5f, -0.5f, // bottom-left
         0.5f, -0.5f, // bottom-right
        -0.5f,  0.5f, // top-left
         0.5f,  0.5f  // top-right
    };

    GLuint VBO;
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertices), square_vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(idx, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(idx);

    glUseProgram(drawProgram);

    // Set the square color based on the test results
    GLint drawColorLocation = glGetUniformLocation(drawProgram, "uDrawColor");
    if (tests_failed) { // MODIFIED: Check if the int flag is non-zero
        // Set color to red if any test failed
        glUniform4f(drawColorLocation, 1.0f, 0.0f, 0.0f, 1.0f);
        printf("\nOne or more tests FAILED. Drawing square in RED.\n");
    } else {
        // Set color to green if all tests passed
        glUniform4f(drawColorLocation, 0.2f, 0.8f, 0.3f, 1.0f);
        printf("\nAll tests PASSED. Drawing square in GREEN.\n");
    }
}

void draw()
{
    glViewport(0, 0, width, height);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

// Checks a condition and sets an int flag on failure.
void check(int actual, int expected, const char* message) {
    if (actual != expected) {
        fprintf(stderr, "Assertion Failed: %s\n", message);
        fprintf(stderr, "--> Expected: %d, but got: %d\n", expected, actual);
        tests_failed = 1; // Set failure flag to 1
    } else {
        printf("OK: %s (Value: %d)\n", message, actual);
    }
}

// Written by Adil Mert Ergörün, https://github.com/mishima2077/opengl-tests