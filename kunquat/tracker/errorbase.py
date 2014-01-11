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
import sys
import traceback
import os


_ERROR_BRIEF = 'Kunquat Tracker encountered an error.'
_SUBMIT_INFO = \
'''Please submit an issue to Kunquat issue tracker at
https://github.com/kunquat/kunquat/issues with the following
information attached.'''


def get_error_details(eclass, einst, trace):
    details_list = traceback.format_exception(eclass, einst, trace)
    return ''.join(details_list)


def print_error_msg(eclass, einst, trace):
    details = get_error_details(eclass, einst, trace)
    print('\n{}\n{}\n\n{}'.format(_ERROR_BRIEF, _SUBMIT_INFO, details),
            file=sys.stderr)


def log_error(eclass, einst, trace):
    pass # TODO: implement once we decide where to write


def setup_basic_error_handler():
    sys.excepthook = _basic_handler


def _basic_handler(eclass, einst, trace):
    print_error_msg(eclass, einst, trace)
    log_error(eclass, einst, trace)
    os.abort()


