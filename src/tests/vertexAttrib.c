#include <stdio.h>
#include <stdlib.h>

#include "glad.h"
#include <GLFW/glfw3.h>

// Window size parameters (important for glViewport)
int width = 1920, height = 1080;

static GLFWwindow* window;
static GLuint program_id0;
static GLuint program_idMax;

static GLuint pos_vbo, ub_vbo, b_vbo, us_vbo, s_vbo;

// Index values for attributes
static GLuint index_zero = 0;
static GLuint index_last;
static GLuint index_pos;

static const char* windowTitle = "glVertexAttribPointer Test";

static const char* vertexShaderSource =
    "#version 100\n"
    "attribute vec4 a_position;\n"
    "attribute vec3 a_color;\n"
    "varying vec3 v_color;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = a_position;\n"
    "   v_color = a_color;\n"
    "}\n";

static const char* fragmentShaderSource =
    "#version 100\n"
    "precision mediump float;\n"
    "varying vec3 v_color;\n"
    "void main()\n"
    "{\n"
    "   gl_FragColor = vec4(v_color, 1.0);\n"
    "}\n";

// Color data for the 4 corners of the window
static GLubyte ubColors[] = { 255,0,0,  0,255,0,  0,0,255,  255,255,0 };
static GLbyte  bColors[]  = { 127,0,0,  0,127,0,  0,0,127,  127,127,0 };
static GLushort usColors[]= { 65535,0,0, 0,65535,0, 0,0,65535, 65535,65535,0 };
static GLshort sColors[]  = { 32767,0,0, 0,32767,0, 0,0,32767, 32767,32767,0 };

void init();
void draw();
void cleanup();

int main(){
    // GLFW and GLAD init
    if(!glfwInit())
        return -1;

    // Enforce OpenGl es2.0
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(width, height, windowTitle, NULL, NULL);
    if(window == NULL)
    {
        fprintf(stderr, "Window could not be created.\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if(!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "GLAD failed to load.\n");
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
    // Trying edge cases for index value
    GLint max_vertex_attribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attribs);
    if (max_vertex_attribs < 2) {
        fprintf(stderr, "Test program needs at least 2 vertex attributes.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    index_last = max_vertex_attribs - 1;
    index_pos = max_vertex_attribs - 2;

    // Vertex Shader
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, NULL);
    glCompileShader(vs);

    // Fragment Shader
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, NULL);
    glCompileShader(fs);

    // Program 1: Binds 'a_color' to index 0
    program_id0 = glCreateProgram();
    glAttachShader(program_id0, vs);
    glAttachShader(program_id0, fs);
    glBindAttribLocation(program_id0, index_zero, "a_color");
    glBindAttribLocation(program_id0, index_pos, "a_position");
    glLinkProgram(program_id0);

    // Program 2: Binds 'a_color' to the max_vertex_attrib - 1
    program_idMax = glCreateProgram();
    glAttachShader(program_idMax, vs);
    glAttachShader(program_idMax, fs);
    glBindAttribLocation(program_idMax, index_last, "a_color");
    glBindAttribLocation(program_idMax, index_pos, "a_position");
    glLinkProgram(program_idMax);
    
    // Shaders are linked, no longer need them
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLfloat positions[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
    };

    // Position VBO
    glGenBuffers(1, &pos_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

    // Color VBOs
    glGenBuffers(1, &ub_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, ub_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ubColors), ubColors, GL_STATIC_DRAW);

    glGenBuffers(1, &b_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, b_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bColors), bColors, GL_STATIC_DRAW);

    glGenBuffers(1, &us_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, us_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(usColors), usColors, GL_STATIC_DRAW);

    glGenBuffers(1, &s_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sColors), sColors, GL_STATIC_DRAW);

    // Position
    glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
    glVertexAttribPointer(index_pos, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(index_pos);
}

void draw()
{
    int w4 = width / 4, h2 = height / 2;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Top Row: program index 0
    glUseProgram(program_id0);

    // Top Left: GLubyte (normalized and not normalized)
    glBindBuffer(GL_ARRAY_BUFFER, ub_vbo);
    glEnableVertexAttribArray(index_zero);
    
    glViewport(0, h2, w4, h2); // Not normalized
    glVertexAttribPointer(index_zero, 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, (void*)0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glViewport(w4, h2, w4, h2); // Normalized
    glVertexAttribPointer(index_zero, 3, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Top Right: GLbyte (normalized and not normalized)
    glBindBuffer(GL_ARRAY_BUFFER, b_vbo);

    glViewport(w4 * 2, h2, w4, h2); // Not normalized
    glVertexAttribPointer(index_zero, 3, GL_BYTE, GL_FALSE, 0, (void*)0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glViewport(w4 * 3, h2, w4, h2); // Normalized
    glVertexAttribPointer(index_zero, 3, GL_BYTE, GL_TRUE, 0, (void*)0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(index_zero);

    // Bottom Row: program maximum index
    glUseProgram(program_idMax);
    
    // Bottom Left: GLushort (normalized and not normalized)
    glBindBuffer(GL_ARRAY_BUFFER, us_vbo);
    glEnableVertexAttribArray(index_last);

    glViewport(0, 0, w4, h2); // Not normalized
    glVertexAttribPointer(index_last, 3, GL_UNSIGNED_SHORT, GL_FALSE, 0, (void*)0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glViewport(w4, 0, w4, h2); // Normalized
    glVertexAttribPointer(index_last, 3, GL_UNSIGNED_SHORT, GL_TRUE, 0, (void*)0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Bottom Right: GLshort (normalized and not normalized)
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);

    glViewport(w4 * 2, 0, w4, h2); // Not normalized
    glVertexAttribPointer(index_last, 3, GL_SHORT, GL_FALSE, 0, (void*)0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glViewport(w4 * 3, 0, w4, h2); // Normalized
    glVertexAttribPointer(index_last, 3, GL_SHORT, GL_TRUE, 0, (void*)0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(index_last);
}

void cleanup()
{
    glDeleteProgram(program_id0);
    glDeleteProgram(program_idMax);
    glDeleteBuffers(1, &pos_vbo);
    glDeleteBuffers(1, &ub_vbo);
    glDeleteBuffers(1, &b_vbo);
    glDeleteBuffers(1, &us_vbo);
    glDeleteBuffers(1, &s_vbo);
}