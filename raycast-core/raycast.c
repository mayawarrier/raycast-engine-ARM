
#include "raycast.h"

#define PROJECTION_FACTOR 2000
#define RAY_ANGLE_INC FOV/SCREEN_SIZE_X;
#define HALF_FOV FOV / 2.0;

double inline reverse_fishbowl(double polar_distance);
int inline find_projected_wall_size(double distance_to_wall);

/* alpha is the current angle at which a ray is being cast.To get it, we
shift to the left of the FOV from the player angle(angle + FOV / 2) and then
subtract in increments of RAY_ANGLE_INC degrees till we span the entire FOV. */
double ALPHA;

/* beta is the angle of the ray relative to player angle, used to reverse
the fishbowl effect */
double BETA;

slice_info cast_ray(int playerX, int playerY, double player_angle, int screen_column) {
	
}

point find_closest_horizontal_wall_intersection() {
	
}

point find_closest_vertical_wall_intersection() {
	
}

double find_closest_distance_to_wall() {
	
}

double inline reverse_fishbowl(double polar_distance) {
	return polar_distance * cos(BETA);
}

int inline find_projected_wall_size(double distance_to_wall) {
	return PROJECTION_FACTOR / distance_to_wall;
}