import pspogg, psp2d

pspogg.init(2)
pspogg.load('bgm.ogg')

pspogg.play()

while 1:
    pad = psp2d.Controller()
    if pad.circle:
        print pspogg.getmillisec()
    if pspogg.endofstream() == 1 or pad.cross:
        print pspogg.getsec()
        print "end of stream"
        pspogg.end()
        break

print "exit"

