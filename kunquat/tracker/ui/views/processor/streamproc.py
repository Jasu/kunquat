# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
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


class StreamProc(QWidget):

    @staticmethod
    def get_name():
        return u'Stream'

    def __init__(self):
        QWidget.__init__(self)

        self._init_state_editor = InitStateEditor()

        v = QVBoxLayout()
        v.addWidget(self._init_state_editor)
        v.addStretch(1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._init_state_editor.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._init_state_editor.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._init_state_editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._init_state_editor.unregister_updaters()


class InitStateEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._init_val = QDoubleSpinBox()
        self._init_val.setMinimum(-99999)
        self._init_val.setMaximum(99999)
        self._init_val.setDecimals(5)

        self._osc_speed = QDoubleSpinBox()
        self._osc_speed.setMinimum(0)
        self._osc_speed.setMaximum(1000)
        self._osc_speed.setDecimals(5)

        self._osc_depth = QDoubleSpinBox()
        self._osc_depth.setMinimum(0)
        self._osc_depth.setMaximum(99999)
        self._osc_depth.setDecimals(5)

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addWidget(QLabel('Initial value:'), 0)
        h.addWidget(self._init_val, 1)
        h.addWidget(QLabel('Initial oscillation speed:'), 0)
        h.addWidget(self._osc_speed, 1)
        h.addWidget(QLabel('Initial oscillation depth:'), 0)
        h.addWidget(self._osc_depth, 1)

        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        self._updater.register_updater(self._perform_updates)

        QObject.connect(
                self._init_val, SIGNAL('valueChanged(double)'), self._set_init_value)
        QObject.connect(
                self._osc_speed, SIGNAL('valueChanged(double)'), self._set_osc_speed)
        QObject.connect(
                self._osc_depth, SIGNAL('valueChanged(double)'), self._set_osc_depth)

        self._update_state()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return 'signal_stream_init_state_{}'.format(self._proc_id)

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_state()

    def _get_stream_params(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        stream_params = proc.get_type_params()
        return stream_params

    def _update_state(self):
        stream_params = self._get_stream_params()

        init_value = stream_params.get_init_value()
        if init_value != self._init_val.value():
            old_block = self._init_val.blockSignals(True)
            self._init_val.setValue(init_value)
            self._init_val.blockSignals(old_block)

        osc_speed = stream_params.get_init_osc_speed()
        if osc_speed != self._osc_speed.value():
            old_block = self._osc_speed.blockSignals(True)
            self._osc_speed.setValue(osc_speed)
            self._osc_speed.blockSignals(old_block)

        osc_depth = stream_params.get_init_osc_depth()
        if osc_depth != self._osc_depth.value():
            old_block = self._osc_depth.blockSignals(True)
            self._osc_depth.setValue(osc_depth)
            self._osc_depth.blockSignals(old_block)

    def _set_init_value(self, value):
        stream_params = self._get_stream_params()
        stream_params.set_init_value(value)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _set_osc_speed(self, value):
        stream_params = self._get_stream_params()
        stream_params.set_init_osc_speed(value)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _set_osc_depth(self, value):
        stream_params = self._get_stream_params()
        stream_params.set_init_osc_depth(value)
        self._updater.signal_update(set([self._get_update_signal_type()]))


