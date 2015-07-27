# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import string

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from kunquat.kunquat.limits import *
import kunquat.tracker.ui.model.tstamp as tstamp
from editorlist import EditorList
from headerline import HeaderLine
from varvalidators import *


class EnvironmentEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._vars = VariableList()

        v = QVBoxLayout()
        v.setMargin(4)
        v.setSpacing(4)
        v.addWidget(HeaderLine('Initial environment state'))
        v.addWidget(self._vars)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._vars.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._vars.unregister_updaters()


class VariableList(EditorList):

    def __init__(self):
        EditorList.__init__(self)
        self._ui_model = None
        self._updater = None

        self._var_names = None
        self._var_names_set = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_var_names()

    def unregister_updaters(self):
        self.disconnect_widgets()
        self._updater.unregister_updater(self._perform_updates)

    def _update_var_names(self):
        module = self._ui_model.get_module()
        env = module.get_environment()
        self._var_names = env.get_var_names()
        self._var_names_set = set(self._var_names)

        self.update_list()

    def _perform_updates(self, signals):
        if 'signal_environment' in signals:
            self._update_var_names()

    def _make_adder_widget(self):
        adder = VariableAdder()
        adder.set_ui_model(self._ui_model)
        return adder

    def _get_updated_editor_count(self):
        var_count = len(self._var_names)
        return var_count

    def _make_editor_widget(self, index):
        editor = VariableEditor()
        editor.set_ui_model(self._ui_model)
        return editor

    def _update_editor(self, index, editor):
        var_name = self._var_names[index]

        editor.set_var_name(var_name)
        editor.set_used_names(self._var_names_set)

    def _disconnect_widget(self, widget):
        widget.unregister_updaters()


class VariableEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._var_name = None

        self._name_editor = VarNameEditor()
        self._type_editor = VarTypeEditor()
        self._value_editor = VarValueEditor()
        self._remove_button = VarRemoveButton()

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addWidget(self._name_editor)
        h.addWidget(self._type_editor)
        h.addWidget(self._value_editor)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

        self._name_editor.set_ui_model(ui_model)
        self._type_editor.set_ui_model(ui_model)
        self._value_editor.set_ui_model(ui_model)
        self._remove_button.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._remove_button.unregister_updaters()
        self._value_editor.unregister_updaters()
        self._type_editor.unregister_updaters()
        self._name_editor.unregister_updaters()

    def set_var_name(self, name):
        self._var_name = name

        self._name_editor.set_var_name(name)
        self._type_editor.set_var_name(name)
        self._value_editor.set_var_name(name)
        self._remove_button.set_var_name(name)

    def set_used_names(self, used_names):
        self._name_editor.set_used_names(used_names)


class VarNameEditor(QLineEdit):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None
        self._validator = None

        self.set_used_names(set())

        self._var_name = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        QObject.connect(self, SIGNAL('editingFinished()'), self._change_name)

    def unregister_updaters(self):
        pass

    def set_var_name(self, name):
        self._var_name = name

        old_block = self.blockSignals(True)
        self.setText(self._var_name)
        self.blockSignals(old_block)

    def set_used_names(self, used_names):
        self._validator = VarNameValidator(used_names)
        self.setValidator(self._validator)

    def _change_name(self):
        new_name = unicode(self.text())
        if new_name == self._var_name:
            return

        module = self._ui_model.get_module()
        env = module.get_environment()
        env.change_var_name(self._var_name, new_name)
        self._updater.signal_update(set(['signal_environment']))

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Escape:
            event.accept()
            self.set_var_name(self._var_name)
        else:
            return QLineEdit.keyPressEvent(self, event)


class VarTypeEditor(QComboBox):

    def __init__(self):
        QComboBox.__init__(self)
        self._ui_model = None
        self._updater = None

        self._var_name = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        module = self._ui_model.get_module()
        env = module.get_environment()
        var_types = env.get_var_types()

        var_type_names = {
                bool: 'Boolean',
                int: 'Integer',
                float: 'Floating point',
                tstamp.Tstamp: 'Timestamp',
            }

        for t in var_types:
            type_name = var_type_names[t]
            self.addItem(type_name)

        QObject.connect(self, SIGNAL('currentIndexChanged(int)'), self._change_type)

    def unregister_updaters(self):
        pass

    def set_var_name(self, name):
        self._var_name = name

        module = self._ui_model.get_module()
        env = module.get_environment()
        var_types = env.get_var_types()
        var_type = env.get_var_type(self._var_name)
        var_type_index = var_types.index(var_type)

        old_block = self.blockSignals(True)
        self.setCurrentIndex(var_type_index)
        self.blockSignals(old_block)

    def _change_type(self, index):
        module = self._ui_model.get_module()
        env = module.get_environment()
        var_types = env.get_var_types()
        env.change_var_type(self._var_name, var_types[index])
        self._updater.signal_update(set(['signal_environment']))


class VarValueEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._var_name = None

        self._editors = {
            bool:           QCheckBox(),
            int:            QLineEdit(),
            float:          QLineEdit(),
            tstamp.Tstamp:  QLineEdit(),
        }

        self._editors[bool].setText(' ') # work around broken clickable region

        self._editors[int].setValidator(IntValidator())
        self._editors[float].setValidator(FloatValidator())
        self._editors[tstamp.Tstamp].setValidator(FloatValidator())

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        module = self._ui_model.get_module()
        env = module.get_environment()
        var_types = env.get_var_types()

        s = QStackedLayout()
        for t in var_types:
            s.addWidget(self._editors[t])
        self.setLayout(s)

        QObject.connect(
                self._editors[bool],
                SIGNAL('stateChanged(int)'),
                self._change_bool_value)
        QObject.connect(
                self._editors[int],
                SIGNAL('editingFinished()'),
                self._change_int_value)
        QObject.connect(
                self._editors[float],
                SIGNAL('editingFinished()'),
                self._change_float_value)
        QObject.connect(
                self._editors[tstamp.Tstamp],
                SIGNAL('editingFinished()'),
                self._change_tstamp_value)

    def unregister_updaters(self):
        pass

    def set_var_name(self, name):
        self._var_name = name

        module = self._ui_model.get_module()
        env = module.get_environment()

        var_types = env.get_var_types()
        var_type = env.get_var_type(self._var_name)
        var_type_index = var_types.index(var_type)
        self.layout().setCurrentIndex(var_type_index)

        var_value = env.get_var_init_value(self._var_name)

        editor = self._editors[var_type]
        old_block = editor.blockSignals(True)
        if var_type == bool:
            editor.setCheckState(Qt.Checked if var_value else Qt.Unchecked)
        elif var_type == int:
            editor.setText(unicode(var_value))
        elif var_type == float:
            editor.setText(unicode(var_value))
        elif var_type == tstamp.Tstamp:
            editor.setText(unicode(float(var_value)))
        else:
            assert False
        editor.blockSignals(old_block)

    def _change_value(self, new_value):
        module = self._ui_model.get_module()
        env = module.get_environment()
        env.change_var_init_value(self._var_name, new_value)
        self._updater.signal_update(set(['signal_environment']))

    def _change_bool_value(self, new_state):
        new_value = (new_state == Qt.Checked)
        self._change_value(new_value)

    def _change_int_value(self):
        new_qstring = self._editors[int].text()
        new_value = int(unicode(new_qstring))
        self._change_value(new_value)

    def _change_float_value(self):
        new_qstring = self._editors[float].text()
        new_value = float(unicode(new_qstring))
        self._change_value(new_value)

    def _change_tstamp_value(self):
        new_qstring = self._editors[tstamp.Tstamp].text()
        new_value = tstamp.Tstamp(float(unicode(new_qstring)))
        self._change_value(new_value)


class VarRemoveButton(QPushButton):

    def __init__(self):
        QPushButton.__init__(self)
        self._ui_model = None
        self._updater = None

        self._var_name = None

        self.setStyleSheet('padding: 0 -2px;')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        icon_bank = ui_model.get_icon_bank()
        self.setIcon(QIcon(icon_bank.get_icon_path('delete_small')))

        QObject.connect(self, SIGNAL('clicked()'), self._remove)

    def unregister_updaters(self):
        pass

    def set_var_name(self, name):
        self._var_name = name

    def _remove(self):
        module = self._ui_model.get_module()
        env = module.get_environment()
        env.remove_var(self._var_name)
        self._updater.signal_update(set(['signal_environment']))


class VariableAdder(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._var_name = NewVarNameEditor()

        self._var_add_button = QPushButton()
        self._var_add_button.setText('Add new variable')
        self._var_add_button.setEnabled(False)

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addWidget(self._var_name)
        h.addWidget(self._var_add_button)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_used_names()

        QObject.connect(
                self._var_name, SIGNAL('textChanged(QString)'), self._text_changed)
        QObject.connect(
                self._var_name, SIGNAL('returnPressed()'), self._add_new_var)
        QObject.connect(
                self._var_add_button, SIGNAL('clicked()'), self._add_new_var)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_environment' in signals:
            self._update_used_names()

    def _get_used_names(self):
        module = self._ui_model.get_module()
        env = module.get_environment()
        used_names = set(env.get_var_names())
        return used_names

    def _update_used_names(self):
        used_names = self._get_used_names()
        self._var_name.set_used_names(used_names)
        self._var_add_button.setEnabled(bool(self._var_name.text()))

    def _text_changed(self, text):
        text = unicode(text)
        used_names = self._get_used_names()
        self._var_add_button.setEnabled(bool(text) and (text not in used_names))

    def _add_new_var(self):
        text = unicode(self._var_name.text())
        assert text and (text not in self._get_used_names())

        module = self._ui_model.get_module()
        env = module.get_environment()
        env.add_var(text, float, 0.0)

        self._var_name.setText('')

        self._updater.signal_update(set(['signal_environment']))


class VarNameValidator(QValidator):

    def __init__(self, used_names):
        QValidator.__init__(self)
        self._used_names = used_names

    def validate(self, contents, pos):
        in_str = unicode(contents)
        if not in_str:
            return (QValidator.Intermediate, pos)

        allowed_init_chars = '_' + string.ascii_lowercase
        allowed_chars = allowed_init_chars + string.digits

        if in_str[0] not in allowed_init_chars:
            return (QValidator.Invalid, pos)

        if all(ch in allowed_chars for ch in in_str):
            if in_str not in self._used_names:
                return (QValidator.Acceptable, pos)
            else:
                return (QValidator.Intermediate, pos)

        return (QValidator.Invalid, pos)


class NewVarNameEditor(QLineEdit):

    def __init__(self):
        QLineEdit.__init__(self)
        self.setMaxLength(ENV_VAR_NAME_MAX - 1)
        self._validator = VarNameValidator(set())
        self.setValidator(self._validator)

    def set_used_names(self, used_names):
        self._validator = VarNameValidator(used_names)
        self.setValidator(self._validator)


