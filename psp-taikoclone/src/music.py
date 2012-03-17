# $Id$
import os
import pspsnd # need import this first, it just works this way.
import pspogg, pspmp3

class UnsupportedError(Exception):
    def __init__(self, reason):
        self.reason = reason
    def __repr__(self):
        return reason 

instance = None
def CMusic(filename):
    global instance
    if instance is not None:
        if os.path.abspath(instance.filename) == os.path.abspath(filename):
            return instance
        instance.stop()
        instance = None
    ext = filename.rsplit('.', 2)[-1]
    if ext.lower() == "ogg":
        instance = CMusicOgg(filename)
    elif ext.lower() == "mp3":
        instance = CMusicMp3(filename)
    else:
        raise UnsupportedError("Only support mp3/ogg at the moment.")
    return instance

class _CMusic(object):
    def __init__(self, filename):
        self.filename = filename

    def start(self):
        pass

    def pause(self):
        pass

    def stop(self):
        pass

    def get_millisec(self):
        pass

# a wrapper of pspogg
class CMusicMp3(_CMusic):
    def __init__(self, filename):
        super(CMusicMp3, self).__init__(filename)
        pspmp3.init(1)        
        pspmp3.load(filename)

    def start(self):
        pspmp3.play()

    def pause(self):
        pspmp3.pause()

    def stop(self):
        pspmp3.end()

    def get_millisec(self):
        return pspmp3.get_millisec()

    def get_volume(self):
        return pspmp3.getvol()

    def set_volume(self, vol):
        return pspmp3.setvol(vol)

# a wrapper of pspmp3
class CMusicOgg(_CMusic):
    def __init__(self, filename):
        super(CMusicOgg, self).__init__(filename)
        pspogg.init(2)
        pspogg.load(filename)

    def start(self):
        pspogg.play()

    def pause(self):
        pspogg.pause()

    def stop(self):
        pspogg.end()

    def get_millisec(self):
        return pspogg.get_millisec()

    def get_volume(self):
        return pspogg.getvolume()

    def set_volume(self, vol):
        return pspogg.volume(vol)
