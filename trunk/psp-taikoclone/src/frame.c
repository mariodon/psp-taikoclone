#include "frame.h"

frame_t *frame_create(OSL_IMAGE *osl_img)
{
	frame_t *frame = NULL;
	
	frame = (frame_t *)malloc(sizeof(frame_t));
	frame->osl_img = osl_img;
	
	frame->x = 0;
	frame->y = 0;
	
	frame->center_x = 0;
	frame->center_y = 0;
	
	frame->scale_x = 1.0;
	frame->scale_y = 1.0;
	
	frame->alpha = 1.0;
	/* default to osl_img->palette if NULL */
	frame->palette = NULL;
}

/* draw a frame, with the status applied.
 * TODO: add plane alpha support.
 * This is the most simple implementation.
 */
void frame_draw(frame_t *frame)
{
	if (frame == NULL || frame->osl_img == NULL) {
		printf("[WARNING]draw null img!");
		return;
	}

	OSL_IMAGE *img = frame->img;
	img->x = frame->x;
	img->y = frame->y;
	img->center_x = frame->center_x;
	img->center_y = frame->center_y;
	img->stretchX = img->sizeX * frame->scale_x;
	img->stretchY = img->sizeY * frame->scale_y;
	img->palette = frame->palette;
	oslDrawImage(img);
}

void frame_destroy(frame_t *frame)
{
	if (frame != NULL) {
		if (frame->osl_img != NULL) {
			oslDeleteImage(frame->osl_img);
			frame->osl_img = NULL;
		}
		free(frame);
	}
}