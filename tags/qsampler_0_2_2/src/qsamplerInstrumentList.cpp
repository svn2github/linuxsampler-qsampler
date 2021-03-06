// qsamplerInstrumentList.cpp
//
/****************************************************************************
   Copyright (C) 2003-2007, rncbc aka Rui Nuno Capela. All rights reserved.
   Copyright (C) 2007, Christian Schoenebeck

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*****************************************************************************/

#include "qsamplerAbout.h"
#include "qsamplerInstrumentList.h"

#include "qsamplerInstrument.h"
#include "qsamplerInstrumentForm.h"

#include "qsamplerOptions.h"
#include "qsamplerMainForm.h"

#include <QApplication>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QCursor>
#include <QFileInfo>

// Needed for lroundf()
#include <math.h>

#ifndef CONFIG_ROUND
static inline long lroundf ( float x )
{
	if (x >= 0.0f)
		return long(x + 0.5f);
	else
		return long(x - 0.5f);
}
#endif

using namespace QSampler;


//-------------------------------------------------------------------------
// QSampler::MidiInstrumentsModel - data model for MIDI prog mappings
//                                  (used for QTableView)

MidiInstrumentsModel::MidiInstrumentsModel ( QObject* pParent)
	: QAbstractTableModel(pParent) 
{
	m_iMidiMap = LSCP_MIDI_MAP_ALL;
}


int MidiInstrumentsModel::rowCount ( const QModelIndex& /*parent*/) const
{
	if (m_iMidiMap == LSCP_MIDI_MAP_ALL) {
		int n = 0;
		for (InstrumentsMap::const_iterator itMap = m_instruments.begin();
				itMap != m_instruments.end(); ++itMap)
			n += (*itMap).size();
		return n;
	}
	InstrumentsMap::const_iterator itMap = m_instruments.find(m_iMidiMap);
	if (itMap == m_instruments.end()) return 0;
	return (*itMap).size();
}


int MidiInstrumentsModel::columnCount ( const QModelIndex& /*parent*/) const
{
	return 9;
}


QVariant MidiInstrumentsModel::data ( const QModelIndex &index, int role ) const
{
	if (!index.isValid())
		return QVariant();

	const Instrument* pInstr = NULL;

	if (m_iMidiMap == LSCP_MIDI_MAP_ALL) {
		int n = 0;
		for (InstrumentsMap::const_iterator itMap = m_instruments.begin();
				itMap != m_instruments.end(); ++itMap) {
			n += (*itMap).size();
			if (index.row() < n) {
				pInstr = &(*itMap)[index.row() + (*itMap).size() - n];
				break;
			}
		}
	} else {
		// resolve MIDI instrument map
		InstrumentsMap::const_iterator itMap = m_instruments.find(m_iMidiMap);
		if (itMap == m_instruments.end()) return QVariant();
		// resolve instrument in that map
		if (index.row() >= (*itMap).size()) return QVariant();
		pInstr = &(*itMap)[index.row()];
	}

	if (!pInstr)
		return QVariant();

	if (role == Qt::UserRole)
		return QVariant::fromValue((void *) pInstr);

	if (role == Qt::DisplayRole) {
		switch (index.column()) {
			case 0: return pInstr->name();
			case 1: return QVariant::fromValue(pInstr->map());
			case 2: return QVariant::fromValue(pInstr->bank());
			case 3: return QVariant::fromValue(pInstr->prog() + 1);
			case 4: return pInstr->engineName();
			case 5: return pInstr->instrumentFile();
			case 6: return QVariant::fromValue(pInstr->instrumentNr());
			case 7: return QString::number(pInstr->volume() * 100.0) + " %";
			case 8: {
				switch (pInstr->loadMode()) {
					case 3: return QObject::tr("Persistent");
					case 2: return QObject::tr("On Demand Hold");
					case 1: return QObject::tr("On Demand");
				}
			}
			default: return QVariant();
		}
	}

	return QVariant();
}


QVariant MidiInstrumentsModel::headerData (
	int section, Qt::Orientation orientation, int role ) const
{
	if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
		return QVariant();

	switch (section) {
		case 0: return tr("Name");
		case 1: return tr("Map");
		case 2: return tr("Bank");
		case 3: return tr("Prog");
		case 4: return tr("Engine");
		case 5: return tr("File");
		case 6: return tr("Nr");
		case 7: return tr("Vol");
		case 8: return tr("Mode");
		default: return QVariant();
	}
}


Instrument* MidiInstrumentsModel::addInstrument (
	int iMap, int iBank, int iProg )
{
	// Check it there's already one instrument item
	// with the very same key (bank, program);
	// if yes, just remove it without prejudice...
	for (int i = 0; i < m_instruments[iMap].size(); i++) {
		if (m_instruments[iMap][i].bank() == iBank &&
			m_instruments[iMap][i].prog() == iProg) {
			m_instruments[iMap].removeAt(i);
			break;
		}
	}

	// Resolve the appropriate place, we keep the list sorted that way ...
	int i = 0;
	for (; i < m_instruments[iMap].size(); ++i) {
		if (iBank < m_instruments[iMap][i].bank()
			|| (iBank == m_instruments[iMap][i].bank() &&
				iProg < m_instruments[iMap][i].prog())) {
			break;
		}
	}

	m_instruments[iMap].insert(i, Instrument(iMap, iBank, iProg));
	Instrument& instr = m_instruments[iMap][i];
	if (!instr.getInstrument())
		m_instruments[iMap].removeAt(i);

	return &instr;
}


void MidiInstrumentsModel::removeInstrument (
	const Instrument& instrument )
{
	const int iMap  = instrument.map();
	const int iBank = instrument.bank();
	const int iProg = instrument.prog();
	for (int i = 0; i < m_instruments[iMap].size(); i++) {
		if (m_instruments[iMap][i].bank() == iBank &&
			m_instruments[iMap][i].prog() == iProg) {
			m_instruments[iMap].removeAt(i);
			break;
		}
	}
}


// Reposition the instrument in the model (called when map/bank/prg changed)
void MidiInstrumentsModel::resort ( const Instrument& instrument )
{
	const int iMap  = instrument.map();
	const int iBank = instrument.bank();
	const int iProg = instrument.prog();
	// Remove given instrument from its current list
	removeInstrument(instrument);
	// Re-add the instrument
	addInstrument(iMap, iBank, iProg);
}


void MidiInstrumentsModel::setMidiMap ( int iMidiMap )
{
	if (iMidiMap < 0)
		iMidiMap = LSCP_MIDI_MAP_ALL;

	m_iMidiMap = iMidiMap;
}


int MidiInstrumentsModel::midiMap (void) const
{
	return m_iMidiMap;
}

void MidiInstrumentsModel::refresh (void)
{
	m_instruments.clear();

	MainForm* pMainForm = MainForm::getInstance();
	if (pMainForm == NULL)
		return;
	if (pMainForm->client() == NULL)
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	// Load the whole bunch of instrument items...
	lscp_midi_instrument_t* pInstrs
		= ::lscp_list_midi_instruments(pMainForm->client(), m_iMidiMap);
	for (int iInstr = 0; pInstrs && pInstrs[iInstr].map >= 0; ++iInstr) {
		const int iMap  = pInstrs[iInstr].map;
		const int iBank = pInstrs[iInstr].bank;
		const int iProg = pInstrs[iInstr].prog;
		addInstrument(iMap, iBank, iProg);
		// Try to keep it snappy :)
		QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	}

	QApplication::restoreOverrideCursor();

	if (pInstrs == NULL && ::lscp_client_get_errno(pMainForm->client())) {
		pMainForm->appendMessagesClient("lscp_list_midi_instruments");
		pMainForm->appendMessagesError(
			tr("Could not get current list of MIDI instrument mappings.\n\nSorry."));
	}

	// inform the outer world (QTableView) that our data changed
	QAbstractTableModel::reset();
}


//-------------------------------------------------------------------------
// QSampler::MidiInstrumentsDelegate - table cell renderer for MIDI prog
// mappings (doesn't actually do anything ATM, but is already there for a
// future cell editor widget implementation)

MidiInstrumentsDelegate::MidiInstrumentsDelegate ( QObject* pParent )
	: QItemDelegate(pParent)
{
}


QWidget* MidiInstrumentsDelegate::createEditor ( QWidget* pParent,
	const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	return QItemDelegate::createEditor(pParent, option, index);
//	return new QLabel(index.model()->data(index, Qt::DisplayRole).toString(), parent);
}


void MidiInstrumentsDelegate::setEditorData ( QWidget */*pEditor*/,
	const QModelIndex& /*index*/) const
{
}


void MidiInstrumentsDelegate::setModelData ( QWidget */*pEditor*/,
	QAbstractItemModel* /*model*/, const QModelIndex& /*index*/) const
{
}


void MidiInstrumentsDelegate::updateEditorGeometry ( QWidget *pEditor,
	const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QItemDelegate::updateEditorGeometry(pEditor, option, index);
//	if (pEditor) pEditor->setGeometry(option.rect);
}


// end of qsamplerInstrumentList.cpp
