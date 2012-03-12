#-*- coding: ISO-8859-1 -*-

import time
import os
import psp2d
import pspsnd

RES_PATH = "taiko/res/"
SFX_PATH = "taiko/hitsounds/"
DON = "don"
KA = "katsu"
soundMap = {}

def play_sfx(name):
    assert name in (DON, KA)
    snd = soundMap[os.path.join(SFX_PATH,name)+"3.wav"]
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

def main2():
    global is_keybuff_inited
    scr = psp2d.Screen()

    fnt = psp2d.Font('font_small.png')
    dy = fnt.textHeight('') + 5

    while True:
        pad = psp2d.Controller()

        scr.blit(img)
        scr.swap()

# TODO: support diy hitsound
def init_sound_fx():
    global soundMap
    snd = pspsnd.Sound(os.path.join(SFX_PATH, DON)+"3.wav")
    snd2 = pspsnd.Sound(os.path.join(SFX_PATH, KA)+"3.wav")
    soundMap[os.path.join(SFX_PATH,DON)+"3.wav"] = snd
    soundMap[os.path.join(SFX_PATH,KA)+"3.wav"] = snd2
    time.sleep(5)    

# load bgm into memory for play
def init_bgm(filename):
    assert isinstance(filename, basestring)
    if filename.endswith(".ogg"):
        init_ogg()
    elif filename.endswith(".mp3"):
        init_mp3()

def clear_bgm(filename):
    assert isinstance(filename, basestring)
    if filename.endswith(".ogg"):
        init_ogg()

def init_ogg():
    pass
def init_mp3():
    pass

def main():
    scr = psp2d.Screen()
    fnt = psp2d.Font(os.path.join(RES_PATH, "font_small.png"))
    dy = fnt.textHeight('') + 5

    while True:
        pad = psp2d.Controller()

        img = psp2d.Image(480, 272)
        img.clear(psp2d.Color(0, 0, 0))

        fnt.drawText(img, 0, 0, 'Analog X: %d' % pad.analogX)
        fnt.drawText(img, 0, dy, 'Analog Y: %d' % pad.analogY)
        fnt.drawText(img, 0, 2 * dy, 'Square: %d' % int(pad.square))
        fnt.drawText(img, 0, 3 * dy, 'Circle: %d' % int(pad.circle))
        fnt.drawText(img, 0, 4 * dy, 'Cross: %d' % int(pad.cross))
        fnt.drawText(img, 0, 5 * dy, 'Triangle: %d' % int(pad.triangle))
        fnt.drawText(img, 0, 6 * dy, 'Left: %d' % int(pad.left))
        fnt.drawText(img, 0, 7 * dy, 'Right: %d' % int(pad.right))
        fnt.drawText(img, 0, 8 * dy, 'Up: %d' % int(pad.up))
        fnt.drawText(img, 0, 9 * dy, 'Down: %d' % int(pad.down))
        fnt.drawText(img, 0, 10 * dy, 'Left trigger: %d' % int(pad.l))
        fnt.drawText(img, 0, 11 * dy, 'Right trigger: %d' % int(pad.r))

        if pad.start:
            break

        scr.blit(img)
        scr.swap()

if __name__ == '__main__':
    main()
