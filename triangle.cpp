/* Using standard C++ output libraries */
#include <cstdlib>
#include <iostream>
using namespace std;

/* Use glew.h instead of gl.h to get all the GL prototypes declared */
#include <GL/glew.h>
/* Using SDL2 for the base window and OpenGL context init */
#include <SDL.h>
/* ADD GLOBAL VARIABLES HERE LATER */

GLuint program;
GLint attribute_coord2d;

GLint refresh_rate = 30;
bool quit = false;

struct v3 {
    union {
        struct {
            float r;
            float g;
            float b;
        };
        struct {
            float x;
            float y;
            float z;
        };
        float E[3];
    };
};

struct v4 {
    union {
        struct {
            float r;
            float g;
            float b;
            float a;
        };
        struct {
            float x;
            float y;
            float z;
            float w;
        };
        float E[4];
    };
};

enum render_command_type {
    RENDER_COMMAND_CLEAR_COLOR,
    RENDER_COMMAND_CLEAR_DEPTH,
    RENDER_COMMAND_TRIANGLE,
};

struct render_command {
    render_command_type Type;
    v4 Color;
    void* Data;
};

struct render_group {
    int RenderCommandCount;
    render_command* RenderCommands;
};

struct memory_arena {
    int Size;
    int Used;
    void *Data;
};

struct game {
    memory_arena *MainArena;
};

game Game;

bool init_resources(void) {

    Game.MainArena = new memory_arena;

	GLint compile_ok = GL_FALSE, link_ok = GL_FALSE;

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	const char *vs_source =
		//"#version 100\n"  // OpenGL ES 2.0
		"#version 120\n"  // OpenGL 2.1
		"attribute vec2 coord2d;                  "
		"void main(void) {                        "
		"  gl_Position = vec4(coord2d, 0.0, 1.0); "
		"}";
	glShaderSource(vs, 1, &vs_source, NULL);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &compile_ok);
	if (!compile_ok) {
		cerr << "Error in vertex shader" << endl;
		return false;
	}

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	const char *fs_source =
		//"#version 100\n"  // OpenGL ES 2.0
		"#version 120\n"  // OpenGL 2.1
		"void main(void) {        "
		"  gl_FragColor[0] = 0.0; "
		"  gl_FragColor[1] = 0.0; "
		"  gl_FragColor[2] = 1.0; "
		"}";
	glShaderSource(fs, 1, &fs_source, NULL);
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &compile_ok);
	if (!compile_ok) {
		cerr << "Error in fragment shader" << endl;
		return false;
	}

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		cerr << "Error in glLinkProgram" << endl;
		return false;
	}

	const char* attribute_name = "coord2d";
	attribute_coord2d = glGetAttribLocation(program, attribute_name);
	if (attribute_coord2d == -1) {
		cerr << "Could not bind attribute " << attribute_name << endl;
		return false;
	}

	return true;
}

void RenderGroupAdd(render_group *Group, render_command_type Type, v4 Color, void *Data) {
    
}

void RenderGroupAddClearColor(render_group *Group, v4 Color) {
    RenderGroupAdd(Group,RENDER_COMMAND_CLEAR_COLOR, Color, 0);
}

void render(SDL_Window* window, float dt) {

    /* Clear the background as white */
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(program);
	glEnableVertexAttribArray(attribute_coord2d);
	GLfloat triangle_vertices[] = {
	    0.0,  0.8,
	   -0.8, -0.8,
	    0.8, -0.8,
	};
	
	float f = 1 / 5150.0; 
	static float current_pos = 0;
	static float mult = 1;

	current_pos += mult * dt;

	if (current_pos > 1000) {
		mult *= -1;
	}	

	float val = (2.0*M_PI*f*current_pos);
	triangle_vertices[1] = (triangle_vertices[1]*sin(val));
	cout << current_pos << " - " << val <<  " " << f << "\n";

	/* Describe our vertices array to OpenGL (it can't guess its format automatically) */
	glVertexAttribPointer(
		attribute_coord2d, // attribute
		2,                 // number of elements per vertex, here (x,y)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		0,                 // no extra data between each position
		triangle_vertices  // pointer to the C array
						  );
	
	/* Push each element in buffer_vertices to the vertex shader */
	glDrawArrays(GL_TRIANGLES, 0, 3);
	
	glDisableVertexAttribArray(attribute_coord2d);

	/* Display the result */
	SDL_GL_SwapWindow(window);

}

void free_resources() {
	glDeleteProgram(program);
}

void mainLoop(SDL_Window* window) {
	SDL_Event e;
	aUint64 NOW = SDL_GetPerformanceCounter();
	Uint64 LAST = 0;
	double deltaTime = 0;

	while (!quit) {
		//Handle events on queue
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT)
				quit = true;
		}

		LAST = NOW;
		NOW = SDL_GetPerformanceCounter();

		deltaTime = (double)((NOW - LAST)*1000 / SDL_GetPerformanceFrequency() );

		render(window, deltaTime);
	}
}

int main(int argc, char* argv[]) {
	/* SDL-related initialising functions */
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("My First Triangle",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			640, 480,
			SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	SDL_GL_CreateContext(window);

	/* Extension wrangler initialising */
	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK) {
		cerr << "Error: glewInit: " << glewGetErrorString(glew_status) << endl;
		return EXIT_FAILURE;
	}

	/* When all init functions run without errors,
	   the program can initialise the resources */
	if (!init_resources())
		return EXIT_FAILURE;

	/* We can display something if everything goes OK */
	mainLoop(window);

	/* If the program exits in the usual way,
	   free resources and exit with a success */
	free_resources();
	return EXIT_SUCCESS;
}

