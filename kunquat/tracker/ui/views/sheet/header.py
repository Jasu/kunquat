# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013-2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import print_function

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from config import *
import utils


class Header(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._col_width = DEFAULT_CONFIG['col_width']
        self._first_col = 0

        self._headers = []

    def set_config(self, config):
        self._config = config
        self._update_contents()

    def set_column_width(self, width):
        self._col_width = width
        self._update_contents()

    def set_first_column(self, num):
        self._first_col = num
        self._update_contents()

    def resizeEvent(self, ev):
        self._update_contents()

    def minimumSizeHint(self):
        w = self._headers[0] if self._headers else ColumnHeader()
        sh = w.minimumSizeHint()
        return QSize(self._col_width * 3, sh.height())

    def _resize_layout(self, max_visible_cols):
        visible_cols = utils.get_visible_cols(self._first_col, max_visible_cols)

        for i in xrange(len(self._headers), visible_cols):
            header = ColumnHeader()
            header.setParent(self)
            header.show()
            self._headers.append(header)
        for i in xrange(visible_cols, len(self._headers)):
            h = self._headers.pop()
            h.hide()

    def _update_contents(self):
        max_visible_cols = utils.get_max_visible_cols(self.width(), self._col_width)

        self._first_col = utils.clamp_start_col(self._first_col, max_visible_cols)

        self._resize_layout(max_visible_cols)

        # Update headers
        for i, header in enumerate(self._headers):
            header.set_config(self._config) # FIXME: bad idea
            header.set_column(self._first_col + i)
            header.move(i * self._col_width, 0)
            header.setFixedWidth(self._col_width)


class ColumnHeader(QWidget):

    def __init__(self):
        QWidget.__init__(self)

    def set_config(self, config):
        self._config = config

    def set_column(self, num):
        self._num = num

        fm = QFontMetrics(self._config['font'], self)
        digit_count = len(str(num))
        rect = fm.tightBoundingRect('8' * digit_count)
        baseline_offset = rect.height()
        self._pixmap = QPixmap(rect.size())

        painter = QPainter(self._pixmap)
        painter.setBackground(self._config['bg_colour'])
        painter.setPen(self._config['fg_colour'])
        painter.setFont(self._config['font'])
        painter.eraseRect(0, 0, self._pixmap.width(), self._pixmap.height())
        painter.drawText(QPoint(0, baseline_offset), str(num))

        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)

        # Background
        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(0, 0, self.width(), self.height())

        # Number
        num_width = self._pixmap.width()
        x_offset = (self.width() - num_width) // 2
        painter.drawPixmap(x_offset, 0, self._pixmap)

        # Border
        painter.setPen(self._config['border_colour'])
        painter.drawLine(self.width() - 1, 0, self.width() - 1, self.height() - 1)

    def minimumSizeHint(self):
        fm = QFontMetrics(self._config['font'], self)
        height = fm.tightBoundingRect('Ag').height()
        return QSize(10, height)


