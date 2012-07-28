# $Id$
# -*- coding:gbk -*-
import struct

DEFAULT_FRAMERATE = 30.0
DEFAULT_INTERP = 0
DEFAULT_IS_LOOPPED = False

DATA = {
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
                "keys": ((0, 28, 0, 128, 100, 0, 0, "bg"),),
            },

            {
                "type": 2,
                "interp": 1,
                "is_loopped": True,
                "keys": ((0, 0, 0), (100, -128, 0),),
            },
        )
    },    
}

def dump(name):
    data = DATA[name]
    str = ""

    # framerate
    str += struct.pack("<f", data.get("framerate", DEFAULT_FRAMERATE));

    # func_count
    str += struct.pack("<I", len(data["funcs"]))

    # funcs
    for func in data["funcs"]:
        # func headers
        str += struct.pack("<IIII", 
            func["type"], \
            func.get("interp", DEFAULT_INTERP), \
            func.get("is_loopped", DEFAULT_IS_LOOPPED),
            len(func["keys"]))
        # func keys
        if func["type"] == 0:   # sequence type
            for key in func["keys"]:
                str += struct.pack("<IIIIIII24s", *key)
        elif func["type"] == 1: # scale type
            for key in func["keys"]:
                str += struct.pack("<Iff", *key)            
        elif func["type"] == 2: # path type
            for key in func["keys"]:
                str += struct.pack("<Iii", *key)
        elif func["type"] == 3: # palette type
            for key in func["keys"]:
                str += struct.pack("<II", key[0], len(key)-1)
                assert len(key) - 1 in (16, 256)
                for c in key[1:]:
                    str += struct.pack("<BBBB", (c>>24)&0xFF, \
                        (c>>16)&0xFF, (c>>8)&0xFF, c&0xFF)
        else:
            assert False, "Unknown func type!"

    fname = name+".ani"
    f = open(fname, "wb")
    f.write(str)
    f.close()

for k in DATA.keys():
	dump(k)
