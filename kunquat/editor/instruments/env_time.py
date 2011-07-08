# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2011
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import division, print_function

from PyQt4 import QtCore, QtGui

import kunquat.editor.kqt_limits as lim
from kunquat.editor.envelope import Envelope
from kunquat.editor.param_check import ParamCheck
from kunquat.editor.param_slider import ParamSlider


class EnvTime(QtGui.QWidget):

    def __init__(self,
                 project,
                 key_base,
                 name,
                 envelope,
                 parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._cur_inst = 0
        self._project = project
        self._key_base = key_base
        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)

        top_layout = QtGui.QHBoxLayout()
        top_layout.setMargin(0)
        top_layout.setSpacing(0)
        label = QtGui.QLabel(name + ':')
        self._enabled = ParamCheck(project,
                                   'Enabled',
                                   False,
                                   self._key_base.format(self._cur_inst),
                                   'enabled')
        self._scale_amount = ParamSlider(project,
                                         'Scale amount:',
                                         (-16, 16),
                                         0,
                                         self._key_base.format(self._cur_inst),
                                         'scale_amount',
                                         decimals=2)
        self._scale_center = ParamSlider(project,
                                         'Scale center:',
                                         (-4800, 4800),
                                         0,
                                         self._key_base.format(self._cur_inst),
                                         'scale_center')
        top_layout.addSpacing(10)
        top_layout.addWidget(label)
        top_layout.addSpacing(10)
        top_layout.addWidget(self._enabled)
        top_layout.addWidget(self._scale_amount)
        top_layout.addWidget(self._scale_center)
        self._env = envelope
        layout.addLayout(top_layout)
        layout.addWidget(self._env, 1)

        self._widgets = [self._enabled,
                         self._scale_amount,
                         self._scale_center,
                         self._env]

    def inst_changed(self, num):
        self._cur_inst = num
        for widget in self._widgets:
            widget.set_key(self._key_base.format(self._cur_inst))

    def sync(self):
        for widget in self._widgets:
            widget.sync()


