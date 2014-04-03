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

from sheetarea import SheetArea
from toolbar import Toolbar


class Sheet(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._toolbar = Toolbar()
        self._sheet_area = SheetArea()

        v = QVBoxLayout()
        v.setMargin(0)
        v.addWidget(self._toolbar)
        v.addWidget(self._sheet_area)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._toolbar.set_ui_model(ui_model)
        self._sheet_area.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._sheet_area.unregister_updaters()
        self._toolbar.unregister_updaters()


