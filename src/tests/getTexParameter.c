#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "glad.h"
#include <GLFW/glfw3.h>

// Window size parameters (important for glViewport)
int width = 1920, height = 1080;

static GLFWwindow* window;
static GLuint shaderProgram, shaderProgramCube;
static GLuint vbo, ebo;
static GLuint tex2D[4], texCubeMap[4]; // 0:REPEAT/NEAREST, 1:MIRROR/LINEAR, 2:CLAMP/MIPMAP, 3:Extra
static int g_tests_failed = 0;

static const char* windowTitle = "glGetTexParameter Final Test";

static const char* vertexShaderSource =
    "#version 100\n"
    "attribute vec2 inPosition;\n"
    "attribute vec2 inTexCoord;\n"
    "varying vec2 vTexCoord;\n"
    "void main()\n"
    "{\n"
    "   vTexCoord = inTexCoord;\n"
    "   gl_Position = vec4(inPosition, 0.0, 1.0);\n"
    "}\n";

static const char* fragmentShaderSource =
    "#version 100\n"
    "precision mediump float;\n"
    "varying vec2 vTexCoord;\n"
    "uniform sampler2D uTexture;\n"
    "void main()\n"
    "{\n"
    "   gl_FragColor = texture2D(uTexture, vTexCoord);\n"
    "}\n";

static const char* vertexShaderCubeSource =
    "#version 100\n"
    "attribute vec2 inPosition;\n"
    "varying vec3 vTexCoord;\n"
    "void main()\n"
    "{\n"
    "   vTexCoord = vec3(inPosition, 1.0);\n"
    "   gl_Position = vec4(inPosition, 0.0, 1.0);\n"
    "}\n";

static const char* fragmentShaderCubeSource =
    "#version 100\n"
    "precision mediump float;\n"
    "varying vec3 vTexCoord;\n"
    "uniform samplerCube uCubeTexture;\n"
    "void main()\n"
    "{\n"
    "   gl_FragColor = textureCube(uCubeTexture, normalize(vTexCoord));\n"
    "}\n";

void init();
void draw();
void cleanup();

GLuint create_texture(GLenum target, GLint wrap_s, GLint wrap_t, GLint min_filter, GLint mag_filter);
void run_texture_tests(GLuint tex_id, GLenum target, GLint wrap_s, GLint wrap_t, GLint min_filter, GLint mag_filter);
void check_param_i(GLenum target, const char* name, GLenum pname, GLint expected);
void check_param_f(GLenum target, const char* name, GLenum pname, GLfloat expected);

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
    if(!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress)){ 
        glfwTerminate(); 
        return -1;
    }

    // OpenGl init
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
    return g_tests_failed ? -1 : 0;
}

void init()
{
    // Compile Shaders
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, NULL);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, NULL);
    glCompileShader(fs);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glBindAttribLocation(shaderProgram, 0, "inPosition");
    glBindAttribLocation(shaderProgram, 1, "inTexCoord");
    glLinkProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLuint vsCube = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsCube, 1, &vertexShaderCubeSource, NULL);
    glCompileShader(vsCube);
    GLuint fsCube = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsCube, 1, &fragmentShaderCubeSource, NULL);
    glCompileShader(fsCube);
    shaderProgramCube = glCreateProgram();
    glAttachShader(shaderProgramCube, vsCube);
    glAttachShader(shaderProgramCube, fsCube);
    glBindAttribLocation(shaderProgramCube, 0, "inPosition");
    glLinkProgram(shaderProgramCube);
    glDeleteShader(vsCube);
    glDeleteShader(fsCube);

    // Geometry
    float vertices[] = { -0.8f,-0.8f, -1.5f,-1.5f,  0.8f,-0.8f, 2.5f,-1.5f,  0.8f,0.8f, 2.5f,2.5f, -0.8f,0.8f, -1.5f,2.5f };
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Set attributes for both programs
    glUseProgram(shaderProgram);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glUseProgram(shaderProgramCube);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Creating Textures
    tex2D[0] = create_texture(GL_TEXTURE_2D, GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
    tex2D[1] = create_texture(GL_TEXTURE_2D, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT, GL_LINEAR, GL_LINEAR);
    tex2D[2] = create_texture(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    texCubeMap[0] = create_texture(GL_TEXTURE_CUBE_MAP, GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
    texCubeMap[1] = create_texture(GL_TEXTURE_CUBE_MAP, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT, GL_LINEAR, GL_LINEAR);
    texCubeMap[2] = create_texture(GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    // Testing Textures
    printf("\n--- Testing Textures ---\n");    
    run_texture_tests(tex2D[0], GL_TEXTURE_2D, GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
    run_texture_tests(tex2D[1], GL_TEXTURE_2D, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT, GL_LINEAR, GL_LINEAR);
    run_texture_tests(tex2D[2], GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    run_texture_tests(texCubeMap[0], GL_TEXTURE_CUBE_MAP, GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
    run_texture_tests(texCubeMap[1], GL_TEXTURE_CUBE_MAP, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT, GL_LINEAR, GL_LINEAR);
    run_texture_tests(texCubeMap[2], GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    
    printf("\n--- Test Run Complete ---\n");
    if (g_tests_failed) {
        printf("!!! ONE OR MORE TESTS FAILED. DISPLAYING BLACK SCREEN. !!!\n");
    } else {
        printf("All tests passed.\n");
    }

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);
    glUseProgram(shaderProgramCube);
    glUniform1i(glGetUniformLocation(shaderProgramCube, "uCubeTexture"), 0);
}

void draw()
{
    if (g_tests_failed) {
        // Full Black screen if any of the tests have failed
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    } else {
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        int w4 = width / 4, h2 = height / 2;

        glActiveTexture(GL_TEXTURE0);

        // Render 2D Textures
        glUseProgram(shaderProgram);
        // Top-Left: REPEAT / NEAREST
        glViewport(0, h2, w4, h2);
        glBindTexture(GL_TEXTURE_2D, tex2D[0]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // Bottom-Left: MIRRORED_REPEAT / LINEAR
        glViewport(0, 0, w4, h2);
        glBindTexture(GL_TEXTURE_2D, tex2D[1]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // Top-Right: CLAMP_TO_EDGE / LINEAR_MIPMAP_LINEAR
        glViewport(w4, h2, w4, h2);
        glBindTexture(GL_TEXTURE_2D, tex2D[2]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // Render Cube Map Textures
        glUseProgram(shaderProgramCube);
        // Top-Left: REPEAT / NEAREST
        glViewport(w4 * 2, h2, w4, h2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texCubeMap[0]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // Bottom-Left: MIRRORED_REPEAT / LINEAR
        glViewport(w4 * 2, 0, w4, h2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texCubeMap[1]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // Top-Right: CLAMP_TO_EDGE / LINEAR_MIPMAP_LINEAR
        glViewport(w4 * 3, h2, w4, h2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texCubeMap[2]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
}

void cleanup()
{
    glDeleteProgram(shaderProgram);
    glDeleteProgram(shaderProgramCube);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteTextures(4, tex2D);
    glDeleteTextures(4, texCubeMap);
}

// This function creates and configures the texture.
GLuint create_texture(GLenum target, GLint wrap_s, GLint wrap_t, GLint min_filter, GLint mag_filter) {
    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(target, tex_id);

    if (target == GL_TEXTURE_2D) {
        // "F" shape on an 8x8 texture
        unsigned char F_tex[]={0,0,0,0,0,0,0,0, 0,255,255,255,255,255,0,0, 0,255,0,0,0,0,0,0, 0,255,255,255,0,0,0,0, 0,255,0,0,0,0,0,0, 0,255,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 8, 8, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, F_tex);
    } else { // GL_TEXTURE_CUBE_MAP
        for (int i = 0; i < 6; i++) {
            unsigned char face_color[3];
            face_color[0] = (i==0 || i==5) ? 255 : 0; // +X Red, -Z Magenta
            face_color[1] = (i==1 || i==4) ? 255 : 0; // -X Green, +Z Cyan
            face_color[2] = (i==2 || i==3) ? 255 : 0; // +Y Blue, -Y Yellow
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, face_color);
        }
    }

    if (min_filter >= GL_NEAREST_MIPMAP_NEAREST) {
        glGenerateMipmap(target);
    }

    // Setting up texture parameters
    glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap_t);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag_filter);
    
    return tex_id;
}

// Function dedicated to running the parameter checks.
void run_texture_tests(GLuint tex_id, GLenum target, GLint wrap_s, GLint wrap_t, GLint min_filter, GLint mag_filter) {
    printf("--- Running checks for Texture ID %d ---\n", tex_id);

    glBindTexture(target, tex_id); 

    check_param_i(target, "WRAP_S", GL_TEXTURE_WRAP_S, wrap_s);
    check_param_f(target, "WRAP_S (as float)", GL_TEXTURE_WRAP_S, (GLfloat)wrap_s);

    check_param_i(target, "WRAP_T", GL_TEXTURE_WRAP_T, wrap_t);
    check_param_f(target, "WRAP_T (as float)", GL_TEXTURE_WRAP_T, (GLfloat)wrap_t);

    check_param_i(target, "MIN_FILTER", GL_TEXTURE_MIN_FILTER, min_filter);
    check_param_f(target, "MIN_FILTER (as float)", GL_TEXTURE_MIN_FILTER, (GLfloat)min_filter);

    check_param_i(target, "MAG_FILTER", GL_TEXTURE_MAG_FILTER, mag_filter);
    check_param_f(target, "MAG_FILTER (as float)", GL_TEXTURE_MAG_FILTER, (GLfloat)mag_filter);
}

// Assertion Helper Functions
void check_param_i(GLenum target, const char* name, GLenum pname, GLint expected) {
    GLint actual = 0;
    glGetTexParameteriv(target, pname, &actual);
    if (expected == actual) {
        printf("  OK  (iv): %s -> Got %d\n", name, actual);
    } else {
        printf("  FAIL(iv): %s -> Expected %d, but got %d\n", name, expected, actual);
        g_tests_failed = 1;
    }
}

void check_param_f(GLenum target, const char* name, GLenum pname, GLfloat expected) {
    GLfloat actual = 0.0f;
    glGetTexParameterfv(target, pname, &actual);
    if (fabsf(expected - actual) < 0.001f) {
        printf("  OK  (fv): %s -> Got %.1f\n", name, actual);
    } else {
        printf("  FAIL(fv): %s -> Expected %.1f, but got %.1f\n", name, expected, actual);
        g_tests_failed = 1;
    }
}

// Written by Adil Mert Ergörün, https://github.com/mishima2077/opengl-tests