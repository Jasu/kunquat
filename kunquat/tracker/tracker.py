# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013
#          Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import time

from audiothread import AudioThread
from backendthread import BackendThread
from frontendthread import FrontendThread


def main():
    at = AudioThread()
    bt = BackendThread()
    ft = FrontendThread()

    at.set_backend(bt)
    bt.set_frontend(ft)
    bt.set_audio_output(at)
    ft.set_backend(bt)
    ft.set_audio_output(at)

    at.start()
    bt.start()
    ft.start()

    try:
        while ft.is_alive():
            print('main')
            time.sleep(1)
    except:
        pass

    ft.halt()
    bt.halt()
    at.halt()
    ft.join()
    bt.join()
    at.join()
    time.sleep(0.1)


