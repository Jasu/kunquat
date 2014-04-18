# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#


class Updater(object):

    def __init__(self):
        self._update_signals = set()
        self._updaters = set()
        self._iterator = set()
        self._is_updating = False

    def signal_update(self, signals = set()):
        assert not self._is_updating
        self._update_signals.add('signal_change')
        self._update_signals |= signals

    def register_updater(self, updater):
        self._updaters.add(updater)

    def unregister_updater(self, updater):
        self._updaters.remove(updater)
        self._iterator -= set([updater])

    def perform_updates(self):
        self._is_updating = True
        try:
            self._perform_updates()
        finally:
            self._is_updating = False

    def _perform_updates(self):
        if not self._update_signals:
            return
        self._iterator = set(self._updaters)
        while len(self._iterator) > 0:
            updater = self._iterator.pop()
            updater(self._update_signals)
        self._update_signals = set()

    def verify_ready_to_exit(self):
        if self._updaters:
            updaters_str = '\n'.join(str(u) for u in self._updaters)
            raise RuntimeError(
                    'Updaters left on exit:\n{}'.format(updaters_str))


