
#include "raycast.h"

#define PROJECTION_FACTOR 2000
#define HALF_FOV FOV / 2.0;

double inline reverse_fishbowl(double polar_distance);
point inline make_point(int x, int y);
slice_info* make_slice_info(int size, int location);

/* ALPHA is the current angle at which a ray is being cast.To get it, we
shift to the left of the FOV from the player angle (angle + FOV / 2) and then
subtract in increments of RAY_ANGLE_INC degrees till we span the entire FOV. 
ALPHA varies from 0 - 360. */
double ALPHA;

/* BETA is the angle of the ray relative to player angle, used to reverse
the fishbowl effect */
double BETA;

slice_info* cast_ray(int playerX, int playerY, double player_angle, int screen_column) {

	// screen_column_angle is the angle from the left of the FOV to the casted ray (at this screen column)
	double screen_column_angle = screen_column * RAY_ANGLE_INC;
	
	// move to the left of the FOV then subtract the offset to compute angle at this screen column
	ALPHA = player_angle + HALF_FOV - screen_column_angle;

	// BETA is angle between the casted ray and the player angle (center of FOV)
	BETA = screen_column_angle - HALF_FOV;

	// wrap around ALPHA to keep it within the bounds of 0 - 360
	if (ALPHA < 0.0) ALPHA += 360.0;
	else if (ALPHA > 360.0) ALPHA -= 360.0;

	point horizontal_intersection = find_closest_horizontal_wall_intersection(playerX, playerY, ALPHA);
	point vertical_intersection = find_closest_vertical_wall_intersection(playerX, playerY, ALPHA);

	double closest_distance = find_closest_distance_to_wall(&horizontal_intersection, &vertical_intersection);

	if (isnan(closest_distance)) {
		return make_slice_info(INT_MAX, INT_MAX);
	} else {
		int slice_size = PROJECTION_FACTOR / closest_distance;
		// limit slice size to the maximum value for this resolution, if it is bigger than the screen
		if (slice_size > SCREEN_SIZE_Y) slice_size = SCREEN_SIZE_Y;
		// return a slice info. location of slice is from the top of the screen
		return make_slice_info(slice_size, (SCREEN_SIZE_Y - slice_size) / 2);
	}
}

point find_closest_horizontal_wall_intersection(int playerX, int playerY, double ray_angle) {
	
}

point find_closest_vertical_wall_intersection(int playerX, int playerY, double ray_angle) {
	
}

double find_closest_distance_to_wall(point* horiz_intersection, point* vert_intersection) {
	
}

double inline reverse_fishbowl(double polar_distance) {
	return polar_distance * cosd(BETA);
}

point inline make_point(int x, int y) {
	point pt;
	pt.x = x;
	pt.y = y;
	return pt;
}

slice_info* make_slice_info(int size, int location) {
	slice_info* slice_info_ptr = malloc(sizeof(slice_info));
	slice_info_ptr->size = size;
	slice_info_ptr->location = location;
	return slice_info_ptr;
}