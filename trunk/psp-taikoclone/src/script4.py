import pspmp3, psp2d

pspmp3.init(1)
pspmp3.load('AAA.MP3')
pspmp3.play()

while 1:
    global lastPad
    pad = psp2d.Controller()
    if pad.circle:
        print pspmp3.getmillisec()
    if pspmp3.endofstream() == 1 or pad.cross:
        print pspmp3.gettime()
        pspmp3.end()
        break

print "exit"

