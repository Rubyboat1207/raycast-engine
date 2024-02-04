#include "libs.h"
#include "vec.h"

struct Wall {
	Point* a;
	Point* b;
	Uint32 color;
};
typedef struct Wall Wall;

inline Wall* new_wall(Point* a, Point* b, Uint32 color){
	Wall* ptr = (Wall*)malloc(sizeof(Wall));
	
	if (ptr == NULL) {
		exit(4);
	}

	ptr->a = a;
	ptr->b = b;
	ptr->color = color;

	return ptr;
}