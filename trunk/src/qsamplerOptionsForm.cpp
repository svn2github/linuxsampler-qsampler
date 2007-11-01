#include "qsamplerOptionsForm.h"

#include "qsamplerAbout.h"
#include "qsamplerOptions.h"

#include <QMessageBox>
#include <QFontDialog>

namespace QSampler {

OptionsForm::OptionsForm(QWidget* parent) : QDialog(parent) {
    ui.setupUi(this);

    // No settings descriptor initially (the caller will set it).
    m_pOptions = NULL;

    // Initialize dirty control state.
    m_iDirtySetup = 0;
    m_iDirtyCount = 0;

    // Set dialog validators...
    ui.ServerPortComboBox->setValidator(new QIntValidator(ui.ServerPortComboBox));

    // Try to restore old window positioning.
    adjustSize();
}

OptionsForm::~OptionsForm() {
}

// Populate (setup) dialog controls from settings descriptors.
void OptionsForm::setup ( qsamplerOptions *pOptions )
{
    // Set reference descriptor.
    m_pOptions = pOptions;

    // Start clean.
    m_iDirtyCount = 0;
    // Avoid nested changes.
    m_iDirtySetup++;

    // Load combo box history...
    m_pOptions->loadComboBoxHistory(ui.ServerHostComboBox);
    m_pOptions->loadComboBoxHistory(ui.ServerPortComboBox);
    m_pOptions->loadComboBoxHistory(ui.ServerCmdLineComboBox);

    // Load Server settings...
    ui.ServerHostComboBox->setCurrentText(m_pOptions->sServerHost);
    ui.ServerPortComboBox->setCurrentText(QString::number(m_pOptions->iServerPort));
    ui.ServerTimeoutSpinBox->setValue(m_pOptions->iServerTimeout);
    ui.ServerStartCheckBox->setChecked(m_pOptions->bServerStart);
    ui.ServerCmdLineComboBox->setCurrentText(m_pOptions->sServerCmdLine);
    ui.StartDelaySpinBox->setValue(m_pOptions->iStartDelay);

    // Load Display options...
    QFont font;

    // Display font.
    if (m_pOptions->sDisplayFont.isEmpty() || !font.fromString(m_pOptions->sDisplayFont))
        font = QFont("Sans Serif", 8);
    ui.DisplayFontTextLabel->setFont(font);
    ui.DisplayFontTextLabel->setText(font.family() + " " + QString::number(font.pointSize()));

    // Display effect.
    ui.DisplayEffectCheckBox->setChecked(m_pOptions->bDisplayEffect);
    toggleDisplayEffect(m_pOptions->bDisplayEffect);

    // Auto-refresh and maximum volume options.
    ui.AutoRefreshCheckBox->setChecked(m_pOptions->bAutoRefresh);
    ui.AutoRefreshTimeSpinBox->setValue(m_pOptions->iAutoRefreshTime);
    ui.MaxVolumeSpinBox->setValue(m_pOptions->iMaxVolume);

    // Messages font.
    if (m_pOptions->sMessagesFont.isEmpty() || !font.fromString(m_pOptions->sMessagesFont))
        font = QFont("Fixed", 8);
    ui.MessagesFontTextLabel->setFont(font);
    ui.MessagesFontTextLabel->setText(font.family() + " " + QString::number(font.pointSize()));

    // Messages limit option.
    ui.MessagesLimitCheckBox->setChecked(m_pOptions->bMessagesLimit);
    ui.MessagesLimitLinesSpinBox->setValue(m_pOptions->iMessagesLimitLines);

    // Other options finally.
    ui.ConfirmRemoveCheckBox->setChecked(m_pOptions->bConfirmRemove);
    ui.KeepOnTopCheckBox->setChecked(m_pOptions->bKeepOnTop);
    ui.StdoutCaptureCheckBox->setChecked(m_pOptions->bStdoutCapture);
    ui.CompletePathCheckBox->setChecked(m_pOptions->bCompletePath);
    ui.InstrumentNamesCheckBox->setChecked(m_pOptions->bInstrumentNames);
    ui.MaxRecentFilesSpinBox->setValue(m_pOptions->iMaxRecentFiles);

#ifndef CONFIG_LIBGIG
    ui.InstrumentNamesCheckBox->setEnabled(false);
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
        m_pOptions->sServerHost         = ui.ServerHostComboBox->currentText().stripWhiteSpace();
        m_pOptions->iServerPort         = ui.ServerPortComboBox->currentText().toInt();
        m_pOptions->iServerTimeout      = ui.ServerTimeoutSpinBox->value();
        m_pOptions->bServerStart        = ui.ServerStartCheckBox->isChecked();
        m_pOptions->sServerCmdLine      = ui.ServerCmdLineComboBox->currentText().stripWhiteSpace();
        m_pOptions->iStartDelay         = ui.StartDelaySpinBox->value();
        // Channels options...
        m_pOptions->sDisplayFont        = ui.DisplayFontTextLabel->font().toString();
        m_pOptions->bDisplayEffect      = ui.DisplayEffectCheckBox->isChecked();
        m_pOptions->bAutoRefresh        = ui.AutoRefreshCheckBox->isChecked();
        m_pOptions->iAutoRefreshTime    = ui.AutoRefreshTimeSpinBox->value();
        m_pOptions->iMaxVolume          = ui.MaxVolumeSpinBox->value();
        // Messages options...
        m_pOptions->sMessagesFont       = ui.MessagesFontTextLabel->font().toString();
        m_pOptions->bMessagesLimit      = ui.MessagesLimitCheckBox->isChecked();
        m_pOptions->iMessagesLimitLines = ui.MessagesLimitLinesSpinBox->value();
        // Other options...
        m_pOptions->bConfirmRemove      = ui.ConfirmRemoveCheckBox->isChecked();
        m_pOptions->bKeepOnTop          = ui.KeepOnTopCheckBox->isChecked();
        m_pOptions->bStdoutCapture      = ui.StdoutCaptureCheckBox->isChecked();
        m_pOptions->bCompletePath       = ui.CompletePathCheckBox->isChecked();
        m_pOptions->bInstrumentNames    = ui.InstrumentNamesCheckBox->isChecked();
        m_pOptions->iMaxRecentFiles     = ui.MaxRecentFilesSpinBox->value();
        // Reset dirty flag.
        m_iDirtyCount = 0;
    }

    // Save combobox history...
    m_pOptions->saveComboBoxHistory(ui.ServerHostComboBox);
    m_pOptions->saveComboBoxHistory(ui.ServerPortComboBox);
    m_pOptions->saveComboBoxHistory(ui.ServerCmdLineComboBox);

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
    bool bEnabled = ui.ServerStartCheckBox->isChecked();
    ui.ServerCmdLineTextLabel->setEnabled(bEnabled);
    ui.ServerCmdLineComboBox->setEnabled(bEnabled);
    ui.StartDelayTextLabel->setEnabled(bEnabled);
    ui.StartDelaySpinBox->setEnabled(bEnabled);

    ui.AutoRefreshTimeSpinBox->setEnabled(ui.AutoRefreshCheckBox->isChecked());
    ui.MessagesLimitLinesSpinBox->setEnabled(ui.MessagesLimitCheckBox->isChecked());

    ui.OkPushButton->setEnabled(m_iDirtyCount > 0);
}


// The channel display font selection dialog.
void OptionsForm::chooseDisplayFont (void)
{
    bool  bOk  = false;
    QFont font = QFontDialog::getFont(&bOk, ui.DisplayFontTextLabel->font(), this);
    if (bOk) {
        ui.DisplayFontTextLabel->setFont(font);
        ui.DisplayFontTextLabel->setText(font.family() + " " + QString::number(font.pointSize()));
        optionsChanged();
    }
}


// The messages font selection dialog.
void OptionsForm::chooseMessagesFont (void)
{
    bool  bOk  = false;
    QFont font = QFontDialog::getFont(&bOk, ui.MessagesFontTextLabel->font(), this);
    if (bOk) {
        ui.MessagesFontTextLabel->setFont(font);
        ui.MessagesFontTextLabel->setText(font.family() + " " + QString::number(font.pointSize()));
        optionsChanged();
    }
}


// The channel display effect demo changer.
void OptionsForm::toggleDisplayEffect ( bool bOn )
{
    QPixmap pm;
    if (bOn)
        pm = QPixmap(":/qsampler/pixmaps/displaybg1.png");
    ui.DisplayFontTextLabel->setPaletteBackgroundPixmap(pm);

    optionsChanged();
}

} // namespace QSampler