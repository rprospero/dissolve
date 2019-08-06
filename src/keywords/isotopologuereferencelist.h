/*
	*** Keyword - Isotopologue Reference List
	*** src/keywords/isotopologuereferencelist.h
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

#ifndef DISSOLVE_KEYWORD_ISOTOPOLOGUEREFERENCELIST_H
#define DISSOLVE_KEYWORD_ISOTOPOLOGUEREFERENCELIST_H

#include "keywords/data.h"
#include "keywords/base.h"
#include "classes/isotopologuereference.h"
#include "templates/list.h"
#include "templates/reflist.h"

// Forward Declarations
/* none */

// Keyword with IsotopologueReference Data
class IsotopologueReferenceListKeyword : public KeywordData< List<IsotopologueReference>& >
{
	public:
	// Constructor
	IsotopologueReferenceListKeyword(List<IsotopologueReference>& references, const RefList<Configuration>& associatedConfigurations);
	// Destructor
	~IsotopologueReferenceListKeyword();


	/*
	 * Associated Configurations
	 */
	private:
	// Associated Configurations, to which the IsotopologueList refers 
	const RefList<Configuration>& associatedConfigurations_;

	public:
	// Return associated Configurations, to which the IsotopologueList refers 
	const RefList<Configuration>& associatedConfigurations() const;


	/*
	 * Arguments
	 */
	public:
	// Return minimum number of arguments accepted
	int minArguments();
	// Return maximum number of arguments accepted
	int maxArguments();
	// Parse arguments from supplied LineParser, starting at given argument offset, utilising specified ProcessPool if required
	bool read(LineParser& parser, int startArg, const CoreData& coreData, ProcessPool& procPool);
	// Write keyword data to specified LineParser
	bool write(LineParser& parser, const char* prefix);


	/*
	 * Object Management
	 */
	protected:
	// Prune any references to the supplied Species in the contained data
	void removeReferencesTo(Species* sp);
	// Prune any references to the supplied Isotopologue in the contained data
	void removeReferencesTo(Isotopologue* iso);
};

#endif

