import psp2d
import os

print os.realmem()
import pspsnd

print os.realmem()
import pspogg

print os.realmem()

pspogg.init(4)
pspogg.load("taiko/songs/bgm.OGG")

pspogg.play()

while 1:
    
    pad = psp2d.Controller()
    if pad.cross:
        break
    continue

    if pspogg.endofstream() == 1 or pad.cross:
        print pspogg.getsec()
        print "end of stream"
        pspogg.end()
        break

print "exit"

