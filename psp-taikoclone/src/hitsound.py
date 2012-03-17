# $Id$
# -*- coding:gbk -*-
import os
import pspsnd

# consts
ROOT = "taiko/hitsound"

DON = "don"
KATSU = "katsu"
FUSEN = "fusen"

ALL_HITSOUND_NAMES = set((DON, KATSU, FUSEN))

ILLEGAL = os.path.join(ROOT, "illegal.wav")

# current hitsound theme
_theme = None
_sounds = {}

def init(theme="default"):
    global _theme
    
    if _theme is None or not _sounds:
        reset()
        
    _theme = theme

    folder = os.path.join(ROOT, _theme)
    assert os.path.isdir(folder), "invalid theme %s" % _theme
    assert os.path.isfile(ILLEGAL), "missing illegal hitsound"
        
    # load illegal sound
    _sounds[ILLEGAL] = pspsnd.Sound(ILLEGAL)
    
    # load hitsounds
    for name in ALL_HITSOUND_NAMES:
        full_name = os.path.join(folder, name)
        assert os.path.isfile(full_name), "missing hitsound %s" % name
        _sounds[full_name] = pspsnd.Sound(full_name)

def play(name=DON):
    assert _theme is not None, "NOT INITED"
    if name not in ALL_HITSOUND_NAMES:
        full_name = ILLEGAL
    else: # ROOT/_theme/name.wav
        full_name = os.path.join(os.path.join(ROOT, _theme), name+".wav")
    snd = _sounds.get(full_name)
    assert snd is not None, "NOT INITED??MISSING FILE??"
    snd.start()
    
def reset():
    _theme = None
    _sounds = {} # yeah, i don't know how to release sound instance.
    
def set_volume(vol):
    pspsnd.setSndFxVolume(vol)