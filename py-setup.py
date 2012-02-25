# -*- coding: utf-8 -*-


from distutils.core import setup
import sys


inc_pkgs = ['kunquat', 'kunquat.extras']
inc_scripts = []
inc_data = []
req_list = []
if '--disable-player' not in sys.argv:
    inc_scripts.append('player/kunquat-player')
    inc_data.append(('share/man/man1', ['player/kunquat-player.1']),
                    #('/etc/bash_completion.d',
                    #    ['player/kunquat-player-completion']),
                   )
else:
    sys.argv.remove('--disable-player')
if '--disable-tracker' not in sys.argv:
    req_list.append('PyQt4')
    inc_pkgs[len(inc_pkgs):len(inc_pkgs)] = [
            'kunquat.editor',
            'kunquat.editor.effects',
            'kunquat.editor.env',
            'kunquat.editor.instruments',
            'kunquat.editor.sheet',
            ]
    inc_scripts.append('tracker/kunquat-tracker')
else:
    sys.argv.remove('--disable-tracker')

setup(name='kunquat',
      version='0.5.1',
      author='Tomi Jylhä-Ollila',
      author_email='tomi.jylha-ollila@iki.fi',
      url='http://kunquat.org/',
      description='A music sequencer.',
      license='CC0',
      requires=req_list,
      packages=inc_pkgs,
      scripts=inc_scripts,
      data_files=inc_data,
     )


