# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

import kunquat.tracker.cmdline as cmdline
from .playbackposition import PlaybackPosition
from .updater import Updater


class PlaybackPanel(QToolBar, Updater):

    def __init__(self):
        super().__init__()
        self._play_button = PlayButton()
        self._play_pattern_button = PlayPatternButton()
        self._play_from_cursor_button = PlayFromCursorButton()
        self._record_button = RecordButton()
        self._silence_button = SilenceButton()
        self._playback_pos = PlaybackPosition()
        self._interactivity_button = InteractivityButton()

        self.add_to_updaters(
                self._play_button,
                self._play_pattern_button,
                self._play_from_cursor_button,
                self._record_button,
                self._silence_button,
                self._playback_pos,
                self._interactivity_button)

        self.addWidget(self._play_button)
        self.addWidget(self._play_pattern_button)
        self.addWidget(self._play_from_cursor_button)
        if cmdline.get_experimental():
            self.addWidget(self._record_button)
        self.addWidget(self._silence_button)
        self.addSeparator()
        self.addWidget(self._playback_pos)
        self.addSeparator()
        self.addWidget(self._interactivity_button)

    def addWidget(self, widget):
        if isinstance(widget, QToolButton):
            widget.setFocusPolicy(Qt.NoFocus)
        super().addWidget(widget)


class PlayButton(QToolButton, Updater):

    def __init__(self):
        super().__init__()
        self.setText('Play')
        self.setToolTip('Play (Comma)')
        self.setAutoRaise(True)

    def _on_setup(self):
        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('play')
        icon = QIcon(icon_path)
        self.setIcon(icon)
        self.clicked.connect(self._ui_model.play)


class PlayPatternButton(QToolButton, Updater):

    def __init__(self):
        super().__init__()
        self.setText('Play Pattern')
        self.setToolTip('Play Pattern (Ctrl + Comma)')
        self.setAutoRaise(True)

    def _on_setup(self):
        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('play_pattern')
        icon = QIcon(icon_path)
        self.setIcon(icon)
        self.clicked.connect(self._ui_model.play_pattern)


class PlayFromCursorButton(QToolButton, Updater):

    def __init__(self):
        super().__init__()
        self.setText('Play from Cursor')
        self.setToolTip('Play from Cursor (Alt + Comma)')
        self.setAutoRaise(True)

    def _on_setup(self):
        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('play_from_cursor')
        icon = QIcon(icon_path)
        self.setIcon(icon)
        self.clicked.connect(self._ui_model.play_from_cursor)


class RecordButton(QToolButton, Updater):

    def __init__(self):
        super().__init__()
        self._sheet_mgr = None
        self._playback_mgr = None

        self.setCheckable(True)
        self.setText('Record')
        self.setAutoRaise(True)

    def _on_setup(self):
        self.register_action('signal_record_mode', self._update_checked)

        self._sheet_mgr = self._ui_model.get_sheet_manager()
        self._playback_mgr = self._ui_model.get_playback_manager()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('record')
        icon = QIcon(icon_path)
        self.setIcon(icon)

        self.clicked.connect(self._clicked)

    def _update_checked(self):
        old_block = self.blockSignals(True)
        is_checked = self._playback_mgr.is_recording()
        self.setChecked(is_checked)
        self.blockSignals(old_block)

    def _clicked(self):
        if self._playback_mgr.is_recording():
            self._playback_mgr.stop_recording()
        else:
            self._playback_mgr.start_recording()
            self._sheet_mgr.set_typewriter_connected(True)
            self._ui_model.play()


class SilenceButton(QToolButton, Updater):

    def __init__(self):
        super().__init__()
        self._playback_mgr = None

        self.setText('Silence')
        self.setToolTip('Silence (Period)')
        self.setAutoRaise(True)

    def _on_setup(self):
        self._playback_mgr = self._ui_model.get_playback_manager()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('silence')
        icon = QIcon(icon_path)
        self.setIcon(icon)

        self.clicked.connect(self._clicked)

    def _clicked(self):
        self._playback_mgr.stop_recording()
        self._ui_model.silence()


class InteractivityButton(QToolButton, Updater):

    def __init__(self):
        super().__init__()
        self.setText('Interactivity')

    def _on_setup(self):
        self.clicked.connect(self._show_ia_window)

    def _show_ia_window(self):
        visibility_mgr = self._ui_model.get_visibility_manager()
        visibility_mgr.show_interactivity_controls()


