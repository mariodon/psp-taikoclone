#include "animation.h"

// step can be positive or negative
frame_t *animate(anime_control_t *data, float step)
{
	if (data == NULL) {
		return;
	}
	
	float time = data->time + step;
	float framef = data->framerate * time;
	int framei = int(framef);
	frame_t *ret = NULL;
	
	
	if (data->enables & ANIME_TYPE_IMAGE) {
		int img_frame =
	}
	
	if (data->enables & ANIME_TYPE_POS) {
	}
	if (data->enables & ANIME_TYPE_SCALE) {
	}
	if (data->enables & ANIME_TYPE_PALETTE) {
	
	}
}