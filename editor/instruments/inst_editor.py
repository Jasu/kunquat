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

from __future__ import print_function

from PyQt4 import QtCore, QtGui

import kqt_limits as lim


class InstEditor(QtGui.QWidget):

    def __init__(self, project, instrument_spin, parent=None):
        QtGui.QWidget.__init__(self, parent)

        self._project = project
        self._cur_inst = instrument_spin.value()
        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)

        load = QtGui.QPushButton('Load')

        remove = QtGui.QPushButton('Remove')

        layout.addWidget(load, 0)
        QtCore.QObject.connect(load,
                               QtCore.SIGNAL('clicked()'),
                               self.load)
        layout.addWidget(remove, 0)
        QtCore.QObject.connect(remove,
                               QtCore.SIGNAL('clicked()'),
                               self.remove)

        QtCore.QObject.connect(instrument_spin,
                               QtCore.SIGNAL('valueChanged(int)'),
                               self.inst_changed)

    def inst_changed(self, num):
        self._cur_inst = num

    def load(self):
        print('load', self._cur_inst)

    def remove(self):
        self._project.remove_dir('ins_{0:02x}'.format(self._cur_inst))


