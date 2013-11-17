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

from PyQt4.QtCore import *
from PyQt4.QtGui import *


class RenderStats(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._stat_manager = None

        self._output_speed = QLabel(self)
        self._render_speed = QLabel(self)
        self._render_load = QLabel(self)
        self._ui_lag = QLabel(self)

        v = QVBoxLayout()
        v.addWidget(self._output_speed)
        v.addWidget(self._render_speed)
        v.addWidget(self._render_load)
        v.addWidget(self._ui_lag)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        updater = ui_model.get_updater()
        updater.register_updater(self.perform_updates)
        self._stat_manager = ui_model.get_stat_manager()

    def update_output_speed(self):
        output_speed = self._stat_manager.get_output_speed()
        text = 'output speed: {} fps'.format(int(output_speed))
        self._output_speed.setText(text)

    def update_render_speed(self):
        output_speed = self._stat_manager.get_render_speed()
        text = 'render speed: {} fps'.format(int(output_speed))
        self._render_speed.setText(text)

    def update_render_load(self):
        render_load = self._stat_manager.get_render_load()
        text = 'render load: {} %'.format(int(render_load * 100))
        self._render_load.setText(text)

    def update_ui_lag(self):
        ui_lag = self._stat_manager.get_ui_lag()
        text = 'ui lag: {}'.format(ui_lag)
        self._ui_lag.setText(text)

    def perform_updates(self, signals):
        self.update_output_speed()
        self.update_render_speed()
        self.update_render_load()
        self.update_ui_lag()
