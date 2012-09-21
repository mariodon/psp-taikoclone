# $Id$
# -*- coding:gbk -*-
import struct
import math

DEFAULT_FRAMERATE = 30.0
DEFAULT_INTERP = 0
DEFAULT_IS_LOOPPED = False

"""
	# note don flying to soul bar
    "note_fly_don" : {
    	"framerate": 60,
        "funcs": (
            {
                "type": 0,
                "keys": ((0, 0, 0, 48, 48, 24, 24, "note_don"),),
            },

            {
                "type": 2,
                "interp": 1,
                "keys": ((0, 104, 104), (5, 147, 75), (10, 224, 35), (15, 310, 9), (20, 367, 7), (25, 417, 14), (30, 437, 20), (35, 452, 26),),
            },
        )
    },
    # note katsu flying to soul bar
	"note_fly_katsu": {
    	"framerate": 60,
        "funcs": (
            {
                "type": 0,
                "keys": ((0, 0, 0, 48, 48, 24, 24, "note_katsu"),),
            },

            {
                "type": 2,
                "interp": 1,
                "keys": ((0, 104, 104), (5, 147, 75), (10, 224, 35), (15, 310, 9), (20, 367, 7), (25, 417, 14), (30, 437, 20), (35, 452, 26),),
            },
        )
    },
    # note large don flying to soul bar
	"note_fly_ldon": {
    	"framerate": 60,
        "funcs": (
            {
                "type": 0,
                "keys": ((0, 0, 0, 48, 48, 24, 24, "note_ldon"),),
            },

            {
                "type": 2,
                "interp": 1,
                "keys": ((0, 104, 104), (5, 147, 75), (10, 224, 35), (15, 310, 9), (20, 367, 7), (25, 417, 14), (30, 437, 20), (35, 452, 26),),
            },
        )
    },
    # note large katsu flying to soul bar
	"note_fly_lkatsu": {
    	"framerate": 60,
        "funcs": (
            {
                "type": 0,
                "keys": ((0, 0, 0, 48, 48, 24, 24, "note_lkatsu"),),
            },

            {
                "type": 2,
                "interp": 1,
                "keys": ((0, 104, 104), (5, 147, 75), (10, 224, 35), (15, 310, 9), (20, 367, 7), (25, 417, 14), (30, 437, 20), (35, 452, 26),),
            },
        )
    },
    # anime_bg_upper
	"bg_upper": {
    	"framerate": 60,
        "funcs": (
            {
                "type": 0,
                "keys": ((0, 0, 28, 128, 100, 0, 0, "bg"),),
            },

            {
                "type": 2,
                "interp": 1,
                "is_loopped": True,
                "keys": ((0, 0, 0), (150, -128, 0),),
            },
        )
    },    
    # explosion_upper
    "explosion_upper": {
        "framerate": 60,
        "funcs": (
            {
                "type": 0,
                "keys": ((0, 0, 0, 128, 128, 49, 49, "explosion_upper"),),
            },

            {
                "type": 1,
                "interp": 1,
                "keys": ((0, 0.0, 0.0), (10, 1.0, 1.0)),
            },
        )
    },
    # flame
    "flame": {
        "framerate": 60,
        "funcs": (
            {
                "type": 0,
                "is_loopped": True,
                "keys": (
                    (0, 0, 0, 128, 128, 38, 81, "flame1"),
                    (1, 0, 0, 128, 128, 38, 81, "flame2"),
                    (2, 0, 0, 128, 128, 38, 81, "flame3"),
                    (3, 0, 0, 128, 128, 38, 81, "flame4"),
                    (4, 0, 0, 128, 128, 38, 81, "flame5"),
                    (5, 0, 0, 128, 128, 38, 81, "flame6"),
                    (6, 0, 0, 128, 128, 38, 81, "flame7"),
                    (7, 0, 0, 128, 128, 38, 81, "flame8"),),
            },
        )
    },
"""

DATA = {
    "bg_upper": {
        "fps": 60,
        "loop": True,
        "keys": (
            (127, True, 
                {
                    "tex_name":"bg", 
                    "sx": 0,
                    "sy": 28,
                    "w": 128,
                    "h": 80,
                    "x": 0,
                }
            ),
            (1, False, 
                {
                    "tex_name":"bg",
                    "sx": 0,
                    "sy": 28,
                    "w": 128,
                    "h": 80,
                    "x": -127,
                }
            )
        ),
    },
}

def dump(data):
    str = ""

    key_count = len(data["keys"])
    play_speed = int(math.ceil(60.0 / data["fps"]))
    loop = data["loop"]

    import make_frame_data

    str += struct.pack("<III", key_count, play_speed, loop);
    for key in data["keys"]:
        str += struct.pack("<II", key[0], key[1])
        str += make_frame_data.dump(key[2])

    return str

def dump_f(name, cfg):
    fname = "../ani/"+name+".ani"
    f = open(fname, "wb")
    f.write(dump(cfg))
    f.close()

if __name__ == "__main__":
    for fname, cfg in DATA.iteritems():
        dump_f(fname, cfg)
