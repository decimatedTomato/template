#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Window size definition (should not be in this file)
#define MONITOR_WIDTH 1920
#define MONITOR_HEIGHT 1080

#include "simulation.h"

// State (should not be in this file)
bool fullscreen = false;
bool just_fullscreened = false, just_loaded = false, just_paused = false, just_stepped = false, just_saved = false;
int prev_width, prev_height, prev_x, prev_y;

// Shaders
unsigned int current_shader;

// Uniform locations
int location_time;
int location_resolution;

int location_texture; // The values of the grid particles
int location_color_count; // The number of colors in the color palette
int location_colors; // The color palette

// Windows
GLFWwindow* window;
int window_width, window_height;

// Monitors
GLFWmonitor* monitor;

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam) {
    //unused parameters are annoying
    if(0) printf("%i", length);
    if(0) printf("%p", userParam);

    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    printf("---------------\n");
    printf("Debug message ( %u ): %s \n", id, message);

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             printf("Source: API"); break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   printf("Source: Window System"); break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: printf("Source: Shader Compiler"); break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     printf("Source: Third Party"); break;
    case GL_DEBUG_SOURCE_APPLICATION:     printf("Source: Application"); break;
    case GL_DEBUG_SOURCE_OTHER:           printf("Source: Other"); break;
    } printf("\n");

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               printf("Type: Error"); break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: printf("Type: Deprecated Behaviour"); break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  printf("Type: Undefined Behaviour"); break;
    case GL_DEBUG_TYPE_PORTABILITY:         printf("Type: Portability"); break;
    case GL_DEBUG_TYPE_PERFORMANCE:         printf("Type: Performance"); break;
    case GL_DEBUG_TYPE_MARKER:              printf("Type: Marker"); break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          printf("Type: Push Group"); break;
    case GL_DEBUG_TYPE_POP_GROUP:           printf("Type: Pop Group"); break;
    case GL_DEBUG_TYPE_OTHER:               printf("Type: Other"); break;
    } printf("\n");

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         printf("Severity: high"); break;
    case GL_DEBUG_SEVERITY_MEDIUM:       printf("Severity: medium"); break;
    case GL_DEBUG_SEVERITY_LOW:          printf("Severity: low"); break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: printf("Severity: notification"); break;
    } printf("\n");
    printf("\n");
}

static char* readFile(const char* filepath) {
	char* buffer = '\0';
	long length;
	FILE* f = fopen(filepath, "r");

	if (f != NULL) {
		fseek(f, 0, SEEK_END); // Will fail with files of size > 4GB
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		buffer = malloc(length + 1);
		if (buffer) {
			fread(buffer, 1, length, f);
		}
		fclose(f);
		buffer[length] = '\0';
	}

	return buffer;
}

static unsigned int CompileShader(unsigned int type, const char* src) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);
    
    /* Error handling */
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(sizeof(char) * length);
        glGetShaderInfoLog(id, length, &length, message);
        printf("Failed to compile %s shader\n", (type == GL_VERTEX_SHADER ? "Vertex":"Fragment"));
        printf("%s\n", message);
        glDeleteShader(id);
        return 0;
    }

    return id;
}

static unsigned int CreateShader(char* vertexShader, char* fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

int init_GLFW(int width, int height, char* name) {
    window_width = width;
    window_height = height;

    /* Initialize the library */
    if (!glfwInit()) return -1;

    /* Create a GLFW debug context */
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(window_width, window_height, name, NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    monitor = glfwGetPrimaryMonitor();
    glfwSwapInterval(1);

    if (GLEW_OK != glewInit()) {
        printf("glewInit failed, something is seriously wrong.\n");
    }

    return 0;
}
void init_Debug_Callback() {
    /* Sets the debug messages to run a certain callback function */
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugOutput, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

    printf("%s\n", glGetString(GL_VERSION));
}

void init_Quad() {
    float square_attributes[] = {
        // positions   texCoords
        -1.0f, -1.0f,  0.0f, 0.0f, // 0
         1.0f, -1.0f,  1.0f, 0.0f, // 1
         1.0f,  1.0f,  1.0f, 1.0f, // 2
        -1.0f,  1.0f,  0.0f, 1.0f  // 3
    };
    unsigned int square_indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    unsigned int square_buffer;
    glGenBuffers(1, &square_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, square_buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * 2 * 2 * sizeof(float), square_attributes, GL_STATIC_DRAW);

    // positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    // uvs
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    unsigned int index_buffer_object;
    glGenBuffers(1, &index_buffer_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), square_indices, GL_STATIC_DRAW);
}

void init_Shader(char *vertex_filepath, char *fragment_filepath) {
    char *vertex_shader = readFile(vertex_filepath);
    char *fragment_shader = readFile(fragment_filepath);
    current_shader = CreateShader(vertex_shader, fragment_shader);
    free(vertex_shader);
    free(fragment_shader);
    glUseProgram(current_shader);
}

void init_Uniforms() {
    location_time = glGetUniformLocation(current_shader, "u_time");
    // assert(location_time != -1);
    glUniform1f(location_time, glfwGetTime());    

    location_resolution = glGetUniformLocation(current_shader, "u_resolution");
    assert(location_resolution != -1);
    glfwGetWindowSize(window, &window_width, &window_height);
    glViewport(0, 0, window_width, window_height);
    glUniform2f(location_resolution, window_width, window_height);

    location_texture = glGetUniformLocation(current_shader, "u_texture");
    // assert(location_texture != -1);
    glUniform1i(location_texture, 0);
}

void take_user_input() {
    glfwPollEvents();
    if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if(!just_fullscreened) {
            fullscreen = !fullscreen;
            just_fullscreened = true;
            if(fullscreen) {
                prev_width = window_width;
                prev_height = window_height;
                glfwGetWindowPos(window, &prev_x, &prev_y);
                glfwSetWindowMonitor(window, monitor, 0, 0, MONITOR_WIDTH, MONITOR_HEIGHT, GLFW_DONT_CARE);
            } else {
                glfwSetWindowMonitor(window, NULL, prev_x, prev_y, prev_width, prev_height, GLFW_DONT_CARE);
            }
        }
    } else just_fullscreened = false;
    if(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        if(!just_paused) {
            pause();
            just_paused = true;
        }
    } else just_paused = false;
    if(glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) {
        if(!just_stepped) {
            step();
            just_stepped = true;
        }
    } else just_stepped = false;
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        if(!just_saved) {
            save();
            just_saved = true;
        }
    } else just_saved = false;
    if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        if(!just_loaded) {
            load();
            just_loaded = true;
        }
    } else just_loaded = false;
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
    }
}

bool render_frame() {
    /* Render here */
    glClear(GL_COLOR_BUFFER_BIT);
    
    /* Update uniforms */
    glfwGetWindowSize(window, &window_width, &window_height); //TODO Change to use callback function
    glViewport(0, 0, window_width, window_height);
    glUniform2f(location_resolution, window_width, window_height);
    glUniform1f(location_time, glfwGetTime());

    /* Draw the bound buffer With an index buffer SQUARE */
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);
    
    return !glfwWindowShouldClose(window);
}

void clean_up() {
    glDeleteProgram(current_shader);
    //TODO probably forgetting some stuff
    glfwTerminate();
}