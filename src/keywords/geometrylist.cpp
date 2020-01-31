/*
	*** Keyword - Geometry List
	*** src/keywords/geometrylist.cpp
	Copyright T. Youngs 2012-2019

	This file is part of Dissolve.

	Dissolve is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Dissolve is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Dissolve.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "keywords/geometrylist.h"
#include "classes/coredata.h"
#include "base/lineparser.h"

// Constructor
GeometryListKeyword:: GeometryListKeyword::GeometryListKeyword(List<Geometry>& data, Geometry::GeometryType type) : KeywordData<List<Geometry>& >(KeywordBase::GeometryListData, data),
type_(type)
{
}
 
// Destructor
GeometryListKeyword::~GeometryListKeyword()
{
}

// Return minimum number of arguments accepted
int GeometryListKeyword::minArguments() const
{	
	if (type_ == Geometry::DistanceType) return 3;
	else if (type_ == Geometry::AngleType) return 4;
	else return 5;
}

// Return maximum number of arguments accepted
int GeometryListKeyword::maxArguments() const
{
	if (type_ == Geometry::DistanceType) return 3;
	else if (type_ == Geometry::AngleType) return 4;
	else return 5;
}

// Parse arguments from supplied LineParser, starting at given argument offset
bool GeometryListKeyword::read(LineParser& parser, int startArg, const CoreData& coreData)
{
	Geometry* g = data_.add();
	for(int i=startArg; i<=(startArg+maxArguments()-1); i++)
	{
		if (parser.argi(i) < 1) return Messenger::error("Index value, %i, not appropriate", parser.argi(i));
	}

	if (maxArguments() == 3)
		g->set(parser.argd(2+startArg), parser.argi(startArg) - 1, parser.argi(1+startArg) - 1);
	else if (maxArguments() == 4)
		g->set(parser.argd(3+startArg), parser.argi(startArg) - 1, parser.argi(1+startArg) - 1, parser.argi(2+startArg) - 1);
	else
		g->set(parser.argd(4+startArg), parser.argi(startArg) - 1, parser.argi(1+startArg) - 1, parser.argi(2+startArg) - 1, parser.argi(3+startArg) - 1);

	hasBeenSet();

	return true;
}

// Write keyword data to specified LineParser
bool GeometryListKeyword::write(LineParser& parser, const char* keywordName, const char* prefix)
{
	CharString index;

	for (Geometry* ref : data_)
	{
		index.clear();
		for (int n=0; n<maxArguments()-1; ++n) index.strcatf("  %i", ref->indices(n) + 1);
		if (!parser.writeLineF("%s%s%s  %12.4e\n", prefix, keywordName, index.get(), ref->value())) return false;
	}

	return true;
}
