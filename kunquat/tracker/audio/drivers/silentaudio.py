# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import time

class Silentaudio():

    def __init__(self):
        self._started = False
        self._audio_source = None

    def set_audio_source(self, audio_source):
        self._audio_source = audio_source

    def put_audio(self, audio_data):
        assert self._started
        (l, r) = audio_data
        frames = len(l)
        play_time = frames / 48000.0
        print play_time
        time.sleep(play_time)
        self._audio_source.acknowledge_audio()

    def start(self):
        self._started = True

    def stop(self):
        pass

    def close(self):
        self.stop()

    @classmethod
    def get_id(cls):
        return 'silentaudio'

    #'Silent null driver'

