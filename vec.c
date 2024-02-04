#pragma once
#include "vec.h"


// from video https://www.youtube.com/watch?v=c065KoXooSw
Intersection get_intersection(Point a, Point b, Point c, Point d) {
	Point r = sub_points(b, a);
	Point s = sub_points(d, c);

	float e = r.x * s.y - r.y * s.x;
	float u = ((c.x - a.x) * r.y - (c.y - a.y) * r.x) / e;
	float t = ((c.x - a.x) * s.y - (c.y - a.y) * s.x) / e;

	if ((0 <= u && u <= 1) && (0 <= t && t <= 1)) {
		// an intersection is born
		Point* pt = (Point*)malloc(sizeof(Point));
		if (pt == NULL) {
			fprintf(stderr, "Memory allocation failed.\n");
			exit(2);
		}
		Point tr = mult_point(r, t);
		pt->x = a.x + tr.x;
		pt->y = a.y + tr.y;

		return (Intersection) { .point = pt, .intersects = true };
	}
	// sad face
	return (Intersection) { .point = NULL, .intersects = false };
}

Intersection get_intersection_ptr(Point a, Point b, Point* c, Point* d) {
	Point r = (Point){.x = b.x - a.x, .y = b.y - a.y};
	Point s = (Point){ .x = d->x - c->x, .y = d->y - c->y };

	float e = r.x * s.y - r.y * s.x;
	float u = ((c->x - a.x) * r.y - (c->y - a.y) * r.x) / e;
	float t = ((c->x - a.x) * s.y - (c->y - a.y) * s.x) / e;
	Point* pt = (Point*)malloc(sizeof(Point));
	if (pt == NULL) {
		fprintf(stderr, "Memory allocation failed.\n");
		exit(2);
	}

	if ((0 <= u && u <= 1) && (0 <= t && t <= 1)) {
		// an intersection is born
		
		Point tr = mult_point(r, t);
		pt->x = a.x + tr.x;
		pt->y = a.y + tr.y;

		return (Intersection) { .point = pt, .intersects = true };
	}
	// sad face
	pt->x = u;
	pt->y = t;
	return (Intersection) { .point = pt, .intersects = false };
}

Point* point_of(float x, float y) {
	Point* ptr = (Point*)malloc(sizeof(Point));
	if (ptr == NULL) {
		exit(3);
	}


	ptr->x = x;
	ptr->y = y;

	return ptr;
}

/*int main(int argc, char* argv[]) {
	Intersection i = get_intersection(
		(Point) {
		.x = 5, .y = 1
	},
		(Point) {
		.x = -1, .y = -1
	},
		(Point) {
		.x = -1, .y = 1
	},
		(Point) {
		.x = 1, .y = -1
	}
	);

	if (i.intersects) {
		printf("Intersection is at: (%f, %f)", i.point->x, i.point->y);
	}
	else {
		printf("No intersection was found.");
	}

}*/