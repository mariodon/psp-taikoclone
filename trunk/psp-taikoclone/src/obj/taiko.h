#ifndef __TAIKO_H__
#define __TAIKO_H__

typedef struct {
	
}taiko_data_t;

void taiko_update(float elapse_time, taiko_data_t *data);
void taiko_render(taiko_data_t *data)
void taiko_on_input();
void taiko_on_combo_change();

#endif