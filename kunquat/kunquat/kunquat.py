# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2010-2013
#          Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

"""A library for accessing Kunquat music data.

This module provides interfaces for inspecting and modifying Kunquat
compositions and rendering them to digital audio.

Classes:
Handle -- An interface for Kunquat compositions.

Exceptions:
KunquatError         -- The base class for Kunquat errors.
KunquatArgumentError -- An error for most invalid argument errors.
KunquatFormatError   -- An error for indicating invalid music data.
KunquatMemoryError   -- An error for indicating memory allocation
                        failure.
KunquatResourceError -- An error for resource access errors.

"""

from __future__ import print_function
import ctypes
import json

__all__ = ['Kunquat',
           'KunquatError', 'KunquatArgumentError',
           'KunquatFormatError', 'KunquatMemoryError',
           'KunquatResourceError']


class BaseHandle(object):

    """Base class for all Kunquat Handles. Do not use directly."""

    @property
    def song(self):
        """Obsolete"""
        assert False

    @song.setter
    def song(self, value):
        """Obsolete"""
        assert False

    @property
    def track(self):
        """The current track ([0, 255], or None for all tracks)"""
        return self._track

    @track.setter
    def track(self, value):
        """Set the track number.

        Note that setting the track number also resets nanoseconds to
        zero.

        """
        track = value
        if track is None:
            track = -1
        _kunquat.kqt_Handle_set_position(self._handle, track, 0)
        self._track = value
        self._nanoseconds = 0

    @property
    def nanoseconds(self):
        """The current position in nanoseconds."""
        return self._nanoseconds

    @nanoseconds.setter
    def nanoseconds(self, value):
        """Set the track position in nanoseconds.

        Note that setting a new nanoseconds value may take a while for
        very large values.

        """
        track = self.track
        if track is None:
            track = -1
        _kunquat.kqt_Handle_set_position(self._handle, track, value)
        self._nanoseconds = value

    @property
    def audio_rate(self):
        """Audio rate in frames per second."""
        return self._audio_rate

    @audio_rate.setter
    def audio_rate(self, value):
        """Set the audio rate.

        Typical values include 48000 (the default) and 44100 ("CD
        quality").

        """
        _kunquat.kqt_Handle_set_audio_rate(self._handle, value)
        self._audio_rate = value

    @property
    def audio_buffer_size(self):
        """Audio buffer size in frames."""
        return self._audio_buffer_size

    @audio_buffer_size.setter
    def audio_buffer_size(self, value):
        """Set the audio buffer size.

        This value sets the maximum (and default) length for audio
        buffers.

        """
        _kunquat.kqt_Handle_set_audio_buffer_size(self._handle, value)
        self._audio_buffer_size = value

    def get_duration(self, track=None):
        """Count the duration of the composition in nanoseconds.

        Arguments:
        track -- The track of which length is to be calculated.  If
                 this parameter is omitted, the function returns the
                 total length of all tracks.

        Return value:
        The duration in nanoseconds.

        Exceptions:
        KunquatArgumentError -- The track number is not valid.

        """
        if track is None:
            track = -1
        return _kunquat.kqt_Handle_get_duration(self._handle, track)

    def play(self, frame_count=None):
        """Play audio according to the state of the handle.

        Optional arguments:
        frame_count -- The number of frames to be mixed.  The default
                       value is self.audio_buffer_size.

        Exceptions:
        KunquatArgumentError -- frame_count is not positive.

        """
        if not frame_count:
            frame_count = self._buffer_size
        _kunquat.kqt_Handle_play(self._handle, frame_count)
        self._nanoseconds = _kunquat.kqt_Handle_get_position(self._handle)

    def has_stopped(self):
        """Return True if playback has stopped."""
        return _kunquat.kqt_Handle_has_stopped(self._handle)

    def get_audio(self):
        """Get audio data.

        Returns:
        A pair containing audio data for, respectively, the left and
        the right output channel.  Buffers shorter than frame_count
        frames indicate that the end has been reached.

        """
        frames_available = _kunquat.kqt_Handle_get_frames_available(self._handle)
        cbuf_left = _kunquat.kqt_Handle_get_audio(self._handle, 0)
        cbuf_right = _kunquat.kqt_Handle_get_audio(self._handle, 1)
        return cbuf_left[:frames_available], cbuf_right[:frames_available]

    def fire_event(self, channel, event):
        """Fire an event.

        Arguments:
        channel -- The channel where the event takes place.  The
                   channel number is >= 0 and < 64.
        event -- The event description, which is a pair (either a tuple
                 or a list) with the event name as the first element
                 and its argument expression as the second element.
                 Example: ['cn+', '300'] (Note On at 300 cents above
                 A4, i.e. C5 in 12-tone Equal Temperament).

        """
        _kunquat.kqt_Handle_fire_event(self._handle, channel, json.dumps(event))

    def receive_events(self):
        """Receive outgoing events.

        Return value:
        A list containing all requested outgoing events fired after
        the last call of receive.

        """
        all_events = []
        el = json.loads(str(_kunquat.kqt_Handle_get_events(self._handle)))
        while el:
            all_events += el
            el = json.loads(str(_kunquat.kqt_Handle_get_events(self._handle)))
        return all_events


class Kunquat(BaseHandle):

    """Kunquat instance for playing and modifying compositions in memory.

    Public methods:
    set_data     -- Set composition data.
    get_duration -- Calculate the length of a track.
    play         -- Play audio.
    get_audio    -- Get audio data.
    fire         -- Fire an event.

    Public instance variables:
    buffer_size -- Mixing buffer size.
    mixing_rate -- Mixing rate.
    nanoseconds -- The current position in nanoseconds.
    track       -- The current track (None or [0,255]).

    To produce two frames of silence, do the following:
    >>> k = Kunquat()
    >>> k.set_data('album/p_manifest.json', {})
    >>> k.set_data('album/p_tracks.json', [0])
    >>> k.set_data('song_00/p_manifest.json', {})
    >>> k.set_data('song_00/p_order_list.json', [ [0, 0] ])
    >>> k.set_data('pat_000/p_manifest.json', {})
    >>> k.set_data('pat_000/p_pattern.json', { 'length': [16, 0] })
    >>> k.set_data('pat_000/instance_000/p_manifest.json', {})
    >>> k.validate()
    >>> k.play(2)
    >>> audio_data = k.get_audio()
    >>> audio_data
    ([0.0, 0.0], [0.0, 0.0])

    """

    def __init__(self, audio_rate=48000):
        """Create a new Kunquat instance.

        Optional arguments:
        audio_rate -- Audio rate in frames per second.  Typical
                      values include 48000 (the default) and 44100
                      ("CD quality").

        Exceptions:
        KunquatArgumentError -- audio_rate is not positive.

        """
        if '_handle' not in self.__dict__:
            self._handle = _kunquat.kqt_new_Handle()
            if not self._handle:
                raise _get_error(json.loads(
                                 _kunquat.kqt_Handle_get_error(None)))
        self._track = None
        self._nanoseconds = 0
        self._audio_buffer_size = _kunquat.kqt_Handle_get_audio_buffer_size(
                self._handle)
        if audio_rate <= 0:
            raise KunquatArgumentError('Mixing rate must be positive')
        self.audio_rate = audio_rate

    def set_data(self, key, value):
        """Set data in the Kunquat instance.

        Arguments:
        key --   The key of the data in the composition.  A key
                 consists of one or more textual elements separated by
                 forward slashes ('/').  The last element is the only
                 one that is allowed and required to contain a period.
                 Examples:
                 'p_composition.json'
                 'pat_000/col_00/p_triggers.json'
                 'ins_01/p_instrument.json'
        value -- The data to be set.  For JSON keys, this should be a
                 Python object -- it is automatically converted to a
                 JSON string.

        Exceptions:
        KunquatArgumentError -- The key is not valid.

        """
        if key[key.index('.'):].startswith('.json'):
            value = json.dumps(value) if value != None else ''
        elif value == None:
            value = ''
        data = buffer(value)
        cdata = (ctypes.c_ubyte * len(data))()
        cdata[:] = [ord(b) for b in data][:]
        _kunquat.kqt_Handle_set_data(self._handle,
                                     key,
                                     ctypes.cast(cdata,
                                         ctypes.POINTER(ctypes.c_ubyte)),
                                     len(data))

    def validate(self):
        """Validate data in the Kunquat instance.

        Exceptions:
        KunquatFormatError -- The module data is not valid.  This
                              indicates that the handle is useless and
                              should be discarded.

        """
        _kunquat.kqt_Handle_validate(self._handle)

    def __del__(self):
        if self._handle:
            _kunquat.kqt_del_Handle(self._handle)
        self._handle = None


def _get_error(obj):
    if obj['type'] == 'ArgumentError':
        return KunquatArgumentError(obj)
    elif obj['type'] == 'FormatError':
        return KunquatFormatError(obj)
    elif obj['type'] == 'MemoryError':
        return KunquatMemoryError(obj)
    elif obj['type'] == 'ResourceError':
        return KunquatResourceError(obj)
    return KunquatError(obj)


def _error_check(result, func, arguments):
    chandle = arguments[0]
    error_str = _kunquat.kqt_Handle_get_error(chandle)
    if not error_str:
        return result
    _kunquat.kqt_Handle_clear_error(chandle)
    raise _get_error(json.loads(error_str))


class KunquatError(Exception):

    """Base class for errors in Kunquat."""

    def __init__(self, obj):
        if isinstance(obj, str) or isinstance(obj, unicode):
            obj = { 'message': obj }
        self.obj = obj

    def __getitem__(self, key):
        return self.obj[key]

    def __str__(self):
        return self.obj['message']


class KunquatArgumentError(KunquatError):
    """Error indicating that a given method argument is invalid."""


class KunquatFormatError(KunquatError):
    """Error indicating that composition data is invalid."""


class KunquatMemoryError(KunquatError, MemoryError):
    """Error indicating that memory allocation failed."""


class KunquatResourceError(KunquatError):
    """Error indicating that an external service request has failed."""


def fake_out_of_memory():
    _kunquat.kqt_fake_out_of_memory(0)


_kunquat = ctypes.CDLL('libkunquat.so')

kqt_Handle = ctypes.c_int

_kunquat.kqt_new_Handle.argtypes = []
_kunquat.kqt_new_Handle.restype = kqt_Handle
_kunquat.kqt_del_Handle.argtypes = [kqt_Handle]
_kunquat.kqt_del_Handle.restype = None

_kunquat.kqt_Handle_get_error.argtypes = [kqt_Handle]
_kunquat.kqt_Handle_get_error.restype = ctypes.c_char_p
_kunquat.kqt_Handle_clear_error.argtypes = [kqt_Handle]
_kunquat.kqt_Handle_clear_error.restype = None

_kunquat.kqt_Handle_validate.argtypes = [kqt_Handle]
_kunquat.kqt_Handle_validate.restype = ctypes.c_int
_kunquat.kqt_Handle_validate.errcheck = _error_check

_kunquat.kqt_Handle_set_data.argtypes = [kqt_Handle,
                                         ctypes.c_char_p,
                                         ctypes.POINTER(ctypes.c_ubyte),
                                         ctypes.c_long]
_kunquat.kqt_Handle_set_data.restype = ctypes.c_int
_kunquat.kqt_Handle_set_data.errcheck = _error_check

_kunquat.kqt_Handle_play.argtypes = [kqt_Handle, ctypes.c_long]
_kunquat.kqt_Handle_play.restype = ctypes.c_int
_kunquat.kqt_Handle_play.errcheck = _error_check
_kunquat.kqt_Handle_has_stopped.argtypes = [kqt_Handle]
_kunquat.kqt_Handle_has_stopped.restype = ctypes.c_int
_kunquat.kqt_Handle_has_stopped.errcheck = _error_check

_kunquat.kqt_Handle_get_frames_available.argtypes = [kqt_Handle]
_kunquat.kqt_Handle_get_frames_available.restype = ctypes.c_long
_kunquat.kqt_Handle_get_frames_available.errcheck = _error_check
_kunquat.kqt_Handle_get_audio.argtypes = [kqt_Handle, ctypes.c_int]
_kunquat.kqt_Handle_get_audio.restype = ctypes.POINTER(ctypes.c_float)
_kunquat.kqt_Handle_get_audio.errcheck = _error_check

_kunquat.kqt_Handle_set_audio_rate.argtypes = [kqt_Handle, ctypes.c_long]
_kunquat.kqt_Handle_set_audio_rate.restype = ctypes.c_int
_kunquat.kqt_Handle_set_audio_rate.errcheck = _error_check
_kunquat.kqt_Handle_get_audio_rate.argtypes = [kqt_Handle]
_kunquat.kqt_Handle_get_audio_rate.restype = ctypes.c_long
_kunquat.kqt_Handle_get_audio_rate.errcheck = _error_check

_kunquat.kqt_Handle_set_audio_buffer_size.argtypes = [kqt_Handle, ctypes.c_long]
_kunquat.kqt_Handle_set_audio_buffer_size.restype = ctypes.c_int
_kunquat.kqt_Handle_set_audio_buffer_size.errcheck = _error_check
_kunquat.kqt_Handle_get_audio_buffer_size.argtypes = [kqt_Handle]
_kunquat.kqt_Handle_get_audio_buffer_size.restype = ctypes.c_long
_kunquat.kqt_Handle_get_audio_buffer_size.errcheck = _error_check

_kunquat.kqt_Handle_get_duration.argtypes = [kqt_Handle, ctypes.c_int]
_kunquat.kqt_Handle_get_duration.restype = ctypes.c_longlong
_kunquat.kqt_Handle_get_duration.errcheck = _error_check
_kunquat.kqt_Handle_set_position.argtypes = [kqt_Handle,
                                             ctypes.c_int,
                                             ctypes.c_longlong]
_kunquat.kqt_Handle_set_position.restype = ctypes.c_int
_kunquat.kqt_Handle_set_position.errcheck = _error_check
_kunquat.kqt_Handle_get_position.argtypes = [kqt_Handle]
_kunquat.kqt_Handle_get_position.restype = ctypes.c_longlong
_kunquat.kqt_Handle_get_position.errcheck = _error_check

_kunquat.kqt_Handle_fire_event.argtypes = [kqt_Handle,
                                     ctypes.c_int,
                                     ctypes.c_char_p]
_kunquat.kqt_Handle_fire_event.restype = ctypes.c_int
_kunquat.kqt_Handle_fire_event.errcheck = _error_check

_kunquat.kqt_Handle_get_events.argtypes = [kqt_Handle]
_kunquat.kqt_Handle_get_events.restype = ctypes.c_char_p
_kunquat.kqt_Handle_get_events.errcheck = _error_check

_kunquat.kqt_fake_out_of_memory.argtypes = [ctypes.c_long]
_kunquat.kqt_fake_out_of_memory.restype = None


