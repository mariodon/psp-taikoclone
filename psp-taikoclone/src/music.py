# $Id$
# -*- coding:gbk -*-
import pspogg, pspmp3
import os

class UnsupportedError(Exception):
    def __init__(self, reason):
        self.reason = reasosn
    def __repr__(self):
        return reason 

instance = None
def CMusic(filename):
    global instance
    if instance is not None:
        if os.path.abspath(instance.filename) != os.path.abspath(filename):
            raise UnsupportedError("Loading Multiple Audio File(mp3/ogg \
                    is not supported at the moment.")
        return instance
    if filename.endswith(".ogg"):
        instance = CMusicOgg(filename)
    elif filename.endswith(".mp3"):
        instance = CMusicMp3(filename)
    else:
        raise UnsupportedError("Only support mp3/ogg at the moment."
    return instance

class _CMusic(object):
    def __init__(self, filename):
        pass

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
        pspmp3.init(1)        
        pspmp3.load(filename)

    def start(self):
        pspmp3.play()

    def pause(self):
        pspmp3.pause()
        pass

    def stop(self):
        pspmp3.end()

    def get_millisec(self):
        return self.mp3.get_millisec()

# a wrapper of pspmp3
class CMusicOgg(_CMusic):
    def __init__(self, filename):
        pspogg.init(2)
        pspogg.load(filename)

    def start(self):
        pspogg.play()

    def pause(self):
        pspogg.pause()

    def stop(self):
        pspogg.end()

    def get_millisec(self):
        return self.ogg.get_millisec()
