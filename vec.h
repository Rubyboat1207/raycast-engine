#pragma once
#include "libs.h"

#define sub_points(a, b) { .x = a.x - b.x, .y = a.y - b.y }
#define mult_point(a, scalar) { .x = a.x * scalar, .y = a.y * scalar}
#define cross_2d(a, b) a.x * b.y - a.y * b.x

struct Point {
	float x;
	float y;
};
typedef struct Point Point;

Point* point_of(float x, float y);


struct Intersection {
	Point* point;
	bool intersects;
};
typedef struct Intersection Intersection;

Intersection get_intersection(Point a, Point b, Point c, Point d);
Intersection get_intersection_ptr(Point a, Point b, Point* c, Point* d);