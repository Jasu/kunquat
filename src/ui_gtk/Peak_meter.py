# coding=utf-8


# Copyright 2008 Tomi Jylhä-Ollila
#
# This file is part of Kunquat.
#
# Kunquat is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Kunquat is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.


import pygtk
pygtk.require('2.0')
import gtk, gobject, cairo, pango
from gtk import gdk
import math

import liblo


class Peak_meter(gtk.Widget):

	def player_state(self, path, args, types):
		if args[1] == 'stop':
			self.active = False
			for i in range(self.channels):
				self.levels[i] = None
				self.clipped[i] = False
			self.queue_draw()
			return
		if self.active:
			return
		self.active = True
		gobject.timeout_add(40, self.request_update)

	def play_stats(self, path, args, types):
		self.channels = args[0]
		for i in range(self.channels):
			peak_val = abs(args[(i * 2) + 1])
			neg_peak = abs(args[(i * 2) + 2])
			if peak_val < neg_peak:
				peak_val = neg_peak
			if peak_val > 1.0:
				peak_val = 1.0
				self.clipped[i] = True
			elif peak_val <= 0.0:
				self.levels[i] = None
				continue
			self.levels[i] = math.log(peak_val, 2) * 6
		self.queue_draw()

	def request_update(self):
		if not self.active:
			return False
		liblo.send(self.engine, '/kunquat/play_stats')
		return True

	def do_realize(self):
		self.set_flags(self.flags() | gtk.REALIZED)
		self.window = gdk.Window(
				self.get_parent_window(),
				width = self.allocation.width,
				height = self.allocation.height,
				window_type = gdk.WINDOW_CHILD,
				wclass = gdk.INPUT_OUTPUT,
				event_mask = self.get_events()
						| gdk.EXPOSURE_MASK)
		self.window.set_user_data(self)
		self.style.attach(self.window)
		self.style.set_background(self.window, gtk.STATE_NORMAL)
		self.window.move_resize(*self.allocation)

	def do_unrealize(self):
		self.window.destroy()
		self.window.set_user_data(None)

	def do_size_request(self, requisition):
		requisition.height = 10

	def do_size_allocate(self, allocation):
		self.allocation = allocation
		if self.flags() & gtk.REALIZED:
			self.window.move_resize(*allocation)

	def do_expose_event(self, event):
		cr = self.window.cairo_create()
		cr.rectangle(event.area.x, event.area.y,
				event.area.width, event.area.height)
		cr.clip()
		self.draw(cr, *self.window.get_size())

	def draw(self, cr, width, height):
		cr.set_source_rgb(*self.ptheme['Background colour'])
		cr.rectangle(0, 0, width, height)
		cr.fill()

		ch_border = 1.0
		ch_width = (height - ((self.channels - 1) * ch_border)) / self.channels

		clip_ind_width = 24
		clip_ind_x = width - clip_ind_width

		meter_border = 0.0
		meter_length = width - clip_ind_width - meter_border

		low_r, low_g, low_b = self.ptheme['Low colour']
		high_r, high_g, high_b = self.ptheme['High colour']
		max_r = (0.4 * self.ptheme['High colour'][0] +
				0.6 * self.ptheme['Clip colour'][0])
		max_g = (0.4 * self.ptheme['High colour'][1] +
				0.6 * self.ptheme['Clip colour'][1])
		max_b = (0.4 * self.ptheme['High colour'][2] +
				0.6 * self.ptheme['Clip colour'][2])

		high_border = (self.range - 6.0) / self.range

		meter_off_grad = cairo.LinearGradient(0, 0, meter_length, 0)
		meter_off_grad.add_color_stop_rgba(0.0,
				low_r, low_g, low_b, 0.3)
		meter_off_grad.add_color_stop_rgba(high_border,
				high_r, high_g, high_b, 0.3)
		meter_off_grad.add_color_stop_rgba(1.0,
				max_r, max_g, max_b, 0.3)

		meter_on_grad = cairo.LinearGradient(0, 0, meter_length, 0)
		meter_on_grad.add_color_stop_rgb(0.0, low_r, low_g, low_b)
		meter_on_grad.add_color_stop_rgb(high_border, high_r, high_g, high_b)
		meter_on_grad.add_color_stop_rgb(1.0, max_r, max_g, max_b)

		for i in range(self.channels):
			cr.rectangle(0, i * (ch_width + ch_border),
					meter_length, ch_width)
			cr.set_source(meter_off_grad)
			cr.fill()
			if self.levels[i] != None and self.levels[i] > -self.range:
				level = meter_length * (self.levels[i] + self.range) / self.range
				cr.rectangle(0, i * (ch_width + ch_border),
						level, ch_width)
				cr.set_source(meter_on_grad)
				cr.fill()
			if self.clipped[i]:
				cr.set_source_rgb(*self.ptheme['Clip colour'])
			else:
				w_alpha = self.ptheme['Clip colour'] + (0.3,)
				cr.set_source_rgba(*w_alpha)
			cr.rectangle(clip_ind_x, i * (ch_width + ch_border),
					clip_ind_width, ch_width)
			cr.fill()

	def do_redraw(self):
		if self.window:
			alloc = self.get_allocation()
			rect = gdk.Rectangle(alloc.x, alloc.y,
					alloc.width, alloc.height)
			self.window.invalidate_rect(rect, True)
			self.window.process_updates(True)
		return True

	def __init__(self, engine, server):
		gtk.Widget.__init__(self)
		self.engine = engine
		self.server = server

		self.ptheme = {
			'Background colour': (0, 0, 0),
			'Clip colour': (1, 0.2, 0.2),
			'High colour': (1, 0.9, 0.4),
			'Low colour': (0.2, 0.8, 0.3),
		}

		self.active = False

		self.range = 60

		self.channels = 2

		self.levels = [None for _ in range(2)]
		self.clipped = [False for _ in range(2)]


gobject.type_register(Peak_meter)


