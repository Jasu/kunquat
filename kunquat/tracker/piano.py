
import kunquat.tracker.kqt_limits as lim
from itertools import cycle

class Piano():
    def __init__(self, p):
        self.p = p 
        self._pressed = {}
        self._channel = cycle(xrange(lim.COLUMNS_MAX))
        
    def play(self, cents):
        ch = self._channel.next()
        self._pressed[cents] = ch
        self.p._playback.play_event(ch, ['.i', self.p._instruments._inst_num])
        self.p._playback.play_event(ch, ['n+', cents])

    def press(self, note, octave, cursor = None):
        cents = self.p._scale.get_cents(note, octave)
        if cents in self._pressed:
            return
        if cursor == None:
            play_note = True
        else:
            play_note = cursor.note_on_cents(cents)
        if play_note:
            self.play(cents)

    def release(self, note, octave):
        cents = self.p._scale.get_cents(note, octave)
        if cents not in self._pressed:
            return
        ch = self._pressed.pop(cents)
        self.p._playback.play_event(ch, ['n-', None])

