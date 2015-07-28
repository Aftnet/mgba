/* Copyright (c) 2013-2015 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "ShortcutView.h"

#include "GamepadButtonEvent.h"
#include "InputController.h"
#include "ShortcutController.h"

#include <QKeyEvent>

using namespace QGBA;

ShortcutView::ShortcutView(QWidget* parent)
	: QWidget(parent)
	, m_controller(nullptr)
	, m_input(nullptr)
{
	m_ui.setupUi(this);
	m_ui.keyEdit->setValueButton(-1);
	m_ui.keySequenceEdit->installEventFilter(this);

	connect(m_ui.keySequenceEdit, SIGNAL(keySequenceChanged(const QKeySequence&)), this, SLOT(updateKey(const QKeySequence&)));
	connect(m_ui.keyEdit, SIGNAL(valueChanged(int)), this, SLOT(updateButton(int)));
	connect(m_ui.keyEdit, SIGNAL(axisChanged(int, int)), this, SLOT(updateAxis(int, int)));
	connect(m_ui.shortcutTable, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(load(const QModelIndex&)));
	connect(m_ui.clearButton, SIGNAL(clicked()), this, SLOT(clear()));
}

void ShortcutView::setController(ShortcutController* controller) {
	m_controller = controller;
	m_ui.shortcutTable->setModel(controller);
}

void ShortcutView::setInputController(InputController* controller) {
	if (m_input) {
		m_input->releaseFocus(this);
	}
	m_input = controller;
	m_input->stealFocus(this);
}

bool ShortcutView::eventFilter(QObject*, QEvent* event) {
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() != Qt::Key_Tab && keyEvent->key() != Qt::Key_Backtab) {
			return false;
		}
		if (!(keyEvent->modifiers() & ~Qt::ShiftModifier)) {
			m_ui.keySequenceEdit->setKeySequence(ShortcutController::keyEventToSequence(keyEvent));
			keyEvent->accept();
			return true;
		}
	}
	return false;
}

void ShortcutView::load(const QModelIndex& index) {
	if (!m_controller) {
		return;
	}
	if (m_controller->isMenuAt(index)) {
		return;
	}
	QKeySequence sequence = m_controller->shortcutAt(index);
	if (index.column() == 1) {
		m_ui.keyboardButton->click();
	} else if (index.column() == 2) {
		m_ui.gamepadButton->click();
	}
	if (m_ui.gamepadButton->isChecked()) {
		bool blockSignals = m_ui.keyEdit->blockSignals(true);
		m_ui.keyEdit->setFocus();
		m_ui.keyEdit->setValueButton(-1); // There are no default bindings
		m_ui.keyEdit->blockSignals(blockSignals);
	} else {
		bool blockSignals = m_ui.keySequenceEdit->blockSignals(true);
		m_ui.keySequenceEdit->setFocus();
		m_ui.keySequenceEdit->setKeySequence(sequence);
		m_ui.keySequenceEdit->blockSignals(blockSignals);
	}
}

void ShortcutView::clear() {
	if (!m_controller) {
		return;
	}
	QModelIndex index = m_ui.shortcutTable->selectionModel()->currentIndex();
	if (m_controller->isMenuAt(index)) {
		return;
	}
	if (m_ui.gamepadButton->isChecked()) {
		m_controller->clearButton(index);
		m_ui.keyEdit->setValueButton(-1);
	} else {
		m_controller->clearKey(index);
		m_ui.keySequenceEdit->setKeySequence(QKeySequence());
	}
}

void ShortcutView::updateKey(const QKeySequence& shortcut) {
	if (!m_controller || m_controller->isMenuAt(m_ui.shortcutTable->selectionModel()->currentIndex())) {
		return;
	}
	m_controller->updateKey(m_ui.shortcutTable->selectionModel()->currentIndex(), shortcut);
}

void ShortcutView::updateButton(int button) {
	if (!m_controller || m_controller->isMenuAt(m_ui.shortcutTable->selectionModel()->currentIndex())) {
		return;
	}
	m_controller->updateButton(m_ui.shortcutTable->selectionModel()->currentIndex(), button);
}

void ShortcutView::updateAxis(int axis, int direction) {
	if (!m_controller || m_controller->isMenuAt(m_ui.shortcutTable->selectionModel()->currentIndex())) {
		return;
	}
	m_controller->updateAxis(m_ui.shortcutTable->selectionModel()->currentIndex(), axis,
	                         static_cast<GamepadAxisEvent::Direction>(direction));
}

void ShortcutView::closeEvent(QCloseEvent*) {
	if (m_input) {
		m_input->releaseFocus(this);
	}
}

bool ShortcutView::event(QEvent* event) {
	if (m_input) {
		if (event->type() == QEvent::WindowActivate) {
			m_input->stealFocus(this);
		} else if (event->type() == QEvent::WindowDeactivate) {
			m_input->releaseFocus(this);
		}
	}
	return QWidget::event(event);
}
