#include "qsamplerInstrumentForm.h"

#include "qsamplerAbout.h"
#include "qsamplerMainForm.h"

// Needed for lroundf()
#include <math.h>

namespace QSampler {

#ifndef CONFIG_ROUND
static inline long lroundf ( float x )
{
	if (x >= 0.0f)
		return long(x + 0.5f);
	else
		return long(x - 0.5f);
}
#endif

InstrumentForm::InstrumentForm(QWidget* parent) : QDialog(parent) {
    ui.setupUi(this);

	// Initialize locals.
	m_pInstrument = NULL;

	m_iDirtySetup = 0;
	m_iDirtyCount = 0;
	m_iDirtyName  = 0;

	// Try to restore normal window positioning.
	adjustSize();
}

InstrumentForm::~InstrumentForm() {
}

// Channel dialog setup formal initializer.
void InstrumentForm::setup ( qsamplerInstrument *pInstrument )
{
	m_pInstrument = pInstrument;

	m_iDirtySetup = 0;
	m_iDirtyCount = 0;
	m_iDirtyName  = 0;

	if (m_pInstrument == NULL)
		return;

	// Check if we're up and connected.
	MainForm* pMainForm = MainForm::getInstance();
	if (pMainForm == NULL)
		return;
	if (pMainForm->client() == NULL)
		return;

	qsamplerOptions *pOptions = pMainForm->options();
	if (pOptions == NULL)
		return;

	// It can be a brand new channel, remember?
	bool bNew = (m_pInstrument->bank() < 0 || m_pInstrument->prog() < 0);
	if (!bNew) {
		m_pInstrument->getInstrument();
		m_iDirtyName++;
	}

	// Avoid nested changes.
	m_iDirtySetup++;

	// Load combo box history...
	pOptions->loadComboBoxHistory(ui.InstrumentFileComboBox);

	// Populate maps list.
	ui.MapComboBox->clear();
	ui.MapComboBox->insertStringList(qsamplerInstrument::getMapNames());

	// Populate Engines list.
	const char **ppszEngines = ::lscp_list_available_engines(pMainForm->client());
	if (ppszEngines) {
		ui.EngineNameComboBox->clear();
		for (int iEngine = 0; ppszEngines[iEngine]; iEngine++)
			ui.EngineNameComboBox->insertItem(ppszEngines[iEngine]);
	}
	else pMainForm->appendMessagesClient("lscp_list_available_engines");

	// Read proper instrument information,
	// and populate the instrument form fields.

	// Instrument map name...
	int iMap = (bNew ? pOptions->iMidiMap : m_pInstrument->map());
	if (iMap < 0)
		iMap = 0;
	const QString& sMapName = qsamplerInstrument::getMapName(iMap);
	if (!sMapName.isEmpty())
		ui.MapComboBox->setCurrentText(sMapName);
	// It might be no maps around...
	bool bMapEnabled = (ui.MapComboBox->count() > 0);
	ui.MapTextLabel->setEnabled(bMapEnabled);
	ui.MapComboBox->setEnabled(bMapEnabled);

	// Instrument bank/program...
	int iBank = (bNew ? pOptions->iMidiBank : m_pInstrument->bank());
	int iProg = (bNew ? pOptions->iMidiProg : m_pInstrument->prog()) + 1;
	if (bNew && iProg > 128) {
		iProg = 1;
		iBank++;
	}
	ui.BankSpinBox->setValue(iBank);
	ui.ProgSpinBox->setValue(iProg);

	// Instrument name...
	ui.NameLineEdit->setText(m_pInstrument->name());

	// Engine name...
	QString sEngineName = m_pInstrument->engineName();
	if (sEngineName.isEmpty() || bNew)
		sEngineName = pOptions->sEngineName;
	if (sEngineName.isEmpty())
		sEngineName = qsamplerChannel::noEngineName();
	if (ui.EngineNameComboBox->findText(sEngineName,
			Qt::MatchExactly | Qt::MatchCaseSensitive) == 0) {
		ui.EngineNameComboBox->insertItem(sEngineName);
	}
	ui.EngineNameComboBox->setCurrentText(sEngineName);
	// Instrument filename and index...
	QString sInstrumentFile = m_pInstrument->instrumentFile();
	if (sInstrumentFile.isEmpty())
		sInstrumentFile = qsamplerChannel::noInstrumentName();
	ui.InstrumentFileComboBox->setCurrentText(sInstrumentFile);
	ui.InstrumentNrComboBox->clear();
	ui.InstrumentNrComboBox->insertStringList(
		qsamplerChannel::getInstrumentList(sInstrumentFile,
		pOptions->bInstrumentNames));
	ui.InstrumentNrComboBox->setCurrentItem(m_pInstrument->instrumentNr());

	// Instrument volume....
	int iVolume = (bNew ? pOptions->iVolume :
		::lroundf(100.0f * m_pInstrument->volume()));
	ui.VolumeSpinBox->setValue(iVolume);

	// Instrument load mode...
	int iLoadMode = (bNew ? pOptions->iLoadMode :
		m_pInstrument->loadMode());
	ui.LoadModeComboBox->setCurrentItem(iLoadMode);

	// Done.
	m_iDirtySetup--;
	stabilizeForm();

	// Done.
	m_iDirtySetup--;
	stabilizeForm();
}


// Special case for name change,
void InstrumentForm::nameChanged ( const QString& /* sName */ )
{
	if (m_iDirtySetup > 0)
		return;

	m_iDirtyName++;
	changed();
}


// Browse and open an instrument file.
void InstrumentForm::openInstrumentFile (void)
{
	MainForm* pMainForm = MainForm::getInstance();
	if (pMainForm == NULL)
		return;

	qsamplerOptions *pOptions = pMainForm->options();
	if (pOptions == NULL)
		return;

	// FIXME: the instrument file filters should be restricted,
	// depending on the current engine.
	QString sInstrumentFile = QFileDialog::getOpenFileName(
		pOptions->sInstrumentDir,                   // Start here.
		tr("Instrument files") + " (*.gig *.dls)",  // Filter (GIG and DLS files)
		this, 0,                                    // Parent and name (none)
		QSAMPLER_TITLE ": " + tr("Instrument files")// Caption.
	);

	if (sInstrumentFile.isEmpty())
		return;

	ui.InstrumentFileComboBox->setCurrentText(sInstrumentFile);
	updateInstrumentName();
}


// Refresh the actual instrument name.
void InstrumentForm::updateInstrumentName (void)
{
	MainForm* pMainForm = MainForm::getInstance();
	if (pMainForm == NULL)
		return;

	qsamplerOptions *pOptions = pMainForm->options();
	if (pOptions == NULL)
		return;

	// TODO: this better idea would be to use libgig
	// to retrieve the REAL instrument names.
	ui.InstrumentNrComboBox->clear();
	ui.InstrumentNrComboBox->insertStringList(
		qsamplerChannel::getInstrumentList(
			ui.InstrumentFileComboBox->currentText(),
			pOptions->bInstrumentNames)
	);

	instrumentNrChanged();
}


// Special case for instrumnet index change,
void InstrumentForm::instrumentNrChanged (void)
{
	if (m_iDirtySetup > 0)
		return;

	if (ui.NameLineEdit->text().isEmpty() || m_iDirtyName == 0) {
		ui.NameLineEdit->setText(ui.InstrumentNrComboBox->currentText());
		m_iDirtyName = 0;
	}

	changed();
}


// Accept settings (OK button slot).
void InstrumentForm::accept (void)
{
	if (m_pInstrument == NULL)
		return;

	MainForm* pMainForm = MainForm::getInstance();
	if (pMainForm == NULL)
		return;
	if (pMainForm->client() == NULL)
		return;

	qsamplerOptions *pOptions = pMainForm->options();
	if (pOptions == NULL)
		return;

	if (m_iDirtyCount > 0) {
		m_pInstrument->setMap(ui.MapComboBox->currentItem());
		m_pInstrument->setBank(ui.BankSpinBox->value());
		m_pInstrument->setProg(ui.ProgSpinBox->value() - 1);
		m_pInstrument->setName(ui.NameLineEdit->text());
		m_pInstrument->setEngineName(ui.EngineNameComboBox->currentText());
		m_pInstrument->setInstrumentFile(ui.InstrumentFileComboBox->currentText());
		m_pInstrument->setInstrumentNr(ui.InstrumentNrComboBox->currentItem());
		m_pInstrument->setVolume(0.01f * float(ui.VolumeSpinBox->value()));
		m_pInstrument->setLoadMode(ui.LoadModeComboBox->currentItem());
	}

	// Save default engine name, instrument directory and history...
	pOptions->sInstrumentDir = QFileInfo(ui.InstrumentFileComboBox->currentText()).dirPath(true);
	pOptions->sEngineName = ui.EngineNameComboBox->currentText();
	pOptions->iMidiMap  = ui.MapComboBox->currentItem();
	pOptions->iMidiBank = ui.BankSpinBox->value();
	pOptions->iMidiProg = ui.ProgSpinBox->value();
	pOptions->iVolume   = ui.VolumeSpinBox->value();
	pOptions->iLoadMode = ui.LoadModeComboBox->currentItem();
	pOptions->saveComboBoxHistory(ui.InstrumentFileComboBox);

	// Just go with dialog acceptance.
	QDialog::accept();
}


// Reject settings (Cancel button slot).
void InstrumentForm::reject (void)
{
	bool bReject = true;

	// Check if there's any pending changes...
	if (m_iDirtyCount > 0 && ui.OkPushButton->isEnabled()) {
		switch (QMessageBox::warning(this,
			QSAMPLER_TITLE ": " + tr("Warning"),
			tr("Some channel settings have been changed.\n\n"
			"Do you want to apply the changes?"),
			tr("Apply"), tr("Discard"), tr("Cancel"))) {
		case 0:     // Apply...
			accept();
			return;
		case 1:     // Discard
			break;
		default:    // Cancel.
			bReject = false;
			break;
		}
	}

	if (bReject)
		QDialog::reject();
}


// Dirty up settings.
void InstrumentForm::changed (void)
{
	if (m_iDirtySetup > 0)
		return;

	m_iDirtyCount++;
	stabilizeForm();
}


// Stabilize current form state.
void InstrumentForm::stabilizeForm (void)
{
	bool bValid = !ui.NameLineEdit->text().isEmpty();

	const QString& sPath = ui.InstrumentFileComboBox->currentText();
	bValid = bValid && !sPath.isEmpty() && QFileInfo(sPath).exists();

	ui.OkPushButton->setEnabled(m_iDirtyCount > 0 && bValid);
}

} // namespace QSampler