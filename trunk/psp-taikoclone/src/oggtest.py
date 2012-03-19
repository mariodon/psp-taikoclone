import osl

osl.initAudio()
bgm = osl.Sound("taiko/songs/aaa.mp3", osl.FMT_NONE)
bgm.play()

while True:
    ctrl = osl.Controller()
    if ctrl.held_start:
        break

