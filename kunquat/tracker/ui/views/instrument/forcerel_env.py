# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
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

from kunquat.tracker.ui.views.envelope import Envelope
from numberslider import NumberSlider


class ForceReleaseEnvelope(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._ins_id = None
        self._updater = None

        header = QLabel('Force release envelope')
        header.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Minimum)
        header_line = QFrame()
        header_line.setFrameShape(QFrame.HLine)
        header_line.setFrameShadow(QFrame.Sunken)
        header_line.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Maximum)
        header_layout = QHBoxLayout()
        header_layout.setContentsMargins(5, 0, 5, 0)
        header_layout.setSpacing(10)
        header_layout.addWidget(header)
        header_layout.addWidget(header_line)

        self._enabled_toggle = QCheckBox('Enabled')
        self._scale_amount = NumberSlider(2, -4, 4, title='Scale amount:')
        self._scale_center = NumberSlider(0, -3600, 3600, title='Scale center:')

        h = QHBoxLayout()
        h.addWidget(self._enabled_toggle)
        h.addWidget(self._scale_amount)
        h.addWidget(self._scale_center)

        self._envelope = Envelope()
        self._envelope.set_node_count_max(32)
        self._envelope.set_y_range(0, 1)
        self._envelope.set_x_range(0, 4)
        self._envelope.set_first_lock(True, False)
        self._envelope.set_last_lock(False, True)
        self._envelope.set_x_range_adjust(False, True)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addLayout(header_layout)
        v.addLayout(h)
        v.addWidget(self._envelope)
        self.setLayout(v)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_envelope()

        QObject.connect(
                self._enabled_toggle,
                SIGNAL('stateChanged(int)'),
                self._enabled_changed)
        QObject.connect(
                self._scale_amount,
                SIGNAL('numberChanged(float)'),
                self._scale_amount_changed)
        QObject.connect(
                self._scale_center,
                SIGNAL('numberChanged(float)'),
                self._scale_center_changed)
        QObject.connect(
                self._envelope,
                SIGNAL('envelopeChanged()'),
                self._envelope_changed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_instrument' in signals:
            self._update_envelope()

    def _update_envelope(self):
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        envelope = instrument.get_force_release_envelope()

        old_block = self._enabled_toggle.blockSignals(True)
        self._enabled_toggle.setCheckState(
                Qt.Checked if envelope['enabled'] else Qt.Unchecked)
        self._enabled_toggle.blockSignals(old_block)

        self._scale_amount.set_number(envelope['scale_amount'])
        self._scale_center.set_number(envelope['scale_center'])

        self._envelope.set_nodes(envelope['envelope']['nodes'])

    def _enabled_changed(self, state):
        new_enabled = (state == Qt.Checked)

        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        envelope = instrument.get_force_release_envelope()

        envelope['enabled'] = new_enabled

        instrument.set_force_release_envelope(envelope)
        self._updater.signal_update(set(['signal_instrument']))

    def _scale_number_changed(self, key, num):
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        envelope = instrument.get_force_release_envelope()

        envelope[key] = num

        instrument.set_force_release_envelope(envelope)
        self._updater.signal_update(set(['signal_instrument']))

    def _scale_amount_changed(self, num):
        self._scale_number_changed('scale_amount', num)

    def _scale_center_changed(self, num):
        self._scale_number_changed('scale_center', num)

    def _envelope_changed(self):
        new_nodes, _ = self._envelope.get_clear_changed()
        assert new_nodes

        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        envelope = instrument.get_force_release_envelope()

        envelope['envelope']['nodes'] = new_nodes
        envelope['enabled'] = True

        instrument.set_force_release_envelope(envelope)
        self._updater.signal_update(set(['signal_instrument']))


