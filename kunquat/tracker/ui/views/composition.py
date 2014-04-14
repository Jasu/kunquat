# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2014
#          Toni Ruottu, Finland 2013-2014
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

from sheet.sheet import Sheet


class Composition(QFrame):

    def __init__(self):
        QFrame.__init__(self)
        self._ui_model = None
        self._sheet = Sheet()

        v = QVBoxLayout()
        v.addWidget(self._sheet)
        self.setLayout(v)

        self.setMinimumHeight(320)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._sheet.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._sheet.unregister_updaters()

