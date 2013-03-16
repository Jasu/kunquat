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

from backendthread import BackendThread
from frontendthread import FrontendThread


def main():
    bt = BackendThread()
    ft = FrontendThread()

    bt.set_event_processor(ft.queue_event)
    ft.set_command_processor(bt.queue_command)

    bt.start()
    ft.start()

    try:
        while True:
            print('main')
            time.sleep(1)
    except:
        pass

    ft.halt()
    bt.halt()
    ft.join()
    bt.join()


