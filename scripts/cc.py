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
import os.path
import subprocess
import sys

import command


def get_cc(cmd=None):
    cc = None
    if cmd == 'gcc':
        cc = get_gcc()
    elif cmd != None:
        print('Unsupported compiler requested: {}'.format(cmd), file=sys.stderr)
        sys.exit(1)
    else:
        cc = get_gcc()

    if not cc:
        if cmd:
            print('Could not find a C compiler.', file=sys.stderr)
        else:
            print('Could not find {}'.format(cmd), file=sys.stderr)
        sys.exit(1)

    return cc


def get_gcc():
    try:
        output = subprocess.check_output(
                ['gcc', '--version'], stderr=subprocess.STDOUT)
    except OSError, subprocess.CalledProcessError:
        output = ''
    if output.startswith('gcc'):
        return GccCommand()
    else:
        return None


class GccCommand():

    def __init__(self):
        self._cmd = 'gcc'
        self._compile_flags = [
                '-std=c99',
                '-pedantic',
                '-Wall',
                '-Wextra',
                '-Werror',
                '-Wwrite-strings',
            ]
        self._include_flags = []
        self._link_dirs = []
        self._link_flags = []

    def get_name(self):
        return 'GCC'

    def get_compile_flags(self):
        return ' '.join(self._compile_flags)

    def set_debug(self, enabled):
        if enabled:
            self._compile_flags.append('-g')
        else:
            self._compile_flags.append('-DNDEBUG')

    def set_optimisation(self, level):
        self._compile_flags.append('-O{:d}'.format(level))

    def set_dynamic_export(self, enabled):
        if enabled:
            self._link_flags.append('-rdynamic')

    def set_pic(self, enabled):
        if enabled:
            self._compile_flags.append('-fPIC')

    def add_define(self, name, value=None):
        if value == None:
            self._compile_flags.append('-D{}'.format(name))
        else:
            self._compile_flags.append('-D{}={}'.format(name, value))

    def add_include_dir(self, inc_dir):
        self._include_flags.append('-I{}'.format(inc_dir))

    def add_lib_dir(self, lib_dir):
        self._link_dirs.append('-L{}'.format(lib_dir))

    def add_lib(self, lib_name):
        self._link_flags.append('-l{}'.format(lib_name))

    def compile(self, builder, source_path, obj_path, echo=None):
        command.make_dirs(builder, os.path.dirname(obj_path), echo='')
        args = ([self._cmd] +
                ['-c', source_path] +
                ['-o', obj_path] +
                self._compile_flags +
                self._include_flags)
        return command.run_command(builder, *args, echo=echo)

    def link_lib(self, builder, obj_paths, so_path, version_major, echo=None):
        command.make_dirs(builder, os.path.dirname(so_path), echo='')
        lib_name = os.path.basename(so_path)
        soname_flag = '-Wl,-soname,{}.{}'.format(lib_name, version_major)
        args = ([self._cmd] +
                ['-o', so_path] +
                obj_paths +
                self._link_dirs +
                self._link_flags +
                ['-shared', soname_flag])
        return command.run_command(builder, *args, echo=echo)

    def build_exe(self, builder, source_path, exe_path, echo=None):
        command.make_dirs(builder, os.path.dirname(exe_path), echo='')
        args = ([self._cmd] +
                ['-o', exe_path] +
                [source_path] +
                self._compile_flags +
                self._include_flags +
                self._link_dirs +
                self._link_flags)
        return command.run_command(builder, *args, echo=echo)


