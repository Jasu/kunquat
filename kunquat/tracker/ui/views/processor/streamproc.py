# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
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


class StreamProc(QWidget):

    @staticmethod
    def get_name():
        return u'Stream'

    def __init__(self):
        QWidget.__init__(self)

        v = QVBoxLayout()
        v.addWidget(QLabel('todo'))
        v.addStretch(1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        pass

    def set_proc_id(self, proc_id):
        pass

    def set_ui_model(self, ui_model):
        pass

    def unregister_updaters(self):
        pass


