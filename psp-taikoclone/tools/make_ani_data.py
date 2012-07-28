# $Id$
# -*- coding:gbk -*-
import struct

DEFAULT_FRAMERATE = 30.0
DEFAULT_INTERP = 0
DEFAULT_IS_LOOPPED = False

DATA = {
    "note_fly_don" : {
    	"framerate": 60,
        "funcs": (
            {
                "type": 0,
                "is_loopped": True,
                "keys": ((0, 0, 0, 48, 48, 24, 24, "note_don"),
                	(2, 0, 0, 48, 48, 24, 24, "note_katsu"),
                	(4, 0, 0, 48, 48, 24, 24, "note_katsu"),),
            },

            {
                "type": 2,
                "interp": 1,
                "is_loopped": True,                
                "keys": ((0, 104, 104), (5, 147, 75), (10, 224, 35), (15, 310, 9), (20, 367, 7), (25, 417, 14), (30, 437, 20), (35, 452, 26),),
            },
        )
    }
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

dump("note_fly_don")
