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

from PyQt4.QtCore import *
from PyQt4.QtGui import *


class TWLed(QFrame):

    def __init__(self):
        super(QFrame, self).__init__()
        self.setMinimumWidth(35)
        self.setMinimumHeight(15)
        self.setMaximumWidth(35)
        self.setMaximumHeight(15)
        self.setFrameStyle(QFrame.Panel | QFrame.Sunken)
        self.setLineWidth(2)

        self._left = QLabel()
        self._left.setMargin(0)
        self._left.setMaximumWidth(10)
        self._center = QLabel()
        self._center.setMargin(0)
        self._right = QLabel()
        self._right.setMargin(0)
        self._right.setMaximumWidth(10)

        h = QHBoxLayout()
        h.addWidget(self._left)
        h.addWidget(self._center)
        h.addWidget(self._right)
        h.setContentsMargins(0,0,0,0)
        h.setSpacing(0)
        self.setLayout(h)

        self.set_leds(0,0,0)

    def set_leds(self, left_on, center_on, right_on):
        led_colors = {
            (0,0,0): ('#400', '#400', '#400'),
            (0,0,1): ('#400', '#400', '#c00'),
            (0,1,0): ('#c00', '#c00', '#c00'),
            (0,1,1): ('#c00', '#c00', '#f00'),
            (1,0,0): ('#c00', '#400', '#400'),
            (1,0,1): ('#c00', '#400', '#c00'),
            (1,1,0): ('#f00', '#c00', '#c00'),
            (1,1,1): ('#f00', '#c00', '#f00')
        }
        bits = (left_on, center_on, right_on)
        styles = ['QLabel { background-color: %s; }' % c for c in led_colors[bits]]
        (left_style, center_style, right_style) = styles
        self._left.setStyleSheet(left_style)
        self._center.setStyleSheet(center_style)
        self._right.setStyleSheet(right_style)

class TypeWriterButton(QPushButton):

    def __init__(self, pitch):
        QPushButton.__init__(self)
        self._updater = None
        self._pitch = pitch
        self._ui_manager = None
        self._typewriter_manager = None

        self._selected_control = None
        self.setMinimumWidth(60)
        self.setMinimumHeight(60)
        layout = QVBoxLayout(self)
        led = TWLed()
        self._led = led
        layout.addWidget(led)
        notename = QLabel()
        self._notename = notename
        notename.setAlignment(Qt.AlignCenter)
        layout.addWidget(notename)
        layout.setAlignment(Qt.AlignCenter)
        self.setFocusPolicy(Qt.NoFocus)

        if self._pitch != None:
            self._notename.setText('%sc' % self._pitch)
            self.setStyleSheet("QLabel { background-color: #ffe; }")
            self._notename.setStyleSheet("QLabel { color: #000; }")
        else:
            self.setStyleSheet("QLabel { background-color: #ccc; }")

        self.setEnabled(False)
        QObject.connect(self, SIGNAL('pressed()'), self._play_sound)
        QObject.connect(self, SIGNAL('released()'), self._stop_sound)

    def set_ui_model(self, ui_model):
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self.perform_updates)
        self._ui_manager = ui_model.get_ui_manager()
        self._typewriter_manager = ui_model.get_typewriter_manager()

    def _play_sound(self):
        if self._selected_control:
            self._selected_control.set_active_note(0, self._pitch)

    def _stop_sound(self):
        if self._selected_control:
            self._selected_control.set_rest(0)

    def update_selected_control(self):
        self._selected_control = self._ui_manager.get_selected_control()
        if self._pitch != None:
            self.setEnabled(True)

    def update_leds(self):
        if self._selected_control == None:
            return
        (left_on, center_on, right_on) = 3 * [0]
        notes = self._selected_control.get_active_notes()
        for (_, note) in notes.items():
            if self._typewriter_manager.get_closest_keymap_pitch(note) == self._pitch:
                if note < self._pitch:
                    left_on = 1
                elif note == self._pitch:
                    center_on = 1
                elif note > self._pitch:
                    right_on = 1
                else:
                    assert False
        self._led.set_leds(left_on, center_on, right_on)

    def perform_updates(self, signals):
        self.update_selected_control()
        self.update_leds()

    def unregister_updaters(self):
        self._updater.unregister_updater(self.perform_updates)
