# $Id$
import os
import psp2d
import config

ROOT = "taiko/res"

_tex = {}

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

def CSoulBar():
    pass

class _SoulBar(object):
    def __init__():
        pass

