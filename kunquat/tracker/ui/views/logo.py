# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2014
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
from PyQt4.QtSvg import QSvgRenderer


class Logo(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self.setMinimumSize(200, 200)
        self.setMaximumSize(200, 200)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def paintEvent(self, ev):
        if self._ui_model:
            logo_painter = QPainter(self)
            icon_bank = self._ui_model.get_icon_bank()
            logo_path = icon_bank.get_kunquat_logo_path()
            logo_renderer = QSvgRenderer(logo_path)
            logo_renderer.render(logo_painter)

    def unregister_updaters(self):
        pass


