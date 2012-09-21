#include "note.h"
#include "drawing.h"
#include "tjaparser.h"
#include "aalib/pspaalib.h"

static note_t *N_fumen, *E_fumen, *M_fumen;
static note_t *note_list = NULL;
static note_t *head, *tail, *cur_hit_obj;
static int last_yellow = 0;
static judge_level_t judge = {108, 75, 25};

int note_init(note_t *_N_fumen, note_t *_E_fumen, note_t *_M_fumen)
{
    if (note_list != NULL) {
        note_destroy();
    }
    head = tail = cur_hit_obj = NULL;
    note_list = _N_fumen;
    N_fumen = _N_fumen;
    E_fumen = _E_fumen;
    M_fumen = _M_fumen;
    return 0;
}


int note_need_update(note_t *note, float play_pos)
{
    int x, x2;
    int left, right;

    x = NOTE_FIT_X + (note->offset - play_pos) * note->speed;
    get_note_left_right(note, x, &left, &right);
    //balloon is special
    if (note->type == NOTE_BALLOON) {
        int dummy_left;
        x2 = x + (((yellow_t *)note)->offset2 - note->offset) * note->speed;
        get_note_left_right(note, x2, &dummy_left, &right);
    } 
    //if (note->type == NOTE_YELLOW || note->type == NOTE_LYELLOW) {
    //    printf("(left, right) = (%d, %d)\n", left, right);
    //}
    return !(left > NOTE_APPEAR_X || right < NOTE_DISAPPEAR_X);
}

int note_need_update2(note_t *note, float play_pos)
{
    int x, x2;
    int left, right;

    x = NOTE_FIT_X + (note->offset - play_pos) * note->speed;
    get_note_left_right(note, x, &left, &right);
    //balloon is special
    if (note->type == NOTE_BALLOON) {
        int dummy_left;
        x2 = x + (((yellow_t *)note)->offset2 - note->offset) * note->speed;
        get_note_left_right(note, x2, &dummy_left, &right);
    } 
    return right >= NOTE_DISAPPEAR_X;
}

void note_update_note(note_t *p, float play_pos)
{
    int x = NOTE_FIT_X + (p->offset - play_pos) * p->speed;
    int x2;
    barline_t *note_barline;

    switch(p->type) {
        case NOTE_DON:
        case NOTE_LDON:
        case NOTE_KATSU:
        case NOTE_LKATSU:
        case NOTE_BARLINE:
            note_barline = (barline_t *)p;
            //draw_note(p, x, NOTE_Y);
            break;
        case NOTE_YELLOW:
        case NOTE_LYELLOW:
            //draw_note(p, x, NOTE_Y);
            break;
        case NOTE_BALLOON:
            x2 = x + (((yellow_t *)p)->offset2 - p->offset) * p->speed;
            if (x >= NOTE_FIT_X) {
                //draw_note(p, x, NOTE_Y);
            } else if (x2 >= NOTE_FIT_X) {
                //draw_note(p, NOTE_FIT_X, NOTE_Y);
            } else {
                //draw_note(p, x2, NOTE_Y);
            }
        default:
            break;
    }
}

int note_update(float play_pos, int auto_play, OSL_CONTROLLER *pad)
{
    int x, x2, t_delta;
    note_t *p, *tmpp;
    note_t *prev;
    branch_start_t *pbs;
    int id = 2;
    int offset = 0;
    int cn = 0;

    int hit_off = FALSE;
    int hit_over = FALSE;
    int hit_ok = FALSE;

    play_input_t input;

    //update taiko flash 
    play_pos += offset; 

    /* [head, tail], init update section */
    if (head == NULL) {
        head = tail = note_list;
        head->prev = tail->prev = NULL;
    }

    //printf("before move tail forward, (%p, %p)\n", head, tail);
    /* start from tail to see if we need update more notes. */
    for (p = tail, prev = tail->prev; p != NULL;  prev = p, p = (note_t *)(p->next)) {

        /* special~ influence note flow */
        if (p->type == NOTE_BRANCH_START) {
            if (note_need_update(p, play_pos)) {
                p->offset = -10000;
                printf("\n[Branch %d]\n", id);
                pbs = (branch_start_t *)p;
                pbs->bak_next = pbs->next;	// backup old next pointer for 		
                							// a later traverse

                if (id == 0) {
                    pbs->next = pbs->fumen_n->next;
                } else if (id == 1) {
                    pbs->next = pbs->fumen_e->next;
                } else if (id == 2) {
                    pbs->next = pbs->fumen_m->next;
                }
                tail = (note_t *)p;
                cn = 0;
            } else {
                break;
            }
        }

        if (note_need_update(p, play_pos)) {
            tail = (note_t *)p;
            cn = 0;
        } else {
            cn ++;
            if (cn >= 5) {
                break;
            }
        }

        // work out reverse link
        p->prev = prev;

        //find the first hit obj
        if (cur_hit_obj == NULL && (p->type == NOTE_DON \
            || p->type == NOTE_LDON || p->type == NOTE_KATSU \
            || p->type == NOTE_LKATSU || p->type == NOTE_YELLOW \
            || p->type == NOTE_LYELLOW || p->type == NOTE_BALLOON)) {
            cur_hit_obj = p;
        }
    }

    memset(&input, 0, sizeof(input));
    if (! auto_play) {
        input.left_don = (pad->pressed.right || pad->pressed.down);
        input.right_don = (pad->pressed.square || pad->pressed.cross);
        input.left_katsu = (pad->pressed.up || pad->pressed.left);
        input.right_katsu = (pad->pressed.triangle || pad->pressed.circle);
        input.big_don = (input.left_don && input.right_don);
        input.big_katsu = (input.left_katsu && input.right_katsu);
    }

    /* generate auto play input */
    if (cur_hit_obj != NULL) {
        t_delta = cur_hit_obj->offset - play_pos;
        x = NOTE_FIT_X + t_delta * cur_hit_obj->speed;        
        switch(cur_hit_obj->type) {
            case NOTE_DON:
            case NOTE_LDON:
            case NOTE_KATSU:
            case NOTE_LKATSU:
                hit_over = (hit_over || -t_delta > judge.bad);
                if (hit_over) {
                    break;
                }
                if (auto_play && x <= NOTE_FIT_X) {
                    if (cur_hit_obj->type == NOTE_DON) {
                        input.left_don = 1;
                    } 
                    if (cur_hit_obj->type == NOTE_LDON) {
                        input.left_don = input.right_don = 1;
                    }
                    if (cur_hit_obj->type == NOTE_KATSU) {
                        input.left_katsu = 1;
                    }
                    if (cur_hit_obj->type == NOTE_LKATSU) {
                        input.right_katsu = input.left_katsu = 1;
                    }                    
                }

                if (((input.left_don || input.right_don) && (cur_hit_obj->type == NOTE_DON || cur_hit_obj->type == NOTE_LDON)) \
                       || ((input.left_katsu || input.right_katsu) && (cur_hit_obj->type == NOTE_KATSU || cur_hit_obj->type == NOTE_LKATSU))) {
                    if (abs(t_delta) <= judge.bad) {
                        hit_ok = TRUE;
                        hit_off = TRUE;
                        hit_over = TRUE;
                    }
                }
                break;
            
            case NOTE_YELLOW:
            case NOTE_LYELLOW:
                //printf("yellow offset2 = %f\n", ((yellow_t *)cur_hit_obj)->offset2);

                x2 = NOTE_FIT_X + (((yellow_t *)cur_hit_obj)->offset2 - play_pos) * cur_hit_obj->speed;        
                if (auto_play && x <= NOTE_FIT_X && x2 >= NOTE_FIT_X) {
                    if (!last_yellow) {
                        input.left_don = 1;
                    }
                    last_yellow = (last_yellow + 1) % 3;
                }

                if (x <= NOTE_FIT_X && x2 >= NOTE_FIT_X && (input.left_don || input.right_don || input.right_katsu || input.left_katsu)) {
                    hit_ok = TRUE;
                } else if (x2 < NOTE_FIT_X) {
                    hit_over = TRUE;
                }
                break;
            case NOTE_BALLOON:
                //printf("balloon hit_count %d", ((balloon_t *)cur_hit_obj)->hit_count);
                x2 = NOTE_FIT_X + (((yellow_t *)cur_hit_obj)->offset2 - play_pos) * cur_hit_obj->speed;        
                if (auto_play && x<= NOTE_FIT_X && x2 >= NOTE_FIT_X) {
                    if (!last_yellow) {
                        input.left_don = 1;
                    }
                    last_yellow = (last_yellow + 1) % 3;
                }

                if (x <= NOTE_FIT_X && x2 >= NOTE_FIT_X && (input.left_don || input.right_don)) {
                    (((balloon_t *)cur_hit_obj)->hit_count) -= input.left_don + input.right_don;
                    hit_ok = TRUE;
                    if ((((balloon_t *)cur_hit_obj)->hit_count) <= 0) {
                        hit_over = hit_off = TRUE;
                    }
                } else if (x2 < NOTE_FIT_X) {
                    hit_over = TRUE;
                }
                break;
            default:
                break;
        }
    }

    if (input.left_don || input.right_don) {
        AalibRewind(PSPAALIB_CHANNEL_WAV_1);
        AalibPlay(PSPAALIB_CHANNEL_WAV_1);
    }
    if (input.left_katsu || input.right_katsu) {
        AalibRewind(PSPAALIB_CHANNEL_WAV_2);
        AalibPlay(PSPAALIB_CHANNEL_WAV_2);
    }

    if (hit_off) {
        if (cur_hit_obj->type == NOTE_BALLOON) {
            AalibRewind(PSPAALIB_CHANNEL_WAV_3);
            AalibPlay(PSPAALIB_CHANNEL_WAV_3);
        }
        cur_hit_obj->type = NOTE_DUMMY;
    }

    if (hit_over) {
        for (p = cur_hit_obj->next; tail == NULL ? p != tail : p != tail->next; p = (note_t *)p->next) {
            if (p->type == NOTE_DON \
                || p->type == NOTE_LDON || p->type == NOTE_KATSU \
                || p->type == NOTE_LKATSU || p->type == NOTE_YELLOW \
                || p->type == NOTE_LYELLOW || p->type == NOTE_BALLOON) {
                cur_hit_obj = p;
                hit_over = FALSE;
                break;
            }
        }
        // not find
        if (hit_over) {
            cur_hit_obj = NULL;
        }
    }

    //printf("after move tail forward, (%p, %p)\n", head, tail);    
    /* start from head to see if anyone doesn't need update any more*/
    for (p = head; p != tail; p = (note_t *)(p->next)) {

        if (!note_need_update2(p, play_pos)) {
            head = p->next;
        } else {
            break;
        }
    }

    //printf("after move head forward, (%p, %p)\n", head, tail);

    /* start from tail to head to do a full update & render */
    for (p = tail; p != head->prev; p = (note_t *)p->prev) {
        if (note_need_update(p, play_pos)) {
            note_update_note(p, play_pos);
        }
        //printf("update %d ", p->type);
    }

    if (head == tail && tail->next == NULL && !note_need_update(head, play_pos)) {
        return 0;
    }
    //printf("\n");
    //oslDrawImage(taiko);    
    //update_taiko_flash(elapse_time);
    return 1;
}

void note_free_note_list(note_t *p)
{
    int note_type;
    note_t *next;

    if (p == NULL) {
        return;
    }

	if (p->type == NOTE_BRANCH_START) {
		branch_start_t *pbs = (branch_start_t *)p;
		
		if (pbs->bak_next != NULL) {
			next = pbs->bak_next;
		} else {
			next = pbs->next;
		}
	} else {
		next = p->next;
	}
	
	note_free_note_list(next);

    free(p);
}

int note_destroy()
{

    if (note_list == NULL) {
        return 0;
    }
    
    note_free_note_list(N_fumen);
    note_free_note_list(E_fumen);
    note_free_note_list(M_fumen); 
    
    N_fumen = E_fumen = M_fumen = NULL;       
    note_list = NULL;
    return 0;
}
