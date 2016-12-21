#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <cstdlib>
#include <ctime>
#include <vector>

#include "ShaderProgram.h"
#include "Matrix.h"
#include "Utils.h"
#include "Entity.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

// GLOBAL GAME VARIABLES____________________________________________________________________________________________________________________________

// SDL & Rendering Objects
SDL_Window* displayWindow;
GLuint fontTexture;
GLuint playerSpriteTexture;
GLuint groundTexture;
GLuint powerupTexture;

Matrix projectionMatrix;
Matrix viewMatrix;
Matrix modelMatrix;

ShaderProgram* program;
Ut ut; // drawText(), LoadTexture()

// GameLogic & Runtime Values
enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL };
int state;
bool gameRunning = true;
float lastFrameTicks = 0.0f;
float elapsed;
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

// Player Attributes. p1 is players[0]. p2 is players[1]
float playerSpeed = 3.0f;
bool p1controlsMoveLeft = false;
bool p1controlsMoveRight = false;
bool p1controlsJump = false;

bool p2controlsMoveLeft = false;
bool p2controlsMoveRight = false;
bool p2controlsJump = false;

// Game Object containers
std::vector<Entity> players;
std::vector<Entity> blocks;


// RENDERING AND UPDATING CODE____________________________________________________________________________________________________________________________
void RenderMainMenu() {
	//draws text
	modelMatrix.identity();
	modelMatrix.Translate(-1.0f, 2.0f, 0.0f);
	program->setModelMatrix(modelMatrix);
	ut.DrawText(program, fontTexture, "IVEN VS CHUK", 0.2f, 0.0001f);
	modelMatrix.Translate(0.5f, -0.5f, 0.0f);
	program->setModelMatrix(modelMatrix);
	ut.DrawText(program, fontTexture, "CONTROLS", 0.2f, 0.0001f);

	modelMatrix.identity();
	modelMatrix.Translate(-2.6f, 0.0f, 0.0f);
	program->setModelMatrix(modelMatrix);
	ut.DrawText(program, fontTexture, "USE ARROW/WASD KEYS TO MOVE", 0.2f, 0.0001f);

	modelMatrix.identity();
	modelMatrix.Translate(-1.0f, -1.0f, 0.0f);
	program->setModelMatrix(modelMatrix);
	ut.DrawText(program, fontTexture, "1/B TO ATTACK", 0.2f, 0.0001f);


	modelMatrix.identity();
	modelMatrix.Translate(-3.5f, -2.0f, 0.0f);
	program->setModelMatrix(modelMatrix);
	ut.DrawText(program, fontTexture, "PRESS SPACE TO START. ESC TO EXIT", 0.2f, 0.0001f);

}

void UpdateMainMenu(float elapsed) {
	// does nothing really. We just have static text to worry about here.
}

void RenderGameLevel() {
	players[1].draw(program);
	players[0].draw(program);
	for (size_t i = 0; i < blocks.size(); i++) {
		blocks[i].draw(program);
	}
	viewMatrix.identity();
	viewMatrix.Translate(-players[0].position[0], -players[0].position[1], 0.0f);
	program->setViewMatrix(viewMatrix);
}

void UpdateGameLevel(float elapsed) {
	for (int i = 0; i < 4; i++) {
		players[0].collided[i] = false;
		players[1].collided[i] = false;
	}
	float penetration;

	// Update all Y's first
	players[0].updateY(elapsed);
	players[1].updateY(elapsed);

	for (int k = 0; k < players.size(); k++) {
		for (size_t i = 0; i < blocks.size(); i++) {
			if (players[k].boundaries[1] < blocks[i].boundaries[0] &&
				players[k].boundaries[0] > blocks[i].boundaries[1] &&
				players[k].boundaries[2] < blocks[i].boundaries[3] &&
				players[k].boundaries[3] > blocks[i].boundaries[2])
			{
				float y_distance = fabs(players[k].position[1] - blocks[i].position[1]);
				float playerHeightHalf = 0.05f * players[k].size[1] * 2;
				float blockHeightHalf = 0.05f * blocks[i].size[1] * 2;
				penetration = fabs(y_distance - playerHeightHalf - blockHeightHalf);

				if (players[k].position[1] > blocks[i].position[1]) {
					players[k].position[1] += penetration + 0.001f;
					players[k].boundaries[0] += penetration + 0.001f;
					players[k].boundaries[1] += penetration + 0.001f;
					players[k].collided[1] = true;
				}
				else {
					players[k].position[1] -= (penetration + 0.001f);
					players[k].boundaries[0] -= (penetration + 0.001f);
					players[k].boundaries[1] -= (penetration + 0.001f);
					players[k].collided[0] = true;
				}
				players[k].speed[1] = 0.0f;
				break;
			}
		}
	}
	
	// Update all X's next
	players[0].updateX(elapsed);
	players[1].updateX(elapsed);
	for (int k = 0; k < players.size(); k++) {
		Entity player = players[k];
		for (size_t i = 0; i < blocks.size(); i++) {
			if (players[k].boundaries[1] < blocks[i].boundaries[0] &&
				players[k].boundaries[0] > blocks[i].boundaries[1] &&
				players[k].boundaries[2] < blocks[i].boundaries[3] &&
				players[k].boundaries[3] > blocks[i].boundaries[2])
			{
				float x_distance = fabs(players[k].position[0] - blocks[i].position[0]);
				float playerWidthHalf = 0.05f * players[k].size[0] * 2;
				float blockWidthHalf = 0.05f * blocks[i].size[0] * 2;
				penetration = fabs(x_distance - (playerWidthHalf + blockWidthHalf));

				if (players[k].position[0] > blocks[i].position[0]) {
					players[k].position[0] += penetration + 0.001f;
					players[k].boundaries[2] += penetration + 0.001f;
					players[k].boundaries[3] += penetration + 0.001f;
					players[k].collided[3] = true;
				}
				else {
					players[k].position[0] -= (penetration + 0.001f);
					players[k].boundaries[2] -= (penetration + 0.001f);
					players[k].boundaries[3] -= (penetration + 0.001f);
					players[k].collided[2] = true;
				}
				players[k].speed[0] = 0.0f;
				break;
			}
		}
	}

	players[0].speed[0] = 0.0f;
	players[1].speed[0] = 0.0f;
	
	// handle controls
	if (p1controlsMoveLeft)
		players[0].speed[0] = -playerSpeed;
	else if (p1controlsMoveRight)
		players[0].speed[0] = playerSpeed;
	if (p1controlsJump && players[0].collided[1])
		players[0].speed[1] = 5.0f;

	if (p2controlsMoveLeft)
		players[1].speed[0] = -playerSpeed;
	else if (p2controlsMoveRight)
		players[1].speed[0] = playerSpeed;
	if (p2controlsJump && players[1].collided[1])
		players[1].speed[1] = 5.0f;
}

void Render() {
	glClear(GL_COLOR_BUFFER_BIT);
	switch (state) {
	case STATE_MAIN_MENU:
		RenderMainMenu();
		break;
	case STATE_GAME_LEVEL:
		RenderGameLevel();
		break;
	}
	SDL_GL_SwapWindow(displayWindow);
}

void Update(float elapsed) {
	switch (state) {
	case STATE_MAIN_MENU:
		UpdateMainMenu(elapsed);
		break;
	case STATE_GAME_LEVEL:
		UpdateGameLevel(elapsed);
		break;
	}
}



// MAIN FUNCTION. SETUP____________________________________________________________________________________________________________________________
int main(int argc, char *argv[])
{
	srand(time(NULL));
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Brian Chuk's Basic Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1600, 900, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	SDL_Event event;
	bool done = false;

	projectionMatrix.setOrthoProjection(-4.0, 4.0, -2.25f, 2.25f, -1.0f, 1.0f);
	program->setModelMatrix(modelMatrix);
	program->setProjectionMatrix(projectionMatrix);
	program->setViewMatrix(viewMatrix);

	//Create GLUint textures
	fontTexture = ut.LoadTexture("font1.png");
	playerSpriteTexture = ut.LoadTexture("p1_jump.png");
	groundTexture = ut.LoadTexture("castleCenter.png");
	powerupTexture = ut.LoadTexture("cherry.png");

	//Initialize entities
	players.push_back(Entity(0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0, 0, playerSpriteTexture, 1.0f, 1.4f, PLAYER));
	players.push_back(Entity(0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0, 0, playerSpriteTexture, 1.0f, 1.4f, PLAYER));
	players[0].isStatic = false;
	players[0].acceleration[1] = -0.01f;
	players[1].isStatic = false;
	players[1].acceleration[1] = -0.01f;
	
	for (int i = 0; i < 55; i++) {
		blocks.push_back(Entity(-2.5f + (i) * 0.2f, 0.0f - (4 * 0.5f), 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, groundTexture, BLOCK));
	}

	for (int i = 0; i < 4; i++) {
		blocks.push_back(Entity(-2.0f + (i)* 0.2f, 0.0f - (1 * 0.5f), 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, groundTexture, BLOCK));
	}

	for (int i = 0; i < 4; i++) {
		blocks.push_back(Entity(1.0f + (i)* 0.2f, -1.8f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, groundTexture, BLOCK));
	}

	while (!done) {
		// Keyboard Controls
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE || event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				done = true;
			switch (event.type) {
				case SDL_KEYDOWN:
					if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						//firing, starting the game
						if (state == STATE_MAIN_MENU)
							state = STATE_GAME_LEVEL;
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_KP_1) {
						// Neutral Attack
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_KP_2) {
						// Strong Attack
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_KP_3) {
						// Up Attack
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
						p1controlsJump = true;
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_LEFT && players[0].boundaries[2] > -3.5f) {
						p1controlsMoveLeft = true;
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT && players[0].boundaries[3] < 3.5f) {
						p1controlsMoveRight = true;
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_W) {
						p2controlsJump = true;
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_A && players[0].boundaries[2] > -3.5f) {
						p2controlsMoveLeft = true;
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_D && players[0].boundaries[3] < 3.5f) {
						p2controlsMoveRight = true;
					}
					break;
				case SDL_KEYUP:
					if (event.key.keysym.scancode == SDL_SCANCODE_KP_1) {
						// Neutral Attack
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_KP_2) {
						// Strong Attack
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_KP_3) {
						// Up Attack
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
						p1controlsMoveLeft = false;
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
						p1controlsMoveRight = false;
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
						p1controlsJump = false;
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_A) {
						p2controlsMoveLeft = false;
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_D) {
						p2controlsMoveRight = false;
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_W) {
						p2controlsJump = false;
					}
					break;
			}
		}

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		if (gameRunning) {
			float fixedElapsed = elapsed;
			if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
				fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
			}
			while (fixedElapsed >= FIXED_TIMESTEP) {
				fixedElapsed -= FIXED_TIMESTEP;
				Update(FIXED_TIMESTEP);
			}
			Update(fixedElapsed);
			Render();
		}
	}

	SDL_Quit();
	return 0;
}
