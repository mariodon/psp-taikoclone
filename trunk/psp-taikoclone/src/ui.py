# $Id$
import os
import psp2d
import config

ROOT = "taiko/res"

_tex = {}

def preload_textures():
    get_texture(config.BG)
    get_texture(config.TAIKO)
    get_texture(config.TAIKO_FLASH_BLUE)
    get_texture(config.TAIKO_FLASH_RED)
    get_texture(config.NORMA_GAUGE)
    get_texture(config.NOTES)

def get_texture(name):
    global _tex
    full_name = os.path.join(ROOT, name)
    ret = _tex.get(full_name)
    if ret is None:
        assert os.path.isfile(full_name), "missing texuture %s" % name      
        ret = psp2d.Image(full_name)
        _tex[full_name] = ret
    return ret

def draw_background(screen):
    tex = get_texture(config.BG)
    screen.blit(tex, 0, 0, -1, -1, config.BG_POS[0], config.BG_POS[1])

def draw_taiko(screen):
    tex = get_texture(config.TAIKO)
    screen.blit(tex, 0, 0, -1, -1, config.TAIKO_POS[0], \
            config.TAIKO_POS[1])

def draw_taiko_flash(screen, color, dir):
    if color == "blue":
        tex = get_texture(config.TAIKO_FLASH_BLUE)
    else:
        assert color == "red"
        tex = get_texture(config.TAIKO_FLASH_RED)
    if dir == "left":
        sx, sy, w, h = config.TAIKO_FLASH_LEFT
    else:
        assert dir == "right"
        sx, sy, w, h = config.TAIKO_FLASH_RIGHT
    screen.blit(tex, sx, sy, w, h, sx + config.TAIKO_FLASH_POS[0],\
            sy + config.TAIKO_FLASH_POS[1], True)

inst_taiko_flash = None
def CTaikoFlash():
    global inst_taiko_flash
    if inst_taiko_flash is not None:
        return inst_taiko_flash
    inst_taiko_flash = _CTaikoFlash()
    return inst_taiko_flash

class _CTaikoFlash(object):
    def __init__(self):
        self.data = [-1, -1, -1, -1]
        self.flash_duration = 0.15

    def update(self, elapse_time):
        for i in xrange(len(self.data)):
            self.data[i] -= elapse_time

    def _get_idx(self, color, dir):
        c = 0 if color == "red" else 1
        dir = 0 if dir == "left" else 1
        return c * 2 + dir

    def _get_color_dir(self, idx):
        color = "red" if idx // 2 == 0 else "blue"
        dir = "left" if idx % 2 == 0 else "right"
        return color, dir

    def add(self, color, dir):
        idx = self._get_idx(color, dir)
        self.data[idx] = self.flash_duration 

    def draw(self, scr):
        for i in xrange(len(self.data)):
            if self.data[i] <= 0:
                continue
            color, dir = self._get_color_dir(i)
            draw_taiko_flash(scr, color, dir)

class SoulBar(object):
    def __init__(self, min, max, norma):
        self.min = min
        self.max = max
        self.norma = norma
        self._value = min
        self.tex = None

    def set_texture(self, name):
        self.tex = get_texture(name)

    def set_value(self, val):
        val = max(self.min, min(val, self.max))
        self._value = val

    def get_value(self):
        return self._value

    value = property(get_value, set_value)

    def draw(self, scr):
        assert self.tex is not None
        sx, sy, w, h = config.NORMA_GAUGE_EMPTY
        dx, dy = config.NORMA_GAUGE_POS
        scr.blit(self.tex, sx, sy, w, h, dx, dy, True) # draw empty gauge
        
        gauge = None
        if self._value == self.max:
            gauge = config.NORMA_GAUGE_FULL
        elif self._value >= self.norma:
            gauge = config.NORMA_GAUGE_NOT_FULL
        else:
            gauge = config.NORMA_GAUGE_NOT_CLEAR
        sx, sy, w, h = gauge
        w = int(1.0 * w * self._value / self.max)
        scr.blit(self.tex, sx, sy, w, h, dx, dy, True)

class CNoteFlow(object):
    def __init__(self, offset, notes, start_x, end_x, y):
        self.notes = notes
        self.offset = offset
        self.time_passed = 0
        self.start_x = start_x
        self.end_x = end_x
        self.note_dist = abs(end_x - start_x)
        self.y = y
        # global note spd
        
        #self.spd = 30 * 160 * 4 / (60000)
        self.spd = 0.32

        print "startx, end_x", self.start_x, self.end_x
        print "y=", self.y
        print "tm_pass_screen", int(self.note_dist / self.spd)        

    def _is_insight(offset, play_time, spd=0):
        return play_time + int(self.note_dist / spd) >= offset

    def get_pos(self, offset, play_time, spd=0):
        ret = self.end_x - (play_time + int(self.note_dist / spd) - offset) * spd

        return ret
    def update(self, play_time):
        # remove earliest notes which is too late to hit
        while self.notes:
            first_note = self.notes[0]
            note_offset = first_note[1]
            if note_offset + self.offset >= play_time:
                self.notes.pop(0)
            else:
                break

        # update the position of all notes which should be seen
        for i, note in enumerate(self.notes):
            if not self._is_insight(note[1], play_time, self.spd):
                pass

    def draw(self, scr, play_time):
        too_late_cnt = 0
        # update the position of all notes which should be seen
        for i, note in enumerate(self.notes):
            x = self.get_pos(note[1], play_time, self.spd)
            if x < self.end_x:
                too_late_cnt += 1
            elif x <= self.start_x:
                draw_note(scr, note[0], int(x), self.y) 
            else:
                break
        # TODO: show missed anim
        if too_late_cnt > 0:
            self.notes = self.notes[too_late_cnt:]

DON = 1
KATSU = 2
BIG_DON = 3
BIG_KATSU = 4
YELLOW = 5 
BIG_YELLOW = 6 
BALLON = 7
IMO = 9
DURATION_END = 8

# bar line can be treated as a special note
def draw_note(screen, type, x, y):
    tex = get_texture(config.NOTES)
    area = None
    if type == DON:
        area = config.NOTE_DON
    elif type == KATSU:
        area = config.NOTE_KATSU
    elif type == BIG_DON:
        area = config.NOTE_BIG_DON
    elif type == BIG_KATSU:
        area = config.NOTE_BIG_KATSU
    elif type == YELLOW:
        area = config.NOTE_YELLOW_HEAD
    elif type == BIG_YELLOW:
        area = config.NOTE_BIG_YELLOW_HEAD
    elif type == DURATION_END:
        return

    if type in (IMO, BALLON):
        sx, sy, w, h = config.NOTE_BALLON
        x -= config.NOTE_BALLON_CENTER[0]
        y -= config.NOTE_BALLON_CENTER[1]
    else:
        assert area is not None
        sx, sy, w, h = area
        x -= w/2
        y -= h/2

    screen.blit(tex, sx, sy, w, h, x, y, True)
    return
