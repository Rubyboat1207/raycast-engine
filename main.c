#include "libs.h"
#include "vec.h"
#include "walls.h"
const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

#define clamp(v, i, a) max(min(v, a), i)

const Uint8* state;

#define WALLS_COUNT 8
const Wall* walls[WALLS_COUNT];

struct Player {
	float posX;
	float posY;
	float rot;
};
typedef struct Player Player;
void runGame(Player* player, SDL_Surface* surface);

void init_walls() {
	// wall 1
	walls[0] = new_wall(point_of(-4, 4), point_of(-6, 0), 0xFF0000);
	// wall 2
	walls[1] = new_wall(point_of(-6, 0), point_of(-4, -4), 0xFF0000);
	// wall 3
	walls[2] = new_wall(point_of(2, -4), point_of(2, 4), 0xFF0000); // not working properly
	// wall 4
	walls[3] = new_wall(point_of(-4, -4), point_of(2, -4), 0xFF0000);
	// wall 5
	walls[4] = new_wall(point_of(-4, 4), point_of(2, 4), 0x324def);
	
	// tri 1
	walls[5] = new_wall(point_of(-2, 0), point_of(0, -2), 0x00ff00);
	walls[6] = new_wall(point_of(0, -2), point_of(0, 2), 0x00ff00);
	walls[7] = new_wall(point_of(-2, 0), point_of(0, 2), 0x00ff00);

}

void free_walls() {
	for (int i = 0; i < WALLS_COUNT; i++) {
		free(walls[i]);
	}
}

int main(int argc, char* args[])
{
	SDL_Window* window = NULL;
	SDL_Surface* surface = NULL;
	Player* player = (Player*)malloc(sizeof(Player));

	if (player == NULL) {
		return 2;
	}

	player->posX = 2;
	player->posY = 2;
	player->rot = 0;

	init_walls();

	state = SDL_GetKeyboardState(NULL);
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL failed to initialize! SDL_Error: %s\n", SDL_GetError());
	}
	else
	{
		window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		}
		else
		{
			surface = SDL_GetWindowSurface(window);

			SDL_UpdateWindowSurface(window);

			SDL_Event e;
			bool quit = false;


			while (quit == false) {
				while (SDL_PollEvent(&e)) {
					if (e.type == SDL_QUIT)
						quit = true;
				}

				runGame(player, surface);
				SDL_UpdateWindowSurface(window);
				if (state[SDL_SCANCODE_Q]) {
					quit = true;
				}
			}
		}
	}

	//Clean up
	SDL_DestroyWindow(window);
	SDL_Quit();

	free(player);

	free_walls();

	return 0;
}

struct RayHit {
	float distance;
	Wall* wall;
};
typedef struct RayHit RayHit;

void set_pixel(SDL_Surface* surface, int x, int y, Uint32 pixel)
{
	Uint32* const target_pixel = (Uint32*)((Uint8*)surface->pixels
		+ y * surface->pitch
		+ x * surface->format->BytesPerPixel);
	*target_pixel = pixel;
}

#define RAYCAST_DISTANCE_MAX 255
RayHit* raycast(float start_x, float start_y, float angle) {
	Point start = (Point){ .x = start_x, .y = start_y };
	Point end = (Point){ .x = start_x + cos(angle) * RAYCAST_DISTANCE_MAX, .y = start_y + sin(angle) * RAYCAST_DISTANCE_MAX };

	RayHit* hit = (RayHit*)malloc(sizeof(RayHit));
	if (hit == NULL) {
		exit(5);
	}
	hit->wall = NULL;
	hit->distance = RAYCAST_DISTANCE_MAX;

	Wall* closest = NULL;
	float dist = RAYCAST_DISTANCE_MAX;

	for (int i = 0; i < WALLS_COUNT; i++) {
		Wall* wall = walls[i];
		

		Intersection intersection = get_intersection_ptr(start, end, wall->a, wall->b);
		if (intersection.intersects ) {
			float x_su = intersection.point->x - start_x;
			float y_su = intersection.point->y - start_y;
			float cur_dist = sqrt(x_su * x_su + y_su * y_su);
			if (cur_dist < dist) {
				closest = wall;
				dist = cur_dist;
			}
		}
		free(intersection.point);
	}

	if (closest != NULL) {
		hit->distance = dist;
		hit->wall = closest;
	}

	return hit;
}

float map_value(float val, float in_min, float in_max, float out_min, float out_max) {
	return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float wall_scale_factor = 13;
float distance_scale_factor = 18;
#define center_y WINDOW_HEIGHT / 2
#define qdegrads M_PI * 0.5
#define move_speed 0.04
#define rotation_speed 2
#define wall_vis_dist 5
#define ray_pixel_off 0.001


void runGame(Player* player, SDL_Surface* surface) {
	float fov = 60;

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

	for (int x = 0; x < WINDOW_WIDTH; x++) {
		float fov_position = (((float) x / WINDOW_WIDTH) * fov) - fov/2;
		float rot = player->rot + fov_position * (M_PI / 180);
		RayHit* hit = raycast(player->posX - (ray_pixel_off * x), player->posY, rot);

		for (int y = 0; y < WINDOW_HEIGHT; y++) {
			Uint32 color = 0xFFFFFF;

			if (y > center_y) {
				color = 0x000000;
			}
			else {
				color = 0x777777;
			}


			if (abs(center_y - y) < (distance_scale_factor / hit->distance) * wall_scale_factor && hit->wall != NULL) {
				color = hit->wall->color;
				
			}
			int r = (color >> 24) & 0xFF;
			int g = (color >> 16) & 0xFF;
			int b = (color >> 8) & 0xFF;
			float distance_multiplier = clamp(map_value(hit->distance, 0, wall_vis_dist, 1, 0), 0, 1);
			
			r *= distance_multiplier;
			g *= distance_multiplier;
			b *= distance_multiplier;


			color = (r << 24) | (g << 16) | (b << 8) | (color & 0xFF);
			
			set_pixel(surface, x, y, color);
		}
		free(hit);
	}
}