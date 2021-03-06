// qsamplerOptionsForm.cpp
//
/****************************************************************************
   Copyright (C) 2004-2007, rncbc aka Rui Nuno Capela. All rights reserved.
   Copyright (C) 2007, Christian Schoenebeck

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "qsamplerOptionsForm.h"

#include "qsamplerAbout.h"
#include "qsamplerOptions.h"

#include <QMessageBox>
#include <QFontDialog>


namespace QSampler {

//-------------------------------------------------------------------------
// QSampler::OptionsForm -- Options form implementation.
//

OptionsForm::OptionsForm ( QWidget* pParent )
	: QDialog(pParent)
{
	m_ui.setupUi(this);

	// No settings descriptor initially (the caller will set it).
	m_pOptions = NULL;

	// Initialize dirty control state.
	m_iDirtySetup = 0;
	m_iDirtyCount = 0;

	// Set dialog validators...
	m_ui.ServerPortComboBox->setValidator(
		new QIntValidator(m_ui.ServerPortComboBox));

	// Try to restore old window positioning.
	adjustSize();

	QObject::connect(m_ui.ServerHostComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.ServerPortComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.ServerTimeoutSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.ServerStartCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.ServerCmdLineComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.StartDelaySpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.DisplayFontPushButton,
		SIGNAL(clicked()),
		SLOT(chooseDisplayFont()));
	QObject::connect(m_ui.DisplayEffectCheckBox,
		SIGNAL(toggled(bool)),
		SLOT(toggleDisplayEffect(bool)));
	QObject::connect(m_ui.AutoRefreshCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.AutoRefreshTimeSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.MaxVolumeSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.MessagesFontPushButton,
		SIGNAL(clicked()),
		SLOT(chooseMessagesFont()));
	QObject::connect(m_ui.MessagesLimitCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.MessagesLimitLinesSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.ConfirmRemoveCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.KeepOnTopCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.StdoutCaptureCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.MaxRecentFilesSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.CompletePathCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.InstrumentNamesCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.OkPushButton,
		SIGNAL(clicked()),
		SLOT(accept()));
	QObject::connect(m_ui.CancelPushButton,
		SIGNAL(clicked()),
		SLOT(reject()));
}

OptionsForm::~OptionsForm()
{
}

// Populate (setup) dialog controls from settings descriptors.
void OptionsForm::setup ( Options *pOptions )
{
	// Set reference descriptor.
	m_pOptions = pOptions;

	// Start clean.
	m_iDirtyCount = 0;
	// Avoid nested changes.
	m_iDirtySetup++;

	// Load combo box history...
	m_pOptions->loadComboBoxHistory(m_ui.ServerHostComboBox);
	m_pOptions->loadComboBoxHistory(m_ui.ServerPortComboBox);
	m_pOptions->loadComboBoxHistory(m_ui.ServerCmdLineComboBox);

	// Load Server settings...
	m_ui.ServerHostComboBox->setEditText(m_pOptions->sServerHost);
	m_ui.ServerPortComboBox->setEditText(QString::number(m_pOptions->iServerPort));
	m_ui.ServerTimeoutSpinBox->setValue(m_pOptions->iServerTimeout);
	m_ui.ServerStartCheckBox->setChecked(m_pOptions->bServerStart);
	m_ui.ServerCmdLineComboBox->setEditText(m_pOptions->sServerCmdLine);
	m_ui.StartDelaySpinBox->setValue(m_pOptions->iStartDelay);

	// Load Display options...
	QFont font;
	QPalette pal;

	// Display font.
	if (m_pOptions->sDisplayFont.isEmpty()
		|| !font.fromString(m_pOptions->sDisplayFont))
		font = QFont("Sans Serif", 8);
	m_ui.DisplayFontTextLabel->setFont(font);
	m_ui.DisplayFontTextLabel->setText(font.family()
		+ ' ' + QString::number(font.pointSize()));

	// Display effect.
	m_ui.DisplayEffectCheckBox->setChecked(m_pOptions->bDisplayEffect);
	toggleDisplayEffect(m_pOptions->bDisplayEffect);

	// Auto-refresh and maximum volume options.
	m_ui.AutoRefreshCheckBox->setChecked(m_pOptions->bAutoRefresh);
	m_ui.AutoRefreshTimeSpinBox->setValue(m_pOptions->iAutoRefreshTime);
	m_ui.MaxVolumeSpinBox->setValue(m_pOptions->iMaxVolume);

	// Messages font.
	if (m_pOptions->sMessagesFont.isEmpty()
		|| !font.fromString(m_pOptions->sMessagesFont))
		font = QFont("Fixed", 8);
	pal = m_ui.MessagesFontTextLabel->palette();
	pal.setColor(QPalette::Background, Qt::white);
	m_ui.MessagesFontTextLabel->setPalette(pal);
	m_ui.MessagesFontTextLabel->setFont(font);
	m_ui.MessagesFontTextLabel->setText(font.family()
		+ ' ' + QString::number(font.pointSize()));

	// Messages limit option.
	m_ui.MessagesLimitCheckBox->setChecked(m_pOptions->bMessagesLimit);
	m_ui.MessagesLimitLinesSpinBox->setValue(m_pOptions->iMessagesLimitLines);

	// Other options finally.
	m_ui.ConfirmRemoveCheckBox->setChecked(m_pOptions->bConfirmRemove);
	m_ui.KeepOnTopCheckBox->setChecked(m_pOptions->bKeepOnTop);
	m_ui.StdoutCaptureCheckBox->setChecked(m_pOptions->bStdoutCapture);
	m_ui.CompletePathCheckBox->setChecked(m_pOptions->bCompletePath);
	m_ui.InstrumentNamesCheckBox->setChecked(m_pOptions->bInstrumentNames);
	m_ui.MaxRecentFilesSpinBox->setValue(m_pOptions->iMaxRecentFiles);

#ifndef CONFIG_LIBGIG
	m_ui.InstrumentNamesCheckBox->setEnabled(false);
#endif

	// Done.
	m_iDirtySetup--;
	stabilizeForm();
}


// Accept settings (OK button slot).
void OptionsForm::accept (void)
{
	// Save options...
	if (m_iDirtyCount > 0) {
		// Server settings....
		m_pOptions->sServerHost    = m_ui.ServerHostComboBox->currentText().trimmed();
		m_pOptions->iServerPort    = m_ui.ServerPortComboBox->currentText().toInt();
		m_pOptions->iServerTimeout = m_ui.ServerTimeoutSpinBox->value();
		m_pOptions->bServerStart   = m_ui.ServerStartCheckBox->isChecked();
		m_pOptions->sServerCmdLine = m_ui.ServerCmdLineComboBox->currentText().trimmed();
		m_pOptions->iStartDelay      = m_ui.StartDelaySpinBox->value();
		// Channels options...
		m_pOptions->sDisplayFont   = m_ui.DisplayFontTextLabel->font().toString();
		m_pOptions->bDisplayEffect = m_ui.DisplayEffectCheckBox->isChecked();
		m_pOptions->bAutoRefresh   = m_ui.AutoRefreshCheckBox->isChecked();
		m_pOptions->iAutoRefreshTime = m_ui.AutoRefreshTimeSpinBox->value();
		m_pOptions->iMaxVolume     = m_ui.MaxVolumeSpinBox->value();
		// Messages options...
		m_pOptions->sMessagesFont  = m_ui.MessagesFontTextLabel->font().toString();
		m_pOptions->bMessagesLimit = m_ui.MessagesLimitCheckBox->isChecked();
		m_pOptions->iMessagesLimitLines = m_ui.MessagesLimitLinesSpinBox->value();
		// Other options...
		m_pOptions->bConfirmRemove = m_ui.ConfirmRemoveCheckBox->isChecked();
		m_pOptions->bKeepOnTop     = m_ui.KeepOnTopCheckBox->isChecked();
		m_pOptions->bStdoutCapture = m_ui.StdoutCaptureCheckBox->isChecked();
		m_pOptions->bCompletePath  = m_ui.CompletePathCheckBox->isChecked();
		m_pOptions->bInstrumentNames = m_ui.InstrumentNamesCheckBox->isChecked();
		m_pOptions->iMaxRecentFiles  = m_ui.MaxRecentFilesSpinBox->value();
		// Reset dirty flag.
		m_iDirtyCount = 0;
	}

	// Save combobox history...
	m_pOptions->saveComboBoxHistory(m_ui.ServerHostComboBox);
	m_pOptions->saveComboBoxHistory(m_ui.ServerPortComboBox);
	m_pOptions->saveComboBoxHistory(m_ui.ServerCmdLineComboBox);

	// Just go with dialog acceptance.
	QDialog::accept();
}


// Reject settings (Cancel button slot).
void OptionsForm::reject (void)
{
	bool bReject = true;

	// Check if there's any pending changes...
	if (m_iDirtyCount > 0) {
		switch (QMessageBox::warning(this,
			QSAMPLER_TITLE ": " + tr("Warning"),
			tr("Some settings have been changed.\n\n"
			"Do you want to apply the changes?"),
			tr("Apply"), tr("Discard"), tr("Cancel"))) {
		case 0:     // Apply...
			accept();
			return;
		case 1:     // Discard
			break;
		default:    // Cancel.
			bReject = false;
		}
	}

	if (bReject)
		QDialog::reject();
}


// Dirty up settings.
void OptionsForm::optionsChanged (void)
{
	if (m_iDirtySetup > 0)
		return;

	m_iDirtyCount++;
	stabilizeForm();
}


// Stabilize current form state.
void OptionsForm::stabilizeForm (void)
{
	bool bEnabled = m_ui.ServerStartCheckBox->isChecked();
	m_ui.ServerCmdLineTextLabel->setEnabled(bEnabled);
	m_ui.ServerCmdLineComboBox->setEnabled(bEnabled);
	m_ui.StartDelayTextLabel->setEnabled(bEnabled);
	m_ui.StartDelaySpinBox->setEnabled(bEnabled);

	m_ui.AutoRefreshTimeSpinBox->setEnabled(
		m_ui.AutoRefreshCheckBox->isChecked());
	m_ui.MessagesLimitLinesSpinBox->setEnabled(
		m_ui.MessagesLimitCheckBox->isChecked());

	m_ui.OkPushButton->setEnabled(m_iDirtyCount > 0);
}


// The channel display font selection dialog.
void OptionsForm::chooseDisplayFont (void)
{
	bool  bOk  = false;
	QFont font = QFontDialog::getFont(&bOk,
		m_ui.DisplayFontTextLabel->font(), this);
	if (bOk) {
		m_ui.DisplayFontTextLabel->setFont(font);
		m_ui.DisplayFontTextLabel->setText(font.family()
			+ ' ' + QString::number(font.pointSize()));
		optionsChanged();
	}
}


// The messages font selection dialog.
void OptionsForm::chooseMessagesFont (void)
{
	bool  bOk  = false;
	QFont font = QFontDialog::getFont(&bOk,
		m_ui.MessagesFontTextLabel->font(), this);
	if (bOk) {
		m_ui.MessagesFontTextLabel->setFont(font);
		m_ui.MessagesFontTextLabel->setText(font.family()
			+ ' ' + QString::number(font.pointSize()));
		optionsChanged();
	}
}


// The channel display effect demo changer.
void OptionsForm::toggleDisplayEffect ( bool bOn )
{
	QPalette pal;
	pal.setColor(QPalette::Foreground, Qt::green);
	if (bOn) {
		QPixmap pm(":/icons/displaybg1.png");
		pal.setBrush(QPalette::Background, QBrush(pm));
	} else {
		pal.setColor(QPalette::Background, Qt::black);
	}
	m_ui.DisplayFontTextLabel->setPalette(pal);

	optionsChanged();
}

} // namespace QSampler


// end of qsamplerOptionsForm.cpp
