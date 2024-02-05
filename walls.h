#include "libs.h"
#include "vec.h"

struct Wall {
	Point* a;
	Point* b;

	Uint32 color;

	bool portal;
	float temp_height;
	float temp_posY;
};
typedef struct Wall Wall;

inline Wall* new_wall(Point* a, Point* b, Uint32 color, bool isPortal, float posY) {
	Wall* ptr = (Wall*)malloc(sizeof(Wall));

	if (ptr == NULL) {
		exit(4);
	}

	ptr->a = a;
	ptr->b = b;
	ptr->color = color;
	ptr->temp_height = 1;
	ptr->temp_posY = posY;
	ptr->portal = isPortal;

	return ptr;
}


struct Sector {
	Point* points;
	int point_count;
	float height;
};
typedef struct Sector Sector;