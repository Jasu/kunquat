# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import division
from __future__ import print_function

from PyQt4 import QtGui, QtCore

import timestamp as ts


class TimestampSpin(QtGui.QWidget):

    ts_changed = QtCore.pyqtSignal(int, int, name='tsChanged')

    def __init__(self,
                 project,
                 label,
                 key,
                 val_range,
                 dict_key=None,
                 decimals=0,
                 parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._key = key
        self._dict_key = dict_key
        self._decimals = decimals
        if dict_key:
            value = project[key][dict_key]
        else:
            value = project[key]
        value = ts.Timestamp(value)

        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        lab = QtGui.QLabel(label)
        layout.addWidget(lab, 0)

        self._spin = QtGui.QDoubleSpinBox()
        self._spin.setMinimum(float(val_range[0]))
        self._spin.setMaximum(float(val_range[1]))
        self._spin.setDecimals(decimals)
        QtCore.QObject.connect(self._spin,
                               QtCore.SIGNAL('valueChanged(double)'),
                               self.value_changed)
        self._lock_update = True
        self._spin.setValue(float(value))
        self._lock_update = False
        layout.addWidget(self._spin, 0)

    def set_key(self, key):
        if self._dict_key:
            value = self._project[key][self._dict_key]
        else:
            value = self._project[key]
        self._lock_update = True
        self._spin.setValue(float(ts.Timestamp(value)))
        self._lock_update = False
        self._key = key

    def value_changed(self, fvalue):
        if self._lock_update:
            return
        value = list(ts.Timestamp(fvalue))
        if self._dict_key:
            d = self._project[self._key]
            d[self._dict_key] = value
            self._project[self._key] = d
        else:
            self._project[self._key] = value
        QtCore.QObject.emit(self, QtCore.SIGNAL('tsChanged(int, int)'),
                                                value[0], value[1])


