
#ifndef RAYCAST_H
#define RAYCAST_H

#include <math.h>
#include <stdbool.h>
#include <limits.h>

#define SCREEN_SIZE_X 320
#define SCREEN_SIZE_Y 240
#define FOV 60.0
#define RAY_ANGLE_INC (FOV/SCREEN_SIZE_X);

#define sind(x) (sin((x) * M_PI / 180))
#define cosd(x) (cos((x) * M_PI / 180))
#define tand(x) (tan((x) * M_PI / 180))

// if point is out of map bounds, x = y = INT_MAX
typedef struct point_unit_coords {
	int x;
	int y;
} point;

// if grid point is out of map bounds, x = y = INT_MAX
typedef struct point_grid_coords {
	int x;
	int y;
} grid_point;

// if slice does not exist at this location, size = location = INT_MAX
typedef struct slice_info {
	int size;
	int location;
	// additional texture info can be inserted here later
} slice_info;

point find_closest_horizontal_wall_intersection(int playerX, int playerY, double ray_angle);

point find_closest_vertical_wall_intersection(int playerX, int playerY, double ray_angle);

// if no wall exists at this ray, returns NAN
double find_closest_distance_to_wall(point* horiz_intersection, point* vert_intersection);

slice_info* cast_ray(int playerX, int playerY, double player_angle, int screen_column);

#endif // RAYCAST_H