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


class ParamSlider(QtGui.QWidget):

    def __init__(self,
                 project,
                 label,
                 key,
                 val_range,
                 dict_key=None,
                 decimals=0,
                 orientation=QtCore.Qt.Horizontal,
                 parent=None):
        assert orientation in (QtCore.Qt.Horizontal, QtCore.Qt.Vertical)
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._factor = 10**decimals
        self._key = key
        self._dict_key = dict_key
        if dict_key:
            value = project[key][dict_key]
        else:
            value = project[key]

        if orientation == QtCore.Qt.Horizontal:
            layout = QtGui.QHBoxLayout(self)
        else:
            layout = QtGui.QVBoxLayout(self)
        lab = QtGui.QLabel(label)
        layout.addWidget(lab, 0)

        self._slider = QtGui.QSlider(orientation)
        self._slider.setRange(val_range[0] * self._factor,
                              val_range[1] * self._factor)
        layout.addWidget(self._slider)
        self._slider.setValue(int(round(value * self._factor)))
        QtCore.QObject.connect(self._slider,
                               QtCore.SIGNAL('valueChanged(int)'),
                               self.value_changed)

        self._value_display = QtGui.QLabel(str(value))
        metrics = QtGui.QFontMetrics(QtGui.QFont())
        min_str = '{0:.{1}f}'.format(val_range[0], decimals)
        max_str = '{0:.{1}f}'.format(val_range[1], decimals)
        width = max(metrics.width(min_str),
                    metrics.width(max_str))
        self._value_display.setFixedWidth(width)
        layout.addWidget(self._value_display)

    def value_changed(self, svalue):
        value = svalue / self._factor
        if self._dict_key:
            d = self._project[self._key]
            d[self._dict_key] = value
            self._project[self._key] = d
        else:
            self._project[self._key] = value
        self._value_display.setText(str(value))


