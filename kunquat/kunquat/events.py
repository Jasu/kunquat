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

import kunquat


"""Kunquat event type descriptions.

"""


EVENT_ARG_BOOL = 'bool'
EVENT_ARG_INT = 'int'
EVENT_ARG_FLOAT = 'float'
EVENT_ARG_TSTAMP = 'tstamp'
EVENT_ARG_STRING = 'string'
EVENT_ARG_PAT = 'pat'
EVENT_ARG_PITCH = 'pitch'
EVENT_ARG_REALTIME = 'realtime'


"""Event information indexed by event name."""
all_events_by_name = kunquat.get_event_info()


"""Trigger event information indexed by event name."""
trigger_events_by_name = dict(
        e for e in all_events_by_name.iteritems() if not e[0].startswith('A'))


