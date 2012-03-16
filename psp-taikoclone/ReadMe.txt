This project is using StacklessPython for psp.

I modified it a little bit and allow pspsnd, pspmp3 to work together.

whenever you want to use pspmp3 or pspogg, import pspsnd(which will do the pspAudioInit).

added interface to pspmp3 and pspogg.

pspmp3.getmillisec()
pspmp3.getvol()
pspmp3.setvol(vol)

pspogg.getmillisec()
pspogg.setvolume(vol)