# $Id$
import os
import psp2d
import config

ROOT = "taiko/res"

_tex = {}

def get_texture(name):
    global _tex
    full_name = os.path.join(ROOT, name)
    assert os.path.isfile(full_name), "missing texuture %s" % name
    ret = _tex.get(full_name)
    if not ret:
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
