# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2012
#         Toni Ruottu, Finland 2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import json
import view
from events import *

class Store(object):
    '''
    >>> from tempfile import mkdtemp
    >>> from shutil import rmtree
    >>> import os
    >>> path = mkdtemp()
    >>> store = Store(path)
    >>> store.flush()
    >>> store['/lol/omg'] = 'jee'
    >>> store['/lol/omg']
    'jee'
    >>> v = store.get_view('/lol')
    >>> v.get('omg')
    'jee'
    >>> v.put('asdf/qwerty', 56)
    >>> store['/lol/asdf/qwerty']
    56
    >>> store.commit()
    >>> sid = store._path
    >>> type(sid) == type('')
    True
    >>> os.path.isdir(sid)
    True
    >>> rmtree(path)
    '''

    callbacks = []

    def __init__(self, path, callbacks=[]):
        self._path = path
        self._memory = {}
        for callback in callbacks:
            self.register_callback(callback)
        self.signal(Init(store=self))

    def __getitem__(self, key):
        return self.get(key)

    def __setitem__(self, key, value):
        self.put(key, value)

    def _encode(self, value):
        if isinstance(value, str):
            return value
        return json.dumps(value)

    def delete(self, key):
        if key in self._memory:
            del self._memory[key]
            self.signal(Value_update(key=key))

    def put(self, key, value):
        self._memory[key] = self._encode(value)
        self.signal(Value_update(key=key))

    def _get(self, key):
        return self._memory[key]

    def get(self, key, parse='raw', key_prefix=''):
        view = self.get_view(key_prefix)
        return view.get(key, parse)

    def get_json(self, key, key_prefix=''):
        view = self.get_view(key_prefix)
        return view.get(key, parse='json')

    def get_view(self, prefix):
        return view.View(self, prefix)

    def commit(self):
        pass

    def flush(self):
        pass

    def to_tar(self, path, key_prefix=''):
        view = self.get_view(key_prefix)
        view.to_tar(path)

    def del_tree(self, key_prefix=''):
        pass

    def from_path(self, path, key_prefix=''):
        view = self.get_view(key_prefix)
        view.from_path(path)

    def register_callback(self, callback):
        self.callbacks.append(callback)

    def signal(self, event):
        for callback in self.callbacks:
            callback(event)

if __name__ == "__main__":
    import doctest
    doctest.testmod()
