#include <stdio.h>
#include <stdlib.h>

#include "glad.h"
#include <GLFW/glfw3.h>

// Window size parameters (important for glViewport)
int width = 1920, height = 1080;

static GLFWwindow* window;
static GLuint shaderProgram;
static GLuint VBO;

static const char* windowTitle = "glSampleCoverage Test";

static const char* vertexShaderSource =
    "#version 100\n"
    "attribute vec3 inPosition;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(inPosition, 1.0);\n"
    "}\n";

static const char* fragmentShaderSource =
    "#version 100\n"
    "precision mediump float;\n"
    "void main()\n"
    "{\n"
    // Use a solid alpha to isolate the effect of glSampleCoverage
    "   gl_FragColor = vec4(0.0, 0.4, 0.8, 1.0);\n"
    "}\n";

void init();
void draw();
void cleanup();

int main(){
    // GLFW and GLAD init
    if(!glfwInit())
        return -1;

    // Enable samples, required for glSampleCoverage function !!!
    // Needs replacement if glfw is removed
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Enforce OpenGl es2.0
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(width, height, windowTitle, NULL, NULL);
    if(window == NULL) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if(!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        return -1;
    }

    // OpenGl init
    init();

    while(!glfwWindowShouldClose(window))
    {
        draw();

        // GLFW specific
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    cleanup();

    glfwTerminate();
    return 0;
}

void init()
{
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, NULL);
    glCompileShader(fs);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    
    GLuint ind_pos = 0;
    glBindAttribLocation(shaderProgram, ind_pos, "inPosition");
    glLinkProgram(shaderProgram);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLfloat vertices[] = {
        -0.8f, -0.8f, 0.0f,
        0.8f, -0.8f, 0.0f,
        0.0f,  0.8f, 0.0f
    };

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(ind_pos, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(ind_pos);
}

void draw()
{
    int w2 = width / 2, h2 = height / 2;

    glClearColor(0.8f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    // Top Left, no sample coverage
    glViewport(0, h2, w2, h2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Top Right, 0.5 inverse: False
    glEnable(GL_SAMPLE_COVERAGE);
    glSampleCoverage(0.5f, GL_FALSE);
    glViewport(w2, h2, w2, h2);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisable(GL_SAMPLE_COVERAGE);

    // Bottom Left, 1.0 inverse: True
    glEnable(GL_SAMPLE_COVERAGE);
    glSampleCoverage(1.0f, GL_TRUE); // Should be invisible
    glViewport(0, 0, w2, h2);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisable(GL_SAMPLE_COVERAGE);
    
    // Bottom Right, 0.5 inverse: True
    glEnable(GL_SAMPLE_COVERAGE);
    glSampleCoverage(0.5f, GL_TRUE); // Should look like top-right
    glViewport(w2, 0, w2, h2);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisable(GL_SAMPLE_COVERAGE);
}

void cleanup()
{
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}