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


class Session():

    def __init__(self):
        self._output_speed = 0
        self._render_speed = 0
        self._render_load = 0
        self._ui_lag = 0
        self._progress_position = 1
        self._progress_steps = 1
        self._audio_levels = (0, 0)
        self._channel_selected_slot_id = dict()
        self._channel_active_slot_id = dict()
        self._channel_active_note = dict()
        self._slot_active_notes = dict()

    def get_output_speed(self):
        return self._output_speed

    def set_output_speed(self, output_speed):
        self._output_speed = output_speed

    def get_render_speed(self):
        return self._render_speed

    def set_render_speed(self, render_speed):
        self._render_speed = render_speed

    def get_render_load(self):
        return self._render_load

    def set_render_load(self, render_load):
        self._render_load = render_load

    def get_ui_lag(self):
        return self._ui_lag

    def set_ui_lag(self, ui_lag):
        self._ui_lag = ui_lag

    def get_progress_position(self):
        return self._progress_position

    def set_progress_position(self, progress_position):
        self._progress_position = progress_position

    def get_progress_steps(self):
        return self._progress_steps

    def set_progress_steps(self, progress_steps):
        self._progress_steps = progress_steps

    def get_audio_levels(self):
        return self._audio_levels

    def set_audio_levels(self, audio_levels):
        self._audio_levels = audio_levels

    def get_selected_slot_id_by_channel(self, channel):
        return self._channel_selected_slot_id[channel]

    def set_selected_slot_id(self, channel, slot):
        slot_id = 'slot_{0:02x}'.format(slot)
        self._channel_selected_slot_id[channel] = slot_id

    def get_active_slot_id_by_channel(self, channel):
        return self._channel_active_slot_id[channel]

    def get_active_note_by_channel(self, channel):
        return self._channel_active_note[channel]

    def get_active_notes_by_slot_id(self, slot_id):
        try:
            notes = self._slot_active_notes[slot_id]
        except KeyError:
            notes = dict()
        return notes

    def set_active_note(self, channel, pitch):
        if pitch == None:
            if channel in self._channel_active_note:
                del self._channel_active_note[channel]
            slot_id = self.get_active_slot_id_by_channel(channel)
            if slot_id in self._slot_active_notes:
               notes = self._slot_active_notes[slot_id]
               if channel in notes:
                   del notes[channel]
        else:
            self._channel_active_note[channel] = pitch
            slot_id = self.get_selected_slot_id_by_channel(channel)
            if not slot_id in self._slot_active_notes:
                self._slot_active_notes[slot_id] = dict()
            notes = self._slot_active_notes[slot_id]
            notes[channel] = pitch
            self._channel_active_slot_id[channel] = slot_id

    def get_keymap_data(self):
        keymap_data = {
            'name': '12-tone Equal Temperament',
            'base_octave': 5,
            'keymap': [
                [-5700, -5600, -5500, -5400, -5300, None, -5200, -5100, -5000, -4900, -4800, -4700, -4600, None],
                [-4500, -4400, -4300, -4200, -4100, None, -4000, -3900, -3800, -3700, -3600, -3500, -3400, None],
                [-3300, -3200, -3100, -3000, -2900, None, -2800, -2700, -2600, -2500, -2400, -2300, -2200, None],
                [-2100, -2000, -1900, -1800, -1700, None, -1600, -1500, -1400, -1300, -1200, -1100, -1000, None],
                [-900, -800, -700, -600, -500, None, -400, -300, -200, -100, 0, 100, 200, None],
                [300, 400, 500, 600, 700, None, 800, 900, 1000, 1100, 1200, 1300, 1400, None],
                [1500, 1600, 1700, 1800, 1900, None, 2000, 2100, 2200, 2300, 2400, 2500, 2600, None],
                [2700, 2800, 2900, 3000, 3100, None, 3200, 3300, 3400, 3500, 3600, 3700, 3800, None],
                [3900, 4000, 4100, 4200, 4300, None, 4400, 4500, 4600, 4700, 4800, 4900, 5000, None],
                [5100, 5200, 5300, 5400, 5500, None, 5600, 5700, 5800, 5900, 6000, 6100, 6200, None],
                [6300, 6400, 6500, 6600, 6700, None, 6800, 6900, 7000, 7100, 7200, 7300, 7400, None]
            ]
        }
        return keymap_data

