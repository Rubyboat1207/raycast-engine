#include <SDL.h>
#include "libs.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const Uint8* state;

const char map[5][8] = { 
	{1,1,1,1,1,1,1,1},
	{4,0,0,0,0,0,0,2},
	{4,0,0,0,0,0,0,2},
	{4,0,0,0,3,3,3,3},
	{1,1,1,1,1,0,0,0},
};

struct Player {
	float posX;
	float posY;
	float rot;
};
typedef struct Player Player;
void runGame(Player* player, SDL_Surface* surface);

int main(int argc, char* args[])
{
	SDL_Window* window = NULL;
	SDL_Surface* screenSurface = NULL;

	Player* player = (Player*)malloc(sizeof(Player));

	if (player == NULL) {
		return 2;
	}

	player->posX = 2;
	player->posY = 2;
	player->rot = 0;

	state = SDL_GetKeyboardState(NULL);

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}
	else
	{
		//Create window
		window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		}
		else
		{
			//Get window surface
			screenSurface = SDL_GetWindowSurface(window);

			//Fill the surface white
			//SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));

			//Update the surface
			SDL_UpdateWindowSurface(window);

			//Hack to get window to stay up
			SDL_Event e;
			bool quit = false;
			while (quit == false) {
				while (SDL_PollEvent(&e)) {
					if (e.type == SDL_QUIT)
						quit = true;
				}
				runGame(player, screenSurface);
				SDL_UpdateWindowSurface(window);
				if (state[SDL_SCANCODE_Q]) {
					quit = true;
				}
			}
		}
	}

	//Destroy window
	SDL_DestroyWindow(window);

	//Quit SDL subsystems
	SDL_Quit();

	free(player);

	return 0;
}

struct RayHit {
	float distance;
	char color;
};
typedef struct RayHit RayHit;

void set_pixel(SDL_Surface* surface, int x, int y, Uint32 pixel)
{
	Uint32* const target_pixel = (Uint32*)((Uint8*)surface->pixels
		+ y * surface->pitch
		+ x * surface->format->BytesPerPixel);
	*target_pixel = pixel;
}

RayHit* raycast(float start_x, float start_y, float angle) {
	float xoff = cos(angle);
	float yoff = sin(angle);

	float x = start_x;
	float y = start_y;
	RayHit* hit = (RayHit*)malloc(sizeof(RayHit));
	if (hit == NULL) {
		exit(2);
	}
	for (int i = 0; i < 500; i++) {
		auto item = map[(int)round(x)][(int)round(y)];
		if (item != 0) {
			float xdist = x - start_x;
			float ydist = y - start_y;
			



			hit->color = item;
			hit->distance = sqrt((xdist * xdist) + (ydist * ydist));


			return hit;
		}

		x += xoff * 0.02;
		y += yoff * 0.02;
	}
	hit->color = 0;
	hit->distance = 900;

	return hit;
}

float map_value(float val, float in_min, float in_max, float out_min, float out_max) {
	return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float wall_scale_factor = 13;
float distance_scale_factor = 18;
#define center_y SCREEN_HEIGHT / 2
#define qdegrads M_PI * 0.5
#define move_speed 0.04
#define rotation_speed 2

void runGame(Player* player, SDL_Surface* surface) {
	float fov = 30;

	if (state[SDL_SCANCODE_LEFT]) {
		player->rot -= (M_PI / 180) * rotation_speed;
	}
	if (state[SDL_SCANCODE_RIGHT]) {
		player->rot += (M_PI / 180) * rotation_speed;
	}

	if (state[SDL_SCANCODE_W]) {
		player->posX += cos(player->rot) * move_speed;
		player->posY += sin(player->rot) * move_speed;
	}
	if (state[SDL_SCANCODE_S]) {
		player->posX -= cos(player->rot) * move_speed;
		player->posY -= sin(player->rot) * move_speed;
	}

	if (state[SDL_SCANCODE_D]) {
		player->posX += cos(player->rot + qdegrads) * move_speed;
		player->posY += sin(player->rot + qdegrads) * move_speed;
	}
	if (state[SDL_SCANCODE_A]) {
		player->posX -= cos(player->rot + qdegrads) * move_speed;
		player->posY -= sin(player->rot + qdegrads) * move_speed;
	}

	for (int x = 0; x < SCREEN_WIDTH; x++) {
		float fov_position = (((float) x / SCREEN_WIDTH) * fov) - fov/2;
		float rot = player->rot + fov_position * (M_PI / 180);
		RayHit* hit = raycast(player->posX, player->posY, rot);
		//printf("distance: %f | angle: %f\n", distance, rot * (180 / M_PI));
		for (int y = 0; y < SCREEN_HEIGHT; y++) {
			Uint32 color = 0xffffff;
			if (abs(center_y - y) < (distance_scale_factor / hit->distance) * wall_scale_factor) {
				switch (hit->color) {
				case 1: color = 0x0000ff; break;
				case 2: color = 0xff0000; break;
				case 3: color = 0x00ff00; break;
				case 4: color = 0xff00ff; break;
				default: color = 0xfffff;
				}
			}
			int r = (color >> 24) & 0xFF;
			int g = (color >> 16) & 0xFF;
			int b = (color >> 8) & 0xFF;

			color = (r << 24) | (g << 16) | (b << 8) | (color & 0xFF);
			
			set_pixel(surface, x, y, color);
		}
		free(hit);
	}
}