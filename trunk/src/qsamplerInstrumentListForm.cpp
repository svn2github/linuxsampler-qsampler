#include "qsamplerInstrumentListForm.h"

#include "qsamplerMainForm.h"
#include "qsamplerOptions.h"
#include "qsamplerInstrument.h"

#include <QToolTip>

namespace QSampler {

InstrumentListForm::InstrumentListForm(QWidget* parent, Qt::WindowFlags flags) : QMainWindow(parent, flags) {
    ui.setupUi(this);

    ui.newInstrumentAction->setText(tr("New &Instrument..."));
    ui.newInstrumentAction->setShortcut(Qt::Key_Insert);
    ui.editInstrumentAction->setText(tr("&Edit..."));
    ui.editInstrumentAction->setShortcut(Qt::Key_Enter);
    ui.deleteInstrumentAction->setText(tr("&Delete"));
    ui.deleteInstrumentAction->setShortcut(Qt::Key_Delete);
    ui.refreshInstrumentsAction->setText(tr("&Refresh"));
    ui.refreshInstrumentsAction->setShortcut(Qt::Key_F5);

    // Setup toolbar widgets.
    InstrumentToolbar = addToolBar(tr("MIDI Instruments"));
    m_pMapComboBox = new QComboBox(InstrumentToolbar);
    m_pMapComboBox->setMinimumWidth(120);
    m_pMapComboBox->setEnabled(false);
    QToolTip::add(m_pMapComboBox, tr("Instrument Map"));

    InstrumentToolbar->addSeparator();
    ui.newInstrumentAction->addTo(InstrumentToolbar);
    ui.editInstrumentAction->addTo(InstrumentToolbar);
    ui.deleteInstrumentAction->addTo(InstrumentToolbar);
    InstrumentToolbar->addSeparator();
    ui.refreshInstrumentsAction->addTo(InstrumentToolbar);

    ui.InstrumentTable->setModel(&model);
    //ui.InstrumentTable->setDelegate(delegate);

	QObject::connect(m_pMapComboBox,
		SIGNAL(activated(int)),
		SLOT(activateMap(int)));

    connect(
        ui.refreshInstrumentsAction,
        SIGNAL(triggered()), SLOT(refreshInstruments(void))
    );
}

InstrumentListForm::~InstrumentListForm() {
	delete m_pMapComboBox;
}


// Notify our parent that we're emerging.
void InstrumentListForm::showEvent ( QShowEvent *pShowEvent )
{
	//MainForm* pMainForm = MainForm::getInstance();
	//if (pMainForm)
	//	pMainForm->stabilizeForm();

	QWidget::showEvent(pShowEvent);
}


// Notify our parent that we're closing.
void InstrumentListForm::hideEvent ( QHideEvent *pHideEvent )
{
	QWidget::hideEvent(pHideEvent);

	//MainForm* pMainForm = MainForm::getInstance();
	//if (pMainForm)
	//	pMainForm->stabilizeForm();
}


// Refresh all instrument list and views.
void InstrumentListForm::refreshInstruments (void)
{
	MainForm* pMainForm = MainForm::getInstance();
	if (pMainForm == NULL)
		return;

	qsamplerOptions *pOptions = pMainForm->options();
	if (pOptions == NULL)
		return;

	// Get/save current map selection...
	int iMap = m_pMapComboBox->currentItem();
	if (iMap < 0 || m_pMapComboBox->count() < 2)
		iMap = pOptions->iMidiMap + 1;

	// Populate maps list.
	m_pMapComboBox->clear();
	m_pMapComboBox->insertItem(tr("(All)"));
	m_pMapComboBox->insertStringList(qsamplerInstrument::getMapNames());

	// Adjust to saved selection...
	if (iMap < 0 || iMap >= m_pMapComboBox->count())
		iMap = 0;
	m_pMapComboBox->setCurrentItem(iMap);
	m_pMapComboBox->setEnabled(m_pMapComboBox->count() > 1);

	activateMap(iMap);
}


// Refresh instrument maps selector.
void InstrumentListForm::activateMap ( int iMap )
{
	MainForm* pMainForm = MainForm::getInstance();
	if (pMainForm == NULL)
		return;

	qsamplerOptions *pOptions = pMainForm->options();
	if (pOptions == NULL)
		return;

	int iMidiMap = iMap - 1;
	if (iMidiMap >= 0)
		pOptions->iMidiMap = iMidiMap;

	model.setMidiMap(iMidiMap);
	model.refresh();
}

} // namespace QSampler