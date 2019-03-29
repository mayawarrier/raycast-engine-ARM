
#include "raycast.h"
#include "Map_Data.h"

// filled in by main
volatile int MAP_DATA[MAP_SIZE_X][MAP_SIZE_Y];

#define PROJECTION_FACTOR 4500
#define HALF_FOV FOV / 2.0

// emits a ray from first intersection with the grid, and traces it until it hits either a wall or goes out of bounds
// if the ray goes out of bounds, return point(INT_MAX, INT_MAX)
// else return the unit coordinates of the location where wall was found
point emit_and_trace_ray(int first_inter_x, int first_inter_y, int inter_offset_x, int inter_offset_y);

double inline reverse_fishbowl(double polar_distance);
point inline make_point(int x, int y);
slice_info* make_slice_info(int size, int location);
grid_point convert_to_grid_point(int unit_x, int unit_y);
bool outside_map_bounds(int unit_x, int unit_y);

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
	ALPHA = player_angle - screen_column_angle + HALF_FOV;

	// BETA is angle between the casted ray and the player angle (center of FOV)
	BETA = screen_column_angle - HALF_FOV;

	// wrap around ALPHA to keep it within the bounds of 0 - 360
	if (ALPHA < 0.0) ALPHA += 360.0;
	else if (ALPHA > 360.0) ALPHA -= 360.0;
	
	point horizontal_intersection = find_closest_horizontal_wall_intersection(playerX, playerY);
	point vertical_intersection = find_closest_vertical_wall_intersection(playerX, playerY);

	double closest_distance = find_closest_distance_to_wall(playerX, &horizontal_intersection, &vertical_intersection);
	
	if (closest_distance == 0) {
		// no wall intersections were found at this ray
		return make_slice_info(INT_MAX, INT_MAX);
	} else {
		// reverse fishbowl the distance and apply the projection factor to find the slice size
		int slice_size = PROJECTION_FACTOR / (reverse_fishbowl(closest_distance));
		// limit slice size to the maximum value for this resolution, if it is bigger than the screen
		if (slice_size > SCREEN_SIZE_Y) slice_size = SCREEN_SIZE_Y;
		// return a slice info. location of slice is from the top of the screen
		return make_slice_info(slice_size, (SCREEN_SIZE_Y - slice_size) / 2);
	}
}

point find_closest_horizontal_wall_intersection(int playerX, int playerY) {

	// first_inter x and y are the (x, y) unit coords of the first intersection with the grid
	// inter_offset x and y are (x, y) offsets to get from the current intersection to the next intersection with the grid
	// current_inter x and y are the (x, y) unit coords of the current intersection of the ray with the grid (i.e. at
	// this moment in the ray travel)
	int first_inter_x, first_inter_y, inter_offset_x, inter_offset_y;
	
	// --------------------------------- compute first intersection with the grid and offset -----------------------

	if (ALPHA >= 0 && ALPHA < 180) {
		// ray facing up
		first_inter_y = ((playerY >> 6) << 6) - 1; // subtract 1 to make A part of the grid block above the grid line
		// move the ray upwards by 64 unit coords when the ray is facing upwards
		inter_offset_y = -64;
	} else if (ALPHA >= 180 && ALPHA < 360) {
		// ray facing down
		first_inter_y = ((playerY >> 6) << 6) + 64; // add 64 to make first_inter_y the y position of the next grid block
		// move the ray downwards by 64 unit coords when the ray is facing downwards
		inter_offset_y = 64;
	}

	// pre-compute tan(alpha) for speed
	double tan_alpha = tand(ALPHA);

	// calculate first_inter_x using line formula
	first_inter_x = playerX + (playerY - first_inter_y) / tan_alpha;
	// calculate projection of inter_offset_y on x axis. -ve because Y axis is flipped
	inter_offset_x = -inter_offset_y / tan_alpha;

	// ---------------------------- emit and trace the ray from first intersection outwards -----------------------

	//  offsets are used to move the head of the ray forward, until ray hits a wall or goes out of bounds
	return emit_and_trace_ray(first_inter_x, first_inter_y, inter_offset_x, inter_offset_y);
}

point find_closest_vertical_wall_intersection(int playerX, int playerY) {

	int first_inter_x, first_inter_y, inter_offset_x, inter_offset_y;

	if (ALPHA >= 90 && ALPHA < 270) {
		first_inter_x = ((playerX >> 6) << 6) - 1;
		inter_offset_x = -64;
	} else if ((ALPHA >= 270 && ALPHA < 360) || (ALPHA >= 0 && ALPHA < 90)) {
		first_inter_x = ((playerX >> 6) << 6) + 64;
		inter_offset_x = 64;
	}
	
	double tan_alpha = tand(ALPHA);

	first_inter_y = playerY + (playerX - first_inter_x) / tan_alpha;		
	inter_offset_y = -inter_offset_x / tan_alpha;
	
	// ---------------------------- emit and trace the ray from first intersection outwards -----------------------

	//  offsets are used to move the head of the ray forward, until ray hits a wall or goes out of bounds
	return emit_and_trace_ray(first_inter_x, first_inter_y, inter_offset_x, inter_offset_y);
}

point emit_and_trace_ray(int first_inter_x, int first_inter_y, int inter_offset_x, int inter_offset_y) {

	// the ray starts at the first intersection
	int current_inter_x = first_inter_x;
	int current_inter_y = first_inter_y;

	// ------------------------------------- emit the ray ------------------------------------------

	// constantly calculate new intersections with the grid, and check if a wall exists there
	bool reached_map_bounds = false;

	// if either a wall exists or map bounds are reached, break out!
	while (true) {

		// find the grid location where these unit coordinates lie
		grid_point current_inter_grid_point = convert_to_grid_point(current_inter_x, current_inter_y);

		// -------------------- check whether or not to break out of ray casting --------------

		// check if a wall exists at this grid location
		if (MAP_DATA[current_inter_grid_point.x][current_inter_grid_point.y] == 1) {
			// we've reached a wall, these are our wall unit coordinates
			// break to prevent moving ray further
			break;
		}
		else if (outside_map_bounds(current_inter_x, current_inter_y)) {
			// we've reached map bounds without finding a wall
			// break to exit to prevent ray moving further out of bounds
			reached_map_bounds = true;
			break;
		}

		// -------------------- move ray further --------------

		// increment current_inter x and y with the offsets to trace the ray further since
		// we haven't reached a wall or map bounds
		current_inter_x += inter_offset_x;
		current_inter_y += inter_offset_y;
	}

	// if map bounds reached, error! else return current location of the ray
	if (reached_map_bounds) {
		return make_point(INT_MAX, INT_MAX);
	}
	else {
		return make_point(current_inter_x, current_inter_y);
	}
}

// if no wall exists at this ray, returns 0
double find_closest_distance_to_wall(int playerX, point* horiz_intersection, point* vert_intersection) {

	// calculate distances to the horizontal and vertical intersections
	double distance_horiz = (playerX - horiz_intersection->x) / cosd(ALPHA);
	double distance_vert = (playerX - vert_intersection->x) / cosd(ALPHA);

	// if the point is (INT_MAX, INT_MAX), no intersection was found

	if (horiz_intersection->x == INT_MAX && vert_intersection->x != INT_MAX) {
		// no horizontal intersection found but vert found, so closest distance is distance_vert
		return distance_vert;
	} else if (vert_intersection->x == INT_MAX && horiz_intersection->x != INT_MAX) {
		// no vertical intersection found but horiz found, so closest distance is distance_horiz
		return distance_horiz;
	} else if (vert_intersection->x != INT_MAX && horiz_intersection->x != INT_MAX) {
		// both intersections were found
		return (distance_horiz > distance_vert) ? distance_vert : distance_horiz;
	} else {
		// no intersections were found
		return 0;
	}
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

grid_point convert_to_grid_point(int unit_x, int unit_y) {
	grid_point pt;
	pt.x = unit_x >> 6;
	pt.y = unit_y >> 6;
	return pt;
}

bool outside_map_bounds(int unit_x, int unit_y) {
	// each grid location is 64 unit coordinates in size
	// check if unit coords are outside (0, MAP_SIZE_X * 64) and (0, MAP_SIZE_Y * 64)
	return (unit_x > MAP_SIZE_X << 6 || unit_x < 0 || unit_y > MAP_SIZE_Y << 6 || unit_y < 0);
}