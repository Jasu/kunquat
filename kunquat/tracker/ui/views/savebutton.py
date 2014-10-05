# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014
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


class SaveButton(QToolButton):

    def __init__(self):
        QToolButton.__init__(self)
        self._ui_model = None
        self._updater = None

        self.setText('Save')
        self.setEnabled(False)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_module' in signals:
            self.setEnabled(True)

    def _clicked(self):
        module = self._ui_model.get_module()

        if not module.get_path():
            module_path = unicode(QFileDialog.getSaveFileName(
                    caption='Save Kunquat Composition',
                    filter='Kunquat compositions (*.kqt *.kqt.gz *.kqt.bz2)'))
            if not module_path:
                return
            module.set_path(module_path)

        module.start_save()


