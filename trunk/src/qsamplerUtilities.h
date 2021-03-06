// qsamplerUtilities.h
//
/****************************************************************************
   Copyright (C) 2004-2017, rncbc aka Rui Nuno Capela. All rights reserved.
   Copyright (C) 2007, 2008 Christian Schoenebeck

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

#ifndef __qsamplerUtilities_h
#define __qsamplerUtilities_h

#include <QString>


namespace qsamplerUtilities {

struct lscpVersion_t {
	int major;
	int minor;
};

QString lscpEscapePath(const QString& sPath);
QString lscpEscapedPathToPosix(const QString& sPath);
QString lscpEscapeText(const QString& sText);
QString lscpEscapedTextToRaw(const QString& sText);

lscpVersion_t getRemoteLscpVersion();

} // namespace qsamplerUtilities


#endif  // __qsamplerUtilities_h
