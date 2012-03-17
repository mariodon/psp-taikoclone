#-*- coding: ISO-8859-1 -*-

import time
import os
import psp2d
import pspsnd
import music

RES_PATH = "taiko/res/"
SONG_PATH = "taiko/songs/"
SFX_PATH = "taiko/hitsounds/"
DON = "don"
KA = "katsu"
soundMap = {}

def play_sfx(name):
    assert name in (DON, KA)
    snd = soundMap[os.path.join(SFX_PATH,name)+".wav"]
    snd.start()

def on_left_down():
    play_sfx(KA)
def on_down_down():
    play_sfx(DON)
def on_cross_down():
    play_sfx(DON)
def on_circle_down():
    play_sfx(KA)

is_keybuffer_inited = False
keybuffer = {"left":None, "down":None, "right":None, "up":None,
        "square":None, "circle":None, "cross":None, "triangle":None}
all_keys = keybuffer.keys()
event_map = {"left_down": on_left_down, "down_down": on_down_down,
        "circle_down": on_circle_down, "cross_down": on_cross_down}

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

    init_sound_fx()

    bgm = music.CMusic(os.path.join(SONG_PATH, "bgm.ogg"))
    bgm.set_volume(70)
    bgm.start()
    
    pspsnd.setSndFxVolume(255)

    while True:
        pad = psp2d.Controller()
        
        new_keybuffer = build_keybuffer(pad)
        update_keybuffer(new_keybuffer)
        if pad.start:
            bgm.stop()
            break

        scr.swap()

# TODO: support diy hitsound
def init_sound_fx():
    global soundMap
    snd = pspsnd.Sound(os.path.join(SFX_PATH, DON)+".wav")
    snd2 = pspsnd.Sound(os.path.join(SFX_PATH, KA)+".wav")
    soundMap[os.path.join(SFX_PATH,DON)+".wav"] = snd
    soundMap[os.path.join(SFX_PATH,KA)+".wav"] = snd2
    time.sleep(5)    

if __name__ == '__main__':
    main()
