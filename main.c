#include "libs.h"
#include "vec.h"
#include "walls.h"
const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

#define clamp(v, i, a) max(min(v, a), i)

const Uint8* state;

#define WALLS_COUNT 8
const Wall* walls[WALLS_COUNT];
const Sector* sectors[1];

struct Player {
	float posX;
	float posY;
	float posZ;
	float rot;
};
typedef struct Player Player;
void runGame(Player* player, SDL_Surface* surface);

void init_walls() {
	// wall 1
	walls[0] = new_wall(point_of(-4, 4), point_of(-6, 0), 0xFF0000, false, 0);
	// wall 2
	walls[1] = new_wall(point_of(-6, 0), point_of(-4, -4), 0xFF0000, false, 0);
	// wall 3
	walls[2] = new_wall(point_of(2, -4), point_of(2, 4), 0xFF0000, false, 0); // not working properly
	// wall 4
	walls[3] = new_wall(point_of(-4, -4), point_of(2, -4), 0xFF0000, false, 0);
	// wall 5
	walls[4] = new_wall(point_of(-4, 4), point_of(2, 4), 0x324def, false, 0);
	
	// tri 1
	walls[5] = new_wall(point_of(-2, 0), point_of(0, -2), 0x00ff00, true, -50);
	walls[6] = new_wall(point_of(0, -2), point_of(0, 2), 0x00ff99, true, -50);
	walls[7] = new_wall(point_of(-2, 0), point_of(0, 2), 0x00ff00, true, -50);

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
	player->posZ = 0;
	player->rot = 0;

	init_walls();

	state = SDL_GetKeyboardState(NULL);
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL failed to initialize! SDL_Error: %s\n", SDL_GetError());
	}
	else
	{
		window = SDL_CreateWindow("CWolf", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
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

			Uint32 lastTime = SDL_GetTicks(), currentTime;

			while (quit == false) {
				while (SDL_PollEvent(&e)) {
					if (e.type == SDL_QUIT)
						quit = true;
				}
				currentTime = SDL_GetTicks();
				Uint32 deltaTime = currentTime - lastTime;

				runGame(player, surface, deltaTime);
				SDL_UpdateWindowSurface(window);
				if (state[SDL_SCANCODE_Q]) {
					quit = true;
				}
				lastTime = currentTime;
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

struct RayHits {
	RayHit* hits;
	int hitCount;
};
typedef struct RayHits RayHits;

void set_pixel(SDL_Surface* surface, int x, int y, Uint32 pixel)
{
	Uint32* const target_pixel = (Uint32*)((Uint8*)surface->pixels
		+ y * surface->pitch
		+ x * surface->format->BytesPerPixel);
	*target_pixel = pixel;
}

int compareRayHits(const void* a, const void* b) {
	RayHit* hitA = (RayHit*)a;
	RayHit* hitB = (RayHit*)b;
	if (hitA->distance < hitB->distance) return 1;
	if (hitA->distance > hitB->distance) return -1;
	return 0;
}

#define RAYCAST_DISTANCE_MAX 255
RayHits raycast(float start_x, float start_y, float angle) {
	Point start = (Point){ .x = start_x, .y = start_y };
	Point end = (Point){ .x = start_x + cos(angle) * RAYCAST_DISTANCE_MAX, .y = start_y + sin(angle) * RAYCAST_DISTANCE_MAX };



	RayHit* hits = (RayHit*)malloc(sizeof(RayHit) * WALLS_COUNT);
	int ray_hits = 0;
	if (hits == NULL) {
		exit(5);
	}

	for (int i = 0; i < WALLS_COUNT; i++) {
		Wall* wall = walls[i];

		Intersection intersection = get_intersection_ptr(start, end, wall->a, wall->b);
		if (intersection.intersects) {
			float x_su = intersection.point->x - start_x;
			float y_su = intersection.point->y - start_y;
			float dist = sqrt(x_su * x_su + y_su * y_su);

			hits[ray_hits].wall = wall;
			hits[ray_hits].distance = dist;

			ray_hits++;
		}
		if (intersection.point != NULL) {
			free(intersection.point);
		}
		
	}

	

	qsort(hits, ray_hits, sizeof(RayHit), compareRayHits);

	return (RayHits) {.hits = hits, .hitCount = ray_hits};
}




float map_value(float val, float in_min, float in_max, float out_min, float out_max) {
	return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float wall_scale_factor = 13;
float distance_scale_factor = 18;
#define center_y WINDOW_HEIGHT / 2
#define qdegrads M_PI * 0.5
#define move_speed 5
#define rotation_speed 45
#define wall_vis_dist 10
#define ray_pixel_off 0.001

void render(Player* player, SDL_Surface* surface);

void runGame(Player* player, SDL_Surface* surface, Uint32 deltaTicks) {
	float delta_seconds = (float) deltaTicks / 1000;
	//printf("%f\n", delta_seconds);

	if (state[SDL_SCANCODE_LEFT]) {
		player->rot -= (M_PI / 180) * rotation_speed * delta_seconds;
	}
	if (state[SDL_SCANCODE_RIGHT]) {
		player->rot += (M_PI / 180) * rotation_speed * delta_seconds;
	}

	if (state[SDL_SCANCODE_W]) {
		player->posX += cos(player->rot) * move_speed * delta_seconds;
		player->posY += sin(player->rot) * move_speed * delta_seconds;
	}
	if (state[SDL_SCANCODE_S]) {
		player->posX -= cos(player->rot) * move_speed * delta_seconds;
		player->posY -= sin(player->rot) * move_speed * delta_seconds;
	}

	if (state[SDL_SCANCODE_SPACE]) {
		player->posZ += 1 * delta_seconds;
	}
	if (state[SDL_SCANCODE_LSHIFT]) {
		player->posZ -= 1 * delta_seconds;
	}

	if (state[SDL_SCANCODE_D]) {
		player->posX += cos(player->rot + qdegrads) * move_speed * delta_seconds;
		player->posY += sin(player->rot + qdegrads) * move_speed * delta_seconds;
	}
	if (state[SDL_SCANCODE_A]) {
		player->posX -= cos(player->rot + qdegrads) * move_speed * delta_seconds;
		player->posY -= sin(player->rot + qdegrads) * move_speed * delta_seconds;
	}

	render(player, surface);
}

void render(Player* player, SDL_Surface* surface) {
	float fov = 60;
	float fov_rad = fov * M_PI / 180;
	for (int x = 0; x < WINDOW_WIDTH; x++) {
		float fov_position = (((float)x / WINDOW_WIDTH) * fov) - fov / 2;

		float rot = player->rot + fov_position * (M_PI / 180);
		RayHits hits = raycast(player->posX, player->posY, rot);

		for (int y = 0; y < WINDOW_HEIGHT; y++) {
			set_pixel(surface, x, y, 0x0000000);
		}

		for (int h = 0; h < hits.hitCount; h++) {
			RayHit hit = hits.hits[h];

			if (hit.wall != NULL) {
				Uint32 wall_color = hit.wall->color;

				Uint32 color = wall_color;

				bool drawing_surface = false;

				float correctedDistance = hit.distance * cos(rot - player->rot);
				float screenDistance = WINDOW_WIDTH / (2 * tan(fov_rad / 2));

				float slice_height = (hit.wall->temp_height * screenDistance) / correctedDistance;
				float start_y = (WINDOW_HEIGHT / 2) - (slice_height / 2) + hit.wall->temp_posY;
				float end_y = slice_height + start_y;
				float distance_multiplier = clamp(map_value(correctedDistance, 0, wall_vis_dist, 1, 0), 0, 1);

				int r = ((color >> 16) & 0xFF) * distance_multiplier;
				int g = ((color >> 8) & 0xFF) * distance_multiplier;
				int b = (color & 0xFF) * distance_multiplier;
				color = SDL_MapRGB(surface->format, r, g, b);

				for (int y = 0; y < WINDOW_HEIGHT; y++) {

					if (!hit.wall->portal) {
						if (y < start_y || y > end_y) {
							continue;
						}

						set_pixel(surface, x, y, color);
					}
					else {
						if (y < start_y || y > end_y) {
							set_pixel(surface, x, y, color);
						}
					}
				}

				//if (!hit.wall->portal) {
				//	break;
				//}
			}
			else {
				for (int y = 0; y < WINDOW_HEIGHT; y++) {
					set_pixel(surface, x, y, 0xFFFFFF);
				}
			}
		}

		free(hits.hits);
	}
}