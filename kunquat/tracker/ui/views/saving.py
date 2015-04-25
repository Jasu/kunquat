# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2015
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


def get_module_save_path():
    module_path_qstring = QFileDialog.getSaveFileName(
            caption='Save Kunquat Composition',
            filter='Kunquat compositions (*.kqt *.kqt.gz *.kqt.bz2)')
    if not module_path_qstring:
        return None
    module_path = str(module_path_qstring.toUtf8())
    return module_path


