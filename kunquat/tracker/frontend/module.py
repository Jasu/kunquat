# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#


from updater import Updater

class Module(Updater):

    def __init__(self):
        super(Module, self).__init__()
        self._backend = None
        self._instruments = {}

    def set_backend(self, backend):
        self._backend = backend

    def get_instrument(self, instrument_number):
        return self._instruments[instrument_number]

    def update_instrument(self, instrument_number, instrument):
        if instrument_number in self._instruments:
            old_instrument = self._instruments[instrument_number]
            self.unregister_child(old_instrument)
        self.register_child(instrument)
        self._instruments[instrument_number] = instrument
        self._signal_update()

    def get_instruments(self, validate=True):
        all_instruments = self._instruments.values()
        if validate:
            valid = [i for i in all_instruments if i.get_existence()]
            return valid
        return all_instruments
