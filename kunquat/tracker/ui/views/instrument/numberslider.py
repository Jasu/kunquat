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


class NumberSlider(QWidget):

    numberChanged = pyqtSignal(float, name='numberChanged')

    def __init__(self, decimal_count, min_val, max_val, title=''):
        QWidget.__init__(self)

        assert decimal_count >= 0

        self._decimal_count = decimal_count
        self._scale = 10**decimal_count

        self._slider = QSlider()
        self._slider.setOrientation(Qt.Horizontal)
        self._slider.setMinimum(int(min_val * self._scale))
        self._slider.setMaximum(int(max_val * self._scale))

        self._value = QLabel()
        fm = QFontMetrics(QFont())
        val_fmt = self._get_val_fmt()
        width = max(fm.width(val_fmt.format(val)) for val in (min_val, max_val))
        self._value.setFixedWidth(width)

        h = QHBoxLayout()
        if title:
            h.addWidget(QLabel(title))
        h.addWidget(self._slider)
        h.addWidget(self._value)
        self.setLayout(h)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Maximum)

        QObject.connect(self._slider, SIGNAL('valueChanged(int)'), self._number_changed)

        self.set_number(min_val)

    def set_number(self, num):
        old_block = self._slider.blockSignals(True)
        int_val = int(round(num * self._scale))
        self._slider.setValue(int_val)
        self._slider.blockSignals(old_block)

        val_fmt = self._get_val_fmt()
        self._value.setText(val_fmt.format(num))

    def _get_val_fmt(self):
        return '{{:.{}f}}'.format(self._decimal_count)

    def _number_changed(self, int_val):
        val = int_val / float(self._scale)
        QObject.emit(self, SIGNAL('numberChanged(float)'), val)

    def sizeHint(self):
        return self._slider.sizeHint()


