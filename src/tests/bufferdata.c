#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "glad.h"
#include <GLFW/glfw3.h>

// Window size parameters (important for glViewport)
int width = 1920, height = 1080;

static GLFWwindow* window;
static GLuint shaderProgram;
static GLint colorLoc;
static GLuint ind_pos = 0;

// Buffers for Vertex Data (Top Row)
static GLuint vbo_stream, vbo_dynamic, vbo_static_bad, vbo_static_good;

// Buffers for Index Data (Bottom Row)
static GLuint quad_vbo; // Shared vertex data for all EBO tests
static GLuint ebo_stream, ebo_dynamic, ebo_static_bad, ebo_static_good;

static const char* windowTitle = "glBufferData Test";

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
    "uniform vec3 uColor;\n"
    "void main()\n"
    "{\n"
    "   gl_FragColor = vec4(uColor, 1.0);\n"
    "}\n";

void init();
void draw();
void cleanup();

int main(){
    //GLFW and GLAD init
    if(!glfwInit())
        return -1;

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

    // OpenGl specific
    init();

    while(!glfwWindowShouldClose(window))
    {
        draw();

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
    glBindAttribLocation(shaderProgram, ind_pos, "inPosition");
    glLinkProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);

    colorLoc = glGetUniformLocation(shaderProgram, "uColor");

    // Top Row VBOs
    float initial_triangle[] = { -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f };
    glGenBuffers(1, &vbo_stream);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_stream);
    glBufferData(GL_ARRAY_BUFFER, sizeof(initial_triangle), initial_triangle, GL_STREAM_DRAW);

    glGenBuffers(1, &vbo_dynamic);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_dynamic);
    glBufferData(GL_ARRAY_BUFFER, sizeof(initial_triangle), NULL, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &vbo_static_bad);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_static_bad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(initial_triangle), NULL, GL_STATIC_DRAW);

    glGenBuffers(1, &vbo_static_good);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_static_good);
    glBufferData(GL_ARRAY_BUFFER, sizeof(initial_triangle), initial_triangle, GL_STATIC_DRAW);

    // Bottom Row EBOs
    float quad_vertices[] = { -0.5f,-0.5f,0.0f, 0.5f,-0.5f,0.0f, -0.5f,0.5f,0.0f, 0.5f,0.5f,0.0f };
    glGenBuffers(1, &quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    unsigned int initial_indices[] = { 0, 1, 2 };
    glGenBuffers(1, &ebo_stream);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_stream);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(initial_indices), initial_indices, GL_STREAM_DRAW);

    glGenBuffers(1, &ebo_dynamic);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_dynamic);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(initial_indices), NULL, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &ebo_static_bad);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_static_bad);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(initial_indices), NULL, GL_STATIC_DRAW);
    
    glGenBuffers(1, &ebo_static_good);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_static_good);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(initial_indices), initial_indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(ind_pos);
}

void draw()
{
    int w4 = width / 4, h2 = height / 2;

    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    // Animation Calculations
    float time = (float)glfwGetTime();
    float y_offset = sin(time * 5.0f) * 0.2f;

    // Top Row: GL_ARRAY_BUFFER tests

    // Top-Left: STREAM_DRAW (Correct Usage - re-specifying data store infrequently)
    glViewport(0, h2, w4, h2);
    glUniform3f(colorLoc, 0.9f, 0.2f, 0.2f); // Red
    static float last_vbo_update_time = 0;
    if (time - last_vbo_update_time > 1.0f) { // Update once per second
        last_vbo_update_time = time;
        float x_rand = (rand() / (float)RAND_MAX - 0.5f) * 0.4f;
        float y_rand = (rand() / (float)RAND_MAX - 0.5f) * 0.4f;
        float stream_vertices[] = { -0.5f+x_rand, -0.5f+y_rand, 0.0f, 0.5f+x_rand, -0.5f+y_rand, 0.0f, 0.0f+x_rand, 0.5f+y_rand, 0.0f };
        glBindBuffer(GL_ARRAY_BUFFER, vbo_stream);
        // Re-allocate the entire buffer data store
        glBufferData(GL_ARRAY_BUFFER, sizeof(stream_vertices), stream_vertices, GL_STREAM_DRAW);
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo_stream);
    glVertexAttribPointer(ind_pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Top-Middle-Left: DYNAMIC_DRAW (Correct Usage - updating frequently)
    glViewport(w4, h2, w4, h2);
    glUniform3f(colorLoc, 0.2f, 0.9f, 0.2f); // Green
    float dynamic_vertices[] = { -0.5f, -0.5f - y_offset, 0.0f, 0.5f, -0.5f - y_offset, 0.0f, 0.0f, 0.5f - y_offset, 0.0f };
    glBindBuffer(GL_ARRAY_BUFFER, vbo_dynamic);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(dynamic_vertices), dynamic_vertices);
    glVertexAttribPointer(ind_pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Top-Middle-Right: STATIC_DRAW (Incorrect Usage - updated frequently)
    glViewport(w4 * 2, h2, w4, h2);
    glUniform3f(colorLoc, 0.9f, 0.5f, 0.2f); // Orange
    float static_bad_vertices[] = { -0.5f, -0.5f + y_offset, 0.0f, 0.5f, -0.5f + y_offset, 0.0f, 0.0f, 0.5f + y_offset, 0.0f };
    glBindBuffer(GL_ARRAY_BUFFER, vbo_static_bad);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(static_bad_vertices), static_bad_vertices);
    glVertexAttribPointer(ind_pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Top-Right: STATIC_DRAW (Correct Usage - never updated)
    glViewport(w4 * 3, h2, w4, h2);
    glUniform3f(colorLoc, 0.9f, 0.9f, 0.9f); // White
    glBindBuffer(GL_ARRAY_BUFFER, vbo_static_good);
    glVertexAttribPointer(ind_pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Bottom Row: ELEMENT_ARRAY_BUFFER tests
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo); // All EBOs use the same vertex data
    glVertexAttribPointer(ind_pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Bottom-Left: STREAM_DRAW (Correct Usage - re-specifying data store infrequently)
    glViewport(0, 0, w4, h2);
    glUniform3f(colorLoc, 0.2f, 0.2f, 0.9f); // Blue
    static float last_ebo_update_time = 0;
    if (time - last_ebo_update_time > 1.0f) { // Update once per second
        last_ebo_update_time = time;
        unsigned int stream_indices[] = { 0, 1, 2 };
        if (fmod(time, 2.0) > 1.0) { stream_indices[0] = 1; stream_indices[1] = 3; stream_indices[2] = 2; }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_stream);
        // Re-allocate the entire buffer data store (orphaning)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(stream_indices), stream_indices, GL_STREAM_DRAW);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_stream);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

    // Bottom-Middle-Left: DYNAMIC_DRAW (Correct Usage - updating frequently)
    glViewport(w4, 0, w4, h2);
    glUniform3f(colorLoc, 0.2f, 0.9f, 0.9f); // Cyan
    unsigned int dynamic_indices[] = { 0, 1, 3 };
    if (fmod(time, 2.0) > 1.0) { dynamic_indices[0] = 0; dynamic_indices[1] = 2; dynamic_indices[2] = 3; }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_dynamic);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(dynamic_indices), dynamic_indices);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

    // Bottom-Middle-Right: STATIC_DRAW (Incorrect Usage - updated frequently)
    glViewport(w4 * 2, 0, w4, h2);
    glUniform3f(colorLoc, 0.9f, 0.2f, 0.9f); // Magenta
    unsigned int static_bad_indices[] = { 0, 2, 1 };
    if (fmod(time, 2.0) > 1.0) { static_bad_indices[0] = 2; static_bad_indices[1] = 1; static_bad_indices[2] = 3; }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_static_bad);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(static_bad_indices), static_bad_indices);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

    // Bottom-Right: STATIC_DRAW (Correct Usage - never updated)
    glViewport(w4 * 3, 0, w4, h2);
    glUniform3f(colorLoc, 0.9f, 0.9f, 0.2f); // Yellow
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_static_good);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
}

void cleanup()
{
    glDeleteBuffers(1, &vbo_stream);
    glDeleteBuffers(1, &vbo_dynamic);
    glDeleteBuffers(1, &vbo_static_bad);
    glDeleteBuffers(1, &vbo_static_good);
    glDeleteBuffers(1, &quad_vbo);
    glDeleteBuffers(1, &ebo_stream);
    glDeleteBuffers(1, &ebo_dynamic);
    glDeleteBuffers(1, &ebo_static_bad);
    glDeleteBuffers(1, &ebo_static_good);
    glDeleteProgram(shaderProgram);
}

// Written by Adil Mert Ergörün, https://github.com/mishima2077/opengl-tests