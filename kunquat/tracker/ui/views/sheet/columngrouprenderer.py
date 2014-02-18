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

from __future__ import print_function
from itertools import islice, izip, izip_longest

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import kunquat.tracker.ui.model.tstamp as tstamp
from config import *
import utils
from trigger_renderer import TriggerRenderer


class ColumnGroupRenderer():

    """Manages rendering of column n for each pattern.

    """

    def __init__(self, num):
        self._num = num

        self._width = DEFAULT_CONFIG['col_width']
        self._px_offset = 0
        self._px_per_beat = DEFAULT_CONFIG['px_per_beat']

        self._heights = []
        self._start_heights = []

    def set_config(self, config):
        self._config = config

    def set_width(self, width):
        if self._width != width:
            self._width = width

    def set_columns(self, columns):
        self._columns = columns
        for i, cache in enumerate(self._caches):
            cache.set_column(self._columns[i])

    def set_pattern_lengths(self, lengths):
        self._lengths = lengths

    def set_pattern_heights(self, heights, start_heights):
        self._heights = heights
        self._start_heights = start_heights
        self._create_caches()

    def _create_caches(self):
        self._caches = [ColumnCache(self._num, i)
                for i in xrange(len(self._heights))]
        for cache in self._caches:
            cache.set_config(self._config)
        self._sync_caches()

    def _sync_caches(self):
        for cache in self._caches:
            cache.set_width(self._width)
            cache.set_px_per_beat(self._px_per_beat)

    def set_px_per_beat(self, px_per_beat):
        if self._px_per_beat != px_per_beat:
            self._px_per_beat = px_per_beat
            self._sync_caches()

    def set_px_offset(self, px_offset):
        if self._px_offset != px_offset:
            self._px_offset = px_offset
            self._sync_caches()

    def get_memory_usage(self):
        try:
            return sum(cache.get_memory_usage() for cache in self._caches)
        except AttributeError:
            return 0

    def draw(self, painter, height):
        # Render columns of visible patterns
        first_index = utils.get_first_visible_pat_index(
                self._px_offset,
                self._start_heights)

        pixmaps_created = 0

        # FIXME: contains some copypasta from Ruler.paintEvent

        overlap = None
        max_tr_width = self._width - 1

        rel_end_height = 0 # empty song

        for pi in xrange(first_index, len(self._heights)):
            if self._start_heights[pi] > self._px_offset + height:
                break

            # Current pattern offset and height
            rel_start_height = self._start_heights[pi] - self._px_offset
            rel_end_height = rel_start_height + self._heights[pi]
            cur_offset = max(0, -rel_start_height)

            # Draw pixmaps
            canvas_y = max(0, rel_start_height)
            cache = self._caches[pi]
            for (src_rect, pixmap) in cache.iter_pixmaps(
                    cur_offset, min(rel_end_height, height) - canvas_y):
                dest_rect = QRect(0, canvas_y, self._width, src_rect.height())
                painter.drawPixmap(dest_rect, pixmap, src_rect)
                canvas_y += src_rect.height()

            pixmaps_created += cache.get_pixmaps_created()

            # Draw overlapping part of previous pattern
            if overlap:
                src_rect, image = overlap

                # Prevent from drawing over the first trigger row
                first_tr = cache.get_first_trigger_row()
                if first_tr:
                    first_ts, _ = first_tr
                    first_rems = first_ts.beats * tstamp.BEAT + first_ts.rem
                    first_start_y = first_rems * self._px_per_beat // tstamp.BEAT
                    first_start_y += rel_start_height
                    src_rect_stop_y = rel_start_height + src_rect.height()
                    if src_rect_stop_y > first_start_y:
                        tr_overlap = src_rect_stop_y - first_start_y
                        src_rect.setHeight(src_rect.height() - tr_overlap)

                width = min(max_tr_width, src_rect.width())
                dest_rect = QRect(
                        0, rel_start_height,
                        width, src_rect.height())
                src_rect.setWidth(width)
                painter.drawImage(dest_rect, image, src_rect)
                overlap = None

            # Find trigger row that overlaps with next pattern
            last_tr = cache.get_last_trigger_row(self._lengths[pi])
            if last_tr:
                last_ts, last_image = last_tr
                last_rems = last_ts.beats * tstamp.BEAT + last_ts.rem
                last_start_y = last_rems * self._px_per_beat // tstamp.BEAT
                last_stop_y = last_start_y + self._config['tr_height']
                if last_stop_y >= self._heights[pi]:
                    # + 1 is the shared pixel row between patterns
                    rect_height = last_stop_y - self._heights[pi] + 1
                    rect_start = self._config['tr_height'] - rect_height
                    rect = QRect(
                            0, rect_start,
                            last_image.rect().width(), rect_height)
                    overlap = rect, last_image
        else:
            # Fill trailing blank
            painter.setBackground(self._config['canvas_bg_colour'])
            painter.eraseRect(
                    QRect(
                        0, rel_end_height,
                        self._width, height - rel_end_height)
                    )

            # Draw trigger row that extends beyond the last pattern
            if overlap:
                src_rect, image = overlap
                # Last pattern and blank do not share pixel rows
                src_rect.setY(src_rect.y() + 1)
                width = min(max_tr_width, src_rect.width())
                dest_rect = QRect(
                        0, rel_end_height,
                        width, src_rect.height())
                src_rect.setWidth(width)
                painter.drawImage(dest_rect, image, src_rect)

        # Testing
        """
        painter.eraseRect(0, 0, self._col_width, height)
        painter.setPen(Qt.white)
        painter.drawRect(0, 0, self._col_width - 1, height - 1)
        painter.drawText(QPoint(2, 12), str(self._num))
        """

        return pixmaps_created


class ColumnCache():

    PIXMAP_HEIGHT = 256

    def __init__(self, col_num, pat_num):
        self._col_num = col_num
        self._pat_num = pat_num

        self._pixmaps = {}
        self._pixmaps_created = 0

        self._tr_cache = TRCache()

        self._width = DEFAULT_CONFIG['col_width']
        self._px_per_beat = DEFAULT_CONFIG['px_per_beat']

    def set_config(self, config):
        self._config = config
        self._pixmaps = {}
        self._tr_cache.set_config(config)

    def set_column(self, column):
        self._column = column
        self._tr_cache.set_triggers(column)

    def set_width(self, width):
        if self._width != width:
            self._width = width
            self._pixmaps = {}

    def set_px_per_beat(self, px_per_beat):
        assert px_per_beat > 0
        if self._px_per_beat != px_per_beat:
            self._px_per_beat = px_per_beat
            self._pixmaps = {}

    def get_memory_usage(self):
        tr_memory_usage = self._tr_cache.get_memory_usage()
        for pixmap in self._pixmaps.values():
            bpp = pixmap.depth()
            px_per_map = self._width * ColumnCache.PIXMAP_HEIGHT
            return tr_memory_usage + len(self._pixmaps) * px_per_map * (bpp // 8)
        else:
            return tr_memory_usage

    def iter_pixmaps(self, start_px, height_px):
        assert start_px >= 0
        assert height_px >= 0

        stop_px = start_px + height_px

        # Get pixmap indices
        start_index = start_px // ColumnCache.PIXMAP_HEIGHT
        stop_index = 1 + (start_px + height_px - 1) // ColumnCache.PIXMAP_HEIGHT

        self._pixmaps_created = 0

        for i in xrange(start_index, stop_index):
            if i not in self._pixmaps:
                self._pixmaps[i] = self._create_pixmap(i)
                self._pixmaps_created += 1

            rect = utils.get_pixmap_rect(
                    i,
                    start_px, stop_px,
                    self._width,
                    ColumnCache.PIXMAP_HEIGHT)

            yield (rect, self._pixmaps[i])

    def get_pixmaps_created(self):
        return self._pixmaps_created

    def _create_pixmap(self, index):
        pixmap = QPixmap(self._width, ColumnCache.PIXMAP_HEIGHT)

        painter = QPainter(pixmap)

        # Background
        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(QRect(0, 0, self._width - 1, ColumnCache.PIXMAP_HEIGHT))

        # Start and stop timestamps
        start_px = index * ColumnCache.PIXMAP_HEIGHT
        stop_px = (index + 1) * ColumnCache.PIXMAP_HEIGHT

        visible_tr_start_px = start_px - self._config['tr_height'] + 1
        start_ts = tstamp.Tstamp(0,
                visible_tr_start_px * tstamp.BEAT // self._px_per_beat)
        stop_ts = tstamp.Tstamp(0,
                stop_px * tstamp.BEAT // self._px_per_beat)

        def ts_to_y_offset(ts):
            rems = ts.beats * tstamp.BEAT + ts.rem
            abs_y = rems * self._px_per_beat // tstamp.BEAT
            y_offset = abs_y - start_px
            return y_offset

        # Trigger rows
        painter.setCompositionMode(QPainter.CompositionMode_SourceOver)
        for ts, image, next_ts in self._tr_cache.iter_images(start_ts, stop_ts):
            y_offset = ts_to_y_offset(ts)

            src_rect = image.rect()
            dest_rect = src_rect.translated(QPoint(0, y_offset))

            if next_ts != None:
                next_y_offset = ts_to_y_offset(next_ts)
                y_dist = next_y_offset - y_offset
                if y_dist < dest_rect.height():
                    rect_height = max(1, y_dist)
                    dest_rect.setHeight(rect_height)
                    src_rect.setHeight(rect_height)

            painter.drawImage(dest_rect, image, src_rect)

        # Border
        painter.setPen(self._config['border_colour'])
        painter.drawLine(
                QPoint(self._width - 1, 0),
                QPoint(self._width - 1, ColumnCache.PIXMAP_HEIGHT))

        # Testing
        """
        painter.setBackground(Qt.black)
        painter.eraseRect(QRect(0, 0, self._width, ColumnCache.PIXMAP_HEIGHT))
        painter.setPen(Qt.white)
        painter.drawRect(0, 0, self._width - 1, ColumnCache.PIXMAP_HEIGHT - 1)
        pixmap_desc = '{}-{}-{}'.format(self._col_num, self._pat_num, index)
        painter.drawText(QPoint(2, 12), pixmap_desc)
        """

        return pixmap

    def get_first_trigger_row(self):
        return self._tr_cache.get_first_trigger_row()

    def get_last_trigger_row(self, max_ts):
        return self._tr_cache.get_last_trigger_row(max_ts)


class TRCache():

    def __init__(self):
        self._images = {}

    def set_config(self, config):
        self._config = config
        self._images = {}

    def set_triggers(self, column):
        self._rows = self._build_trigger_rows(column)
        self._images = {} # TODO: only remove out-of-date images

    def _build_trigger_rows(self, column):
        trs = {}
        for ts in column.get_trigger_row_positions():
            trow = [column.get_trigger(ts, i)
                    for i in xrange(column.get_trigger_count_at_row(ts))]
            trs[ts] = trow

        trlist = list(trs.items())
        trlist.sort()
        return trlist

    def iter_images(self, start_ts, stop_ts):
        images_created = 0

        next_tstamps = (row[0] for row in islice(self._rows, 1, None))

        for row, next_ts in izip_longest(self._rows, next_tstamps):
            ts, triggers = row
            if ts < start_ts:
                continue
            elif ts >= stop_ts:
                break
            if ts not in self._images:
                self._images[ts] = self._create_image(triggers)
                images_created += 1
            yield (ts, self._images[ts], next_ts)

        if images_created > 0:
            print('{} trigger row image{} created'.format(
                images_created, 's' if images_created != 1 else ''))

    def get_memory_usage(self):
        total_byte_count = 0
        for image in self._images.values():
            total_byte_count += image.byteCount()
        return total_byte_count

    def _create_image(self, triggers):
        rends = [TriggerRenderer(self._config, t) for t in triggers]
        widths = [r.get_total_width() for r in rends]

        image = QImage(
                sum(widths),
                self._config['tr_height'],
                QImage.Format_ARGB32)
        image.fill(0)

        painter = QPainter(image)
        painter.setCompositionMode(QPainter.CompositionMode_Plus)
        for renderer, width in izip(rends, widths):
            renderer.draw_trigger(painter)
            painter.setTransform(QTransform().translate(width, 0), True)

        # Testing
        """
        painter.setBackground(Qt.black)
        painter.eraseRect(QRect(0, 0, image.width(), image.height()))
        painter.setPen(Qt.red)
        painter.drawRect(QRect(0, 0, image.width() - 1, image.height() - 1))
        painter.setTransform(QTransform().rotate(-45))
        for i in xrange(4):
            side = self._config['tr_height']
            painter.fillRect(QRect(i * side * 2, 0, side, (i + 1) * side * 3), Qt.red)
        """

        return image

    def get_first_trigger_row(self):
        if self._rows:
            ts, _ = self._rows[0]
            if ts not in self._images:
                return None
            return ts, self._images[ts]
        return None

    def get_last_trigger_row(self, max_ts):
        for row in reversed(self._rows):
            ts, _ = row
            if ts <= max_ts:
                if ts not in self._images:
                    # We haven't rendered the trigger row yet,
                    # so it must be outside the view
                    return None
                return ts, self._images[ts]
        return None


