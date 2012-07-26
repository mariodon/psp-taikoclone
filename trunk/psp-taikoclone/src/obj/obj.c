#include "load_texture.h"
#include "bg_upper.h"
#include "animation.h"

bg_upper_def_t bg_upper_def_t;

bool init_obj_def()
{
	//#############################
	//bg_upper
	//#############################
	image_bg_upper_normal;
	image_bg_upper_miss;
	image_bg_upper_clear;
	image_bg_upper_hot;
	
	anim_bg_upper;
	
	anim_bg_upper.add_image_animation(image_bg_upper_normal);
	anim_bg_upper.step(t)
	f = anim_bg_upper.get_frame();
	draw(f, f->x, f->y)
	draw(f, f->x + delta, f->y)
	draw(f, f->x + delta * 2, f->y);
	
	anim_bg_upper.set_image_animation(image_bg_upper_normal)
//	anim_bg_upper.reset_image_animation(); do not do this.

	//#############################
	//note fly
	//#############################
	image_note_don;
	image_note_katsu;
	image_note_ldon;
	image_note_rdon;
	pos_curve_animator;
	scale_animator;
	
	anim_note_fly[60];
	
	anim_note_fly[0].set_image_animation(image_note_don);
	anim_note_fly[0].set_scale_animation(scale_animator);
	anim_note_fly[0].set_curve_animation(pos_curve_animation);
	anim_note_fly[0].set_stop_callback(play_a_soul_explosion_animation);
	
	//#############################
	//note animation
	//#############################
	palette_animator_become_red_for_3_seconds;
	palette_animator_become_yellow_for_3_seconds;
	anim_note[80];
	
	anim_note.add_image_animation(image_note_don)
	//note will be open mouth and close mouth

	//#############################
	//soul explosion
	//#############################
	soul_explode_images;
	
	anim_soul_explosion();
	anim_soul_explosion.set_images();
	
	or
	
	anim_soul_explosion.set_image();
	//make sure the following 2 has same length
	anim_soul_explosion.set_scale_animator();
	anim_soul_explosion.set_palette_animator();
	
	//#############################
	//don chan
	//#############################	
	don_chan_images_jump;
	don_chan_images_idle;
	don_chan_images_miss;
	don_chan_images_gogo;
	don_chan_golden_animator;
	
	don_chan_animation;
	don_chan_animation.set_image_animator(idle);
	//when gogotime
	don_chan_animation.set_image_animator(don_chan_images_gogo);
	//when full soul
	don_chan_animation.set_palette_animator(don_chan_golden_animator);
	//when miss
	don_chan_animation.set_image_animator(don_chan_images_miss);
	don_chan_animation.set_palette_animator(NULL);
		
	//#############################
	//chicks
	//#############################	
	chick_great_images;
	chick_good_images;
	chick_bad_images;
	chick_great_pos_animator;
	chick_good_pos_animator;
	chick_bad_pos_animator;		
	
	chick_animation[80];
	//when a note is hit great
	chick_animation.pop().reset()
	chick_animation.set_image_animator(chick_greate_images);
	chick_animation.set_pos_animator(chick_greate_pos_animator);
	
	
	// init bg upper definition
	bg_upper_def_t.bg_upper_normal = NULL;
	bg_upper_def_t.bg_upper_miss = NULL;
	bg_upper_def_t.bg_upper_clear = NULL;
	bg_upper_def_t.bg_upper_full = NULL;
	bg_upper_def_t.scroll_speed = 10;
	
	// init donchan definition
}

void delete_note_animator()
{
	recyle_to_pool();
}

void get_note_animator()
{
	one = get_one_from_pool();
	return one;
}

void obj_update()
{
	
}

void obj_register()
{
}