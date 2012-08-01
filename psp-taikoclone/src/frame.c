#include "frame.h"

frame_t *frame_create(OSL_IMAGE *osl_img)
{
	frame_t *frame = NULL;
	
	frame = (frame_t *)malloc(sizeof(frame_t));
	frame->osl_img = osl_img;
	
	frame->x = 0;
	frame->y = 0;
	
	frame->scale_x = 1.0;
	frame->scale_y = 1.0;
	
    frame->alpha = 1.0;

	/* default to osl_img->palette if NULL */
	frame->palette = NULL;
	return frame;
}

/* draw a frame, with the status applied.
 * This is the most simple implementation.
 */
void frame_draw(frame_t *frame)
{
	if (frame == NULL) { oslFatalError("invalid frame!"); }

    OSL_IMAGE *img = frame->osl_img;
	if (img == NULL) { return; }

    // set up frame->osl_img for rendering
    img->stretchX = (img->offsetX1 - img->offsetX0) * frame->scale_x;
    img->stretchY = (img->offsetY1 - img->offsetY0) * frame->scale_y;
    img->x = frame->x - img->centerX * frame->scale_x;
    img->y = frame->y - img->centerY * frame->scale_y;

	OSL_PALETTE *bak_palette;

    // set blend func
    if (fabs(frame->alpha - 1.0) > 0.001) {
        int destfix = (int)(frame->alpha * 255);
        if (destfix < 0) { destfix = 0; }
        else if (destfix > 255) { destfix = 255; }

        oslSetAlpha(OSL_FX_ALPHA, destfix);
    }

	// be careful not overwrite the img's old palette	
	if (frame->palette != NULL) {
		bak_palette = img->palette;
		img->palette = frame->palette;
		oslDrawImageSimple(img);
		img->palette = bak_palette;
	} else {
		oslDrawImageSimple(img);
	}

    // restore blend func
    if (fabs(frame->alpha - 1.0) > 0.001) {
        oslSetAlpha(OSL_FX_DEFAULT, 0);
    }
}

void frame_draw_xy(frame_t *frame, int x, int y)
{
	if (frame == NULL) { oslFatalError("invalid frame"); }
	
	frame->x += x;
	frame->y += y;
	frame_draw(frame);
	frame->x -= x;
	frame->y -= y;
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

inline int frame_get_width(frame_t *frame)
{
	return frame->osl_img->stretchX;
}

inline int frame_get_height(frame_t *frame)
{
	return frame->osl_img->stretchY;
}
