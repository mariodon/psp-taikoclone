#-*- coding: ISO-8859-1 -*-

import time
import os
import psp2d
import hitsound
import music
import ui
import config
import tja_parser

RES_PATH = "taiko/res/"
SONG_PATH = "taiko/songs/"

taiko_flash = ui.CTaikoFlash() 
soul_bar = ui.SoulBar(0, 240, 180)
noteflow = None

def on_up_down():
    taiko_flash.flash_duration += 0.01
#    print taiko_flash.flash_duration
def on_triangle_down():
    taiko_flash.flash_duration -= 0.01
#    print taiko_flash.flash_duration    
def on_left_down():
    taiko_flash.add("blue", "left")
    hitsound.play(hitsound.KATSU)
    soul_bar.value -= 1
def on_down_down():
    taiko_flash.add("red", "left")
    hitsound.play(hitsound.DON)
    soul_bar.value += 1
def on_cross_down():
    taiko_flash.add("red", "right")
    hitsound.play(hitsound.DON)
    soul_bar.value += 1    
def on_circle_down():
    taiko_flash.add("blue", "right")
    hitsound.play(hitsound.KATSU)
    soul_bar.value -= 1    

is_keybuffer_inited = False
keybuffer = {"left":None, "down":None, "right":None, "up":None,
        "square":None, "circle":None, "cross":None, "triangle":None}
all_keys = keybuffer.keys()
event_map = {"left_down": on_left_down, "down_down": on_down_down,
        "circle_down": on_circle_down, "cross_down": on_cross_down,
        "triangle_down": on_triangle_down, "up_down": on_up_down,}

def build_keybuffer(ctrl):
    ret = {}
    for key in all_keys:
        ret[key] = getattr(ctrl, key)
    return ret

def update_keybuffer(new_keybuffer):
    global keybuffer
    global is_keybuffer_inited
    if not is_keybuffer_inited:
        keybuffer = new_keybuffer
        is_keybuffer_inited = True
        return
    # compare keybuffer and new_keybuffer to emit event
    for key in all_keys:
        if keybuffer[key] == new_keybuffer[key]:
            continue
        event = key + "_" + (keybuffer[key] and "up" or "down")
        if event in event_map:
            event_map[event]()
        keybuffer[key] = new_keybuffer[key]

def main():

    scr = psp2d.Screen()

    fnt = psp2d.Font(os.path.join(RES_PATH, 'font_small.png'))
    dy = fnt.textHeight('') + 5

    # init hitsound
    hitsound.init("default")
    hitsound.set_volume(100)

    # init music
    bgm = music.CMusic(os.path.join(SONG_PATH, "aaa.mp3"))
    bgm.set_volume(40)

    # init misc
    soul_bar.set_texture(config.NORMA_GAUGE)

    # init fumen
    tja_parser.debug_mode = True
    tja_parser.tja2osu(os.path.join(SONG_PATH, "aaa.tja"))

    # init note flow
    global noteflow
    offset = int(-tja_parser.OFFSET * 1000)
    notes = tja_parser.HitObjects
    start_x = config.NOTE_APPEAR_X
    end_x = config.NOTE_DISAPPEAR_X
    note_y = config.NOTE_Y
    noteflow = ui.CNoteFlow(offset, notes, start_x, end_x, note_y)
    # init images
    ui.preload_textures()

    # start play
    bgm.start()
    fps = 60.0
    input_fps = 120.0
    t1 = -1
    t4 = -1
    t0 = time.clock()
    fps_cnt = 0

    input_cost_max = -1
    draw_cost_max = -1
    # TODO: limit rendering framerate.
    while True:

        t3 = time.clock()
        if t3 - t4 >= 1/input_fps:
            tb = time.clock()
            pad = psp2d.Controller()
            
            new_keybuffer = build_keybuffer(pad)
            update_keybuffer(new_keybuffer)
            if pad.start:
                print "frame = %d, time = %f" % (fps_cnt, time.clock() - t0)
                print "cost: input=%f, draw=%f" % (input_cost_max,
                        draw_cost_max)
                bgm.stop()
                break
            input_cost_max = max(input_cost_max, time.clock()-tb)
            t4 = t3
    
        t2 = time.clock()
        if t2 - t1 >= 1/fps:
            tb = time.clock()
            fps_cnt += 1
            ui.draw_background(scr)
            ui.draw_taiko(scr)
            taiko_flash.update(t2 - t1)
            taiko_flash.draw(scr)
            soul_bar.draw(scr)
            #noteflow.draw(scr, bgm.get_millisec())
            scr.swap()
            t1 = t2
            draw_cost_max = max(draw_cost_max, time.clock() - tb)

if __name__ == '__main__':
    main()
