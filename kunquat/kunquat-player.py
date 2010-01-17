# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import print_function
import math
from optparse import OptionParser, OptionGroup
import os.path
import string
import sys
import termios

import kunquat
import pulseaudio


PROGRAM_NAME = 'kunquat-player'
PROGRAM_VERSION = '0.3.1'
AUTHORS = [ u'Tomi Jylhä-Ollila' ]


def print_version():
    print(PROGRAM_NAME + ' ' + PROGRAM_VERSION)
    if len(AUTHORS) == 1:
        auth = 'Author: '
    else:
        auth = 'Authors: '
    print(auth + AUTHORS[0])
    for author in AUTHORS[1:]:
        print(''.join([' '] * len(auth)) + author)
    print('No rights reserved')
    print('CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/')


def print_interactive_help():
    help = ['\nSupported keys in interactive mode:',
            '  `h`          Show this help.',
            '  Space        Pause/resume playback.',
            '  Left         Seek backwards 10 seconds.',
            '  Right        Seek forwards 10 seconds.',
            '  [0-9], `a`   Select subsong (`a` plays all subsongs).',
            '  `p`          Select previous subsong.',
            '  `n`          Select next subsong.',
            '  Backspace    Select previous file.',
            '  Return       Select next file.',
            '  `q`          Quit.']
    print('\n'.join(help))


def play(options, file, pa_handle, key_handler, list_pos):
    if os.path.basename(os.path.realpath(file)) == 'kunquatc00':
        handle = kunquat.RWHandle(file, options.frequency)
    else:
        handle = kunquat.RHandle(file, options.frequency)
    handle.buffer_size = options.buffer_size
    handle.subsong = options.subsong
    status = Status(options.frequency)
    duration = handle.get_duration(options.subsong)
    if options.interactive:
        print('Playing ' + file)
    return_key = KeyHandler.RETURN
    status_len_max = 0
    bufs = handle.mix()
    while bufs[0]:
        pa_handle.write(*bufs)
        if options.interactive:
            status_line = status.get_status_line(handle, duration, bufs)
            print(status_line, end='')
            status_len = len(status_line)
            if status_len < status_len_max:
                print(''.join([' '] * (status_len_max - status_len)), end='')
            else:
                status_len_max = status_len
            print(end='\r')
            key = key_handler.get_key()
            if key:
                if key == 'q':
                    return_key = key
                    break
                elif key == KeyHandler.BACKSPACE:
                    if 'first' not in list_pos:
                        return_key = key
                        break
                elif key == KeyHandler.RETURN:
                    if 'last' not in list_pos:
                        return_key = key
                        break
                elif key == 'h':
                    print_interactive_help()
                elif key == ' ':
                    print('\n --- PAUSE ---', end='\r')
                    res_key = key_handler.wait_key()
                    status_len_max = 14
                    if res_key == 'q':
                        return_key = 'q'
                        break
                elif key == KeyHandler.LEFT:
                    if handle.nanoseconds < 10000000000:
                        handle.nanoseconds = 0
                    else:
                        handle.nanoseconds = handle.nanoseconds - 10000000000
                elif key == KeyHandler.RIGHT:
                    handle.nanoseconds = handle.nanoseconds + 10000000000
                elif key == 'a':
                    handle.subsong = None
                elif key in string.digits:
                    if handle['subs_%02x/p_subsong.json' % int(key)]:
                        handle.subsong = int(key)
                elif key == 'p':
                    if handle.subsong == 0:
                        handle.subsong = None
                        duration = handle.get_duration(handle.subsong)
                    elif handle.subsong:
                        handle.subsong = handle.subsong - 1
                        duration = handle.get_duration(handle.subsong)
                elif key == 'n':
                    if handle.subsong is None:
                        handle.subsong = 0
                        duration = handle.get_duration(handle.subsong)
                    elif handle['subs_%02x/p_subsong.json' %
                                (handle.subsong + 1)] and handle.subsong < 255:
                        handle.subsong = handle.subsong + 1
                        duration = handle.get_duration(handle.subsong)
        bufs = handle.mix()
    pa_handle.drain()
    return return_key


def play_all(options, files):
    pa_handle = pulseaudio.Simple(PROGRAM_NAME, 'Music',
                                  options.frequency, 2)
    key_handler = KeyHandler()
    file_index = 0
    while file_index < len(files):
        file = files[file_index]
        list_pos = []
        if file_index == 0:
            list_pos.append('first')
        if file_index == len(files) - 1:
            list_pos.append('last')
        try:
            if options.interactive:
                key_handler.disable_blocking()
            action = play(options, file, pa_handle, key_handler, list_pos)
            if options.interactive:
                print()
            if action == 'q':
                break
            elif action == KeyHandler.BACKSPACE:
                if file_index > 0:
                    file_index = file_index - 1
            elif action == KeyHandler.RETURN:
                file_index = file_index + 1
        finally:
            if options.interactive:
                key_handler.restore()
    if options.interactive:
        print('Done.')


def main():
    usage = 'Usage: %prog [options] <files>'
    parser = OptionParser(usage)
    parser.add_option('-q', '--quiet',
            action='store_false', dest='interactive', default=True,
            help='Quiet and non-interactive operation'
                 ' (only error messages will be displayed).')
    parser.add_option('--version',
            action='store_true', dest='version', default=False,
            help='Display version information and exit.')

    output_opts = OptionGroup(parser, 'Output options')
    output_opts.add_option('--buffer-size', metavar='n',
            type='int', default=2048,
            help='Use audio buffer size of n frames.'
                 ' Valid range is [64,262144].')
    output_opts.add_option('--frequency', metavar='n',
            type='int', default=48000,
            help='Set mixing frequency to n frames/second.'
                 ' Valid range is [2000,192000].')
    parser.add_option_group(output_opts)

    play_opts = OptionGroup(parser, 'Playback options')
    play_opts.add_option('-s', '--subsong', metavar='sub',
            default='all',
            help='Play the subsong sub.'
                 ' Valid values are numbers in the range [0,255] and `all`.')
    parser.add_option_group(play_opts)

    parser.epilog = 'In addition to command line options, kunquat-player'\
                    ' supports several control keys in interactive mode.'\
                    ' Press `h` while playing to get the list of supported'\
                    ' keys.'
    options, files = parser.parse_args()
    if options.version:
        print_version()
        sys.exit()
    if not files:
        print('No input files specified. Use -h for help.')
        sys.exit(1)
    if 'all'.startswith(options.subsong):
        options.subsong = None
    else:
        options.subsong = int(options.subsong)
    try:
        play_all(options, files)
    except KeyboardInterrupt:
        print()


class KeyHandler(object):

    LEFT = 256
    DOWN = 257
    UP = 258
    RIGHT = 259
    RETURN = 260
    BACKSPACE = 261

    key_map = { '\033[A' : UP,
                '\033[B' : DOWN,
                '\033[C' : RIGHT,
                '\033[D' : LEFT,
                '\x0a' : RETURN,
                '\x7f' : BACKSPACE }

    def __init__(self):
        self.orig_term = termios.tcgetattr(sys.stdin)
        self.nb_term = termios.tcgetattr(sys.stdin)
        self.wait_term = termios.tcgetattr(sys.stdin)
        # Set lflags.
        self.wait_term[3] = self.nb_term[3] = \
                self.nb_term[3] & ~(termios.ICANON |
                                    termios.ECHO |
                                    termios.IEXTEN)
        # Set cc.
        self.nb_term[6][termios.VMIN] = 0
        self.nb_term[6][termios.VTIME] = 0

    def disable_blocking(self):
        termios.tcsetattr(sys.stdin, termios.TCSAFLUSH, self.nb_term)

    def wait_key(self):
        termios.tcsetattr(sys.stdin, termios.TCSAFLUSH, self.wait_term)
        key = sys.stdin.read(1)
        self.disable_blocking()
        return key

    def get_key(self):
        key = sys.stdin.read(3)
        if not key:
            return None
        if key in KeyHandler.key_map:
            return KeyHandler.key_map[key]
        return key

    def restore(self):
        termios.tcsetattr(sys.stdin, termios.TCSAFLUSH, self.orig_term)

    def __del__(self):
        self.restore()


class Status(object):

    clip_map = { (True, True)   : u'▌',
                 (True, False)  : u'▘',
                 (False, True)  : u'▖',
                 (False, False) : u' ' }
    
    block_map = { (3, 3) : u'█',
                  (3, 2) : u'▜',
                  (3, 1) : u'▛',
                  (3, 0) : u'▀',
                  (2, 3) : u'▟',
                  (2, 2) : u'▐',
                  (2, 1) : u'▞',
                  (2, 0) : u'▝',
                  (1, 3) : u'▙',
                  (1, 2) : u'▚',
                  (1, 1) : u'▌',
                  (1, 0) : u'▘',
                  (0, 3) : u'▄',
                  (0, 2) : u'▗',
                  (0, 1) : u'▖',
                  (0, 0) : u' ' }

    SILENCE = -384

    def __init__(self, freq):
        self.left_clipped = False
        self.right_clipped = False
        self.freq = freq
        self.hold_limit = freq
        self.left_hold = [Status.SILENCE, self.hold_limit]
        self.right_hold = [Status.SILENCE, self.hold_limit]

    def dur_to_str(self, duration):
        seconds = duration / 1000000000.0
        seconds = seconds - 0.05
        if seconds < 0:
            seconds = 0
        return '%02d:%04.1f' % (seconds // 60, seconds % 60)

    def get_status_line(self, handle, duration, bufs):
        components = []
        if handle.subsong is None:
            components.append('All subsongs')
        else:
            components.append('Subsong: ' + str(handle.subsong))
        elapsed = 'Time: ' + self.dur_to_str(handle.nanoseconds)
        if handle.nanoseconds <= duration:
            elapsed = (elapsed + ' [' +
                       self.dur_to_str(duration - handle.nanoseconds) + ']')
            elapsed = elapsed + ' of ' + self.dur_to_str(duration)
        components.append(elapsed)
        return self.get_peak_meter(14, -40, -4, bufs) + ', '.join(components)

    def get_peak_meter(self, length, lower, upper, bufs):
        if lower < Status.SILENCE:
            lower = Status.SILENCE
        left_vol_linear = max(bufs[0]) - min(bufs[0]) / 2
        right_vol_linear = max(bufs[1]) - min(bufs[1]) / 2
        self.left_clipped = (self.left_clipped or
                             len([a for a in bufs[0] if abs(a) > 1.0]) > 0)
        self.right_clipped = (self.right_clipped or
                              len([a for a in bufs[1] if abs(a) > 1.0]) > 0)
        left_bar = self.get_single_meter(length - 3, lower, upper,
                                         left_vol_linear, self.left_hold)
        right_bar = self.get_single_meter(length - 3, lower, upper,
                                          right_vol_linear, self.right_hold)
        self.left_hold[1] = self.left_hold[1] + len(bufs[0])
        self.right_hold[1] = self.right_hold[1] + len(bufs[0])
        return ('[' + self.combine_meters(left_bar, right_bar) + ']' +
                Status.clip_map[self.left_clipped, self.right_clipped])

    def get_single_meter(self, length, lower, upper, vol_linear, hold):
        if vol_linear <= 0:
            vol = Status.SILENCE
        else:
            vol = math.log(vol_linear, 2) * 6
        scale = length * 2
        scale_dB = upper - lower
        fill_length = int((vol - lower) * scale / scale_dB)
        if fill_length < 0:
            fill_length = 0
        elif fill_length > scale:
            fill_length = scale
        bar = [1] * fill_length + [0] * (scale - fill_length)
        if hold[1] >= self.hold_limit or hold[0] <= vol:
            hold[0] = vol
            hold[1] = 0
            hold_pos = fill_length - 1
        else:
            hold_pos = int((hold[0] - lower) * scale / scale_dB) - 1
        if hold_pos >= 0:
            if hold_pos >= scale:
                hold_pos = scale - 1
            bar[hold_pos] = 1
        return [a + 2 * b for a, b in zip(bar[0::2], bar[1::2])]

    def combine_meters(self, left_bar, right_bar):
        bar = zip(left_bar, right_bar)
        return ''.join([Status.block_map[pair] for pair in bar])


if __name__ == '__main__':
    main()


