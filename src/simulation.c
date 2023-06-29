#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

// Portability issues
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080


// My opengl functions file
#include "handle_opengl.c"

// USER DEFINITIONS
#define DEFAULT_WINDOW_WIDTH SCREEN_WIDTH/3
#define DEFAULT_WINDOW_HEIGHT SCREEN_HEIGHT/3
// #define DEFAULT_WINDOW_WIDTH 640
// #define DEFAULT_WINDOW_HEIGHT 480

#define GRID_WIDTH 50
#define GRID_HEIGHT 50

#define VERTEX_SHADER_FILE_PATH "res/Shaders/vertex_old.glsl"
#define SHADER_FILE_PATH "res/Shaders/frag_test.glsl"

#define START_PAUSED true

// DISPLAY
typedef enum DisplayMode {
    PLAYING, PAUSED
} DisplayMode;

DisplayMode mode = PLAYING;
int window_width = DEFAULT_WINDOW_WIDTH;
int window_height = DEFAULT_WINDOW_HEIGHT;
bool is_fullscreen = false;

/* Update everything in simulation */
void update() {
    for (unsigned int i = 0; i < GRID_WIDTH; i++) {
        for (unsigned int j = 0; j < GRID_HEIGHT; j++) {
            // update_position(i, j);
        }
    }
    // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GRID_WIDTH, GRID_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, current_grid);
}

void load() {
    // load from save
    render_frame();
}

void pause() {
    if (mode == PAUSED) mode = PLAYING;
    else if (mode == PLAYING) mode = PAUSED;
}

void step() {
    mode = PAUSED;
    update();
    render_frame();
}

void save() {
    // save
}

int main() {
    long long unsigned int t = time(NULL);
    printf("Random seed %llu\n", t);
    srand(t);

    if(init_GLFW(window_width, window_height, "Simulation") == -1) exit(1);
    init_Debug_Callback();
    init_Quad();

    init_Shader(VERTEX_SHADER_FILE_PATH, SHADER_FILE_PATH);
    init_Uniforms();

    init_texture();

    /* Loop until the user closes the window */
    bool keep_running = true;
    while (keep_running) {
        take_user_input();
        keep_running = render_frame();
        if (mode != PAUSED) update();
    }

    clean_up();
    return 0;
}