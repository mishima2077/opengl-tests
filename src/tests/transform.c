#include <stdio.h>
#include <stdlib.h>

#include "glad.h"
#include <GLFW/glfw3.h>

// Window size parameters (important for glViewport)
int width = 1920, height = 1080;

static GLFWwindow* window;
static GLuint program2, program3, program4;
static GLint loc2, loc3, loc4;
static GLuint pos_vbo, col_vbo;

static const char* windowTitle = "Transform Test";

static const char* vertexShaderSource2 =
    "#version 100\n"
    "precision mediump float;\n"
    "attribute vec3 inPosition;\n"
    "attribute vec4 inColor;\n"
    "uniform mat2 transform;\n"
    "varying vec4 v_color;\n"
    "void main()\n"
    "{\n"
    "   vec2 new_color = transform * vec2(inColor.r, inColor.g);\n"
    "   v_color = vec4(new_color, inColor.b, inColor.a);\n"
    "   gl_Position = vec4(inPosition, 1.0);\n"
    "}\n";

static const char* vertexShaderSource3 =
    "#version 100\n"
    "precision mediump float;\n"
    "attribute vec3 inPosition;\n"
    "attribute vec4 inColor;\n"
    "uniform mat3 transform;\n"
    "varying vec4 v_color;\n"
    "void main()\n"
    "{\n"
    "   vec3 new_color = transform * inColor.rgb;\n"
    "   v_color = vec4(new_color, inColor.a);\n"
    "   gl_Position = vec4(inPosition, 1.0);\n"
    "}\n";

static const char* vertexShaderSource4 =
    "#version 100\n"
    "precision mediump float;\n"
    "attribute vec3 inPosition;\n"
    "attribute vec4 inColor;\n"
    "uniform mat4 transform;\n"
    "varying vec4 v_color;\n"
    "void main()\n"
    "{\n"
    "   vec4 new_color = transform * inColor;\n"
    "   v_color = new_color;\n"
    "   gl_Position = vec4(inPosition, 1.0);\n"
    "}\n"; 

// Fragment shader simply displays the color it receives.
static const char* fragmentShaderSource =
    "#version 100\n"
    "precision mediump float;\n"
    "varying vec4 v_color;\n"
    "void main()\n"
    "{\n"
    "   gl_FragColor = v_color;\n"
    "}\n";

// Transformation Matrices
static float mat2_identity[] =  { 1.0f, 0.0f, 0.0f, 1.0f };
static float mat2_transform[] = { 1.0f,-0.8f, 0.8f, 1.0f }; // Green -> Red
static float mat2_transpose[] = { 1.0f, 0.8f,-0.8f, 1.0f }; // Red -> Green

static float mat3_identity[] =  { 1,0,0, 0,1,0, 0,0,1 };
static float mat3_transform[] = { 1,-0.8f,-0.8f, 0.8f,1,0, 0.8f,0,1 }; // Green and Blue -> Red
static float mat3_transpose[] = { 1,0.8f,0.8f, -0.8f,1,0, -0.8f,0,1 }; // Red -> Green and Blue

static float mat4_identity[] =  { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
static float mat4_transform[] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0.5f,0.5f,0.5f,1 }; // Transparency should increase (triangle less visible)
static float mat4_transpose[] = { 1,0,0,0.5f, 0,1,0,0.5f, 0,0,1,0.5f, 0,0,0,1 }; // Transparency should decrease (triangle more visible)

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
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if(!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress))
    {
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, NULL);
    glCompileShader(fs);

    GLuint vs2 = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs2, 1, &vertexShaderSource2, NULL);
    glCompileShader(vs2);

    GLuint vs3 = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs3, 1, &vertexShaderSource3, NULL);
    glCompileShader(vs3);

    GLuint vs4 = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs4, 1, &vertexShaderSource4, NULL);
    glCompileShader(vs4);

    GLuint idx_pos = 0;
    GLuint idx_col = 1;

    program2 = glCreateProgram();
    glAttachShader(program2, vs2);
    glAttachShader(program2, fs);
    glBindAttribLocation(program2, idx_pos, "inPosition");
    glBindAttribLocation(program2, idx_col, "inColor");
    glLinkProgram(program2);

    program3 = glCreateProgram();
    glAttachShader(program3, vs3);
    glAttachShader(program3, fs);
    glBindAttribLocation(program3, idx_pos, "inPosition");
    glBindAttribLocation(program3, idx_col, "inColor");
    glLinkProgram(program3);

    program4 = glCreateProgram();
    glAttachShader(program4, vs4);
    glAttachShader(program4, fs);
    glBindAttribLocation(program4, idx_pos, "inPosition");
    glBindAttribLocation(program4, idx_col, "inColor");
    glLinkProgram(program4);

    glDeleteShader(vs2);
    glDeleteShader(vs3);
    glDeleteShader(vs4);
    glDeleteShader(fs);

    // Vertex Data
    // Position
    float position[] = {
        -0.7f, -0.7f, 0.0f,
        0.7f, -0.7f, 0.0f,
        0.0f,  0.7f, 0.0f
    };
    // Color
    float color[] = {
        1.0f, 0.0f, 0.0f, 0.75f, // Left -> Red
        0.0f, 1.0f, 0.0f, 0.75f, // Right -> Green
        0.0f, 0.0f, 1.0f, 0.75f  // Top -> Blue
    };

    // Create Vertex Buffer Objects
    // Position VBO
    glGenBuffers(1, &pos_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);
    glVertexAttribPointer(idx_pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(idx_pos);

    // Color VBO
    glGenBuffers(1, &col_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, col_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
    glVertexAttribPointer(idx_col, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(idx_col);
 
    loc2 = glGetUniformLocation(program2, "transform");
    loc3 = glGetUniformLocation(program3, "transform");
    loc4 = glGetUniformLocation(program4, "transform");
}

void draw()
{
    int w4 = width / 4, h3 = height / 3;

    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // ROW 1, glUniformMatrix2fv
    glUseProgram(program2);
    // Identity
    glViewport(0, 2*h3, w4, h3);
    glUniformMatrix2fv(loc2, 1, GL_FALSE, mat2_identity);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // Transform
    glViewport(w4, 2*h3, w4, h3);
    glUniformMatrix2fv(loc2, 1, GL_FALSE, mat2_transform);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // Transpose
    glViewport(w4*2, 2*h3, w4, h3);
    glUniformMatrix2fv(loc2, 1, GL_TRUE, mat2_transform);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // Manual Transpose
    glViewport(w4*3, 2*h3, w4, h3);
    glUniformMatrix2fv(loc2, 1, GL_FALSE, mat2_transpose);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // ROW 2, glUniformMatrix3fv
    glUseProgram(program3);
    // Identity
    glViewport(0, h3, w4, h3);
    glUniformMatrix3fv(loc3, 1, GL_FALSE, mat3_identity);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // Transform
    glViewport(w4, h3, w4, h3);
    glUniformMatrix3fv(loc3, 1, GL_FALSE, mat3_transform);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // Transpose
    glViewport(w4*2, h3, w4, h3);
    glUniformMatrix3fv(loc3, 1, GL_TRUE, mat3_transform);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // Manual Transpose
    glViewport(w4*3, h3, w4, h3);
    glUniformMatrix3fv(loc3, 1, GL_FALSE, mat3_transpose);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // ROW 3, glUniformMatrix4fv
    glUseProgram(program4);
    // Identity
    glViewport(0, 0, w4, h3);
    glUniformMatrix4fv(loc4, 1, GL_FALSE, mat4_identity);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // Transform
    glViewport(w4, 0, w4, h3);
    glUniformMatrix4fv(loc4, 1, GL_FALSE, mat4_transform);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // Transpose
    glViewport(w4*2, 0, w4, h3);
    glUniformMatrix4fv(loc4, 1, GL_TRUE, mat4_transform);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // Manual Transpose
    glViewport(w4*3, 0, w4, h3);
    glUniformMatrix4fv(loc4, 1, GL_FALSE, mat4_transpose);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void cleanup()
{
    glDeleteBuffers(1, &pos_vbo);
    glDeleteBuffers(1, &col_vbo);
    glDeleteProgram(program2);
    glDeleteProgram(program3);
    glDeleteProgram(program4);
}