# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010-2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4 import QtCore, QtGui

from inst_list import InstList
from itertools import cycle
import kunquat.tracker.kqt_limits as lim

class Instruments(QtGui.QWidget):

    def __init__(self,
                 p,
                 tw,
                 piano,
                 project,
                 instrument_spin,
                 playback_manager,
                 note_input,
                 scale,
                 octave_spin,
                 parent=None):
        QtGui.QWidget.__init__(self, parent)
        self.p = p

        self._tw = tw
        self._piano = piano
        self._project = project
        self._inst_list = InstList(self.p, project, instrument_spin)
        self._instrument_spin = instrument_spin
        self._playback_manager = playback_manager
        self._note_input = note_input
        self._scale = scale
        self._octave_spin = octave_spin

        edit_button = QtGui.QPushButton()
        edit_button.setText('Edit')
        QtCore.QObject.connect(edit_button, QtCore.SIGNAL('clicked()'),
                               self.edit)
        load = QtGui.QPushButton('Import')
        QtCore.QObject.connect(load,
                               QtCore.SIGNAL('clicked()'),
                               self.load)
        save = QtGui.QPushButton('Export')
        QtCore.QObject.connect(save,
                               QtCore.SIGNAL('clicked()'),
                               self.save)
        remove = QtGui.QPushButton('Remove')
        QtCore.QObject.connect(remove,
                               QtCore.SIGNAL('clicked()'),
                               self.remove)

        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)


        buttons = QtGui.QWidget()
        button_layout = QtGui.QVBoxLayout(buttons)
        button_layout.setMargin(0)
        button_layout.setSpacing(0)
        layout.addWidget(self._inst_list)
        layout.addWidget(buttons)
        button_layout.addWidget(load, 0)
        button_layout.addWidget(save, 0)
        button_layout.addWidget(edit_button)
        button_layout.addWidget(remove, 0)

        self._inst_num = 0
        self._channel = cycle(xrange(lim.COLUMNS_MAX))

        QtCore.QObject.connect(instrument_spin,
                               QtCore.SIGNAL('currentIndexChanged (const QString&)'),
                               self.inst_changed)
        QtCore.QObject.connect(octave_spin,
                               QtCore.SIGNAL('valueChanged(int)'),
                               self.octave_changed)

    def load(self):
        slot = 0
        ids = self.p.project._composition.instrument_ids()
        numbers = [int(i.split('_')[1]) for i in ids]
        while slot in numbers:
            slot += 1
        fname = QtGui.QFileDialog.getOpenFileName(
                caption='Load Kunquat instrument (to index {0})'.format(slot),
                filter='Kunquat instruments (*.kqti *.kqti.gz *.kqti.bz2)')
        if fname:
            self._project.import_kqti(slot, str(fname))

    def save(self):
        slot = self._inst_num
        fname = QtGui.QFileDialog.getSaveFileName(
                caption='Save Kunquat instrument (of index {0})'.format(slot),
                filter='Kunquat instruments (*.kqti *.kqti.gz *.kqti.bz2)')
        if fname:
            self._project.export_kqti(slot, str(fname))

    def edit(self):
        slot = self._inst_num
        ins_id = self.get_ins_id(slot)
        self.p.instrument_window(ins_id)

    def remove(self):
        slot = self._inst_num
        inst = self._project._composition.get_instrument(slot)
        self.p.kill_instrument_window(inst.get_id())
        inst.delete()

    def get_ins_id(self, slot):
        return 'ins_%0.2d' % slot

    def init(self):
        self._inst_list.init()

    def inst_changed(self, text):
        if text == '':
            return
        parts = text.split(':')
        number = int(parts[0] )
        self._inst_num = number

    def octave_changed(self, num):
        self._note_input.base_octave = num
        self._tw.update()

    def keyPressEvent(self, ev):
        self._tw.keyPressEvent(ev)

    def keyReleaseEvent(self, ev):
        self._tw.keyReleaseEvent(ev)

    def sync(self):
        self._inst_list.sync()
