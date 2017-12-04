/*
	*** Mime Strings
	*** src/gui/mimestrings.cpp
	Copyright T. Youngs 2012-2017

	This file is part of dUQ.

	dUQ is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	dUQ is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with dUQ.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/mimestrings.h"

/*
 * Mime String
 */

// Constructor
MimeString::MimeString(MimeString::MimeStringType type, QString data)
{
	type_ = type;
	data_ = data;
}

// Return type of data contained in string
MimeString::MimeStringType MimeString::type() const
{
	return type_;
}

// Return string data
QString MimeString::data() const
{
	return data_;
}

/*
 * Mime Strings
 */

// Constructor
MimeStrings::MimeStrings()
{
}

bool MimeStrings::hasFormat(const QString& mimeType) const
{
	if (mimeType == "duq/mimestrings") return true;

	return false;
}

QStringList MimeStrings::formats() const
{
	return QStringList() << "duq/mimestrings";
}

QVariant MimeStrings::retrieveData(const QString& mimeType, QVariant::Type type) const
{
	return QVariant();
}

// Add mime string
void MimeStrings::add(MimeString::MimeStringType type, QString data)
{
	MimeString* mimeString = new MimeString(type, data);
	mimeStrings_.own(mimeString);
}

// Add mime strings from source MimeStrings
void MimeStrings::add(MimeStrings& sourceStrings)
{
	for (MimeString* mimeString = sourceStrings.mimeStrings().first(); mimeString != NULL; mimeString = mimeString->next) add(mimeString->type(), mimeString->data());
}

// Return mime strings
List<MimeString>& MimeStrings::mimeStrings()
{
	return mimeStrings_;
}
