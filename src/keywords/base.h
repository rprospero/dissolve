/*
	*** Keyword Base Class
	*** src/module/keywordbase.h
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

#ifndef DISSOLVE_KEYWORDBASE_H
#define DISSOLVE_KEYWORDBASE_H

#include "base/charstring.h"
#include "templates/listitem.h"
#include "templates/vector3.h"

// Forward Declarations
class CoreData;
class Module;
class LineParser;
class ProcessPool;

// Keyword Base Class
class KeywordBase : public ListItem<KeywordBase>
{
	public:
	// Keyword Data Type
	enum KeywordDataType { AtomTypeSelectionData, BoolData, BroadeningFunctionData, CharStringData, ComplexData, Data1DStoreData, Data2DStoreData, Data3DStoreData, DoubleData, EnumOptionsData, EnumStringData, FileAndFormatData, IntegerData, IsotopologueListData, ModuleGroupsData, ModuleReferenceListData, PairBroadeningFunctionData, ProcedureData, SpeciesReferenceListData, SpeciesSiteData, SpeciesSiteReferenceListData, Vec3DoubleData, Vec3IntegerData, WindowFunctionData };
	// Constructor
	KeywordBase(KeywordDataType type);
	// Destructor
	virtual ~KeywordBase();
	// Return DataType name
	static const char* keywordDataType(KeywordDataType kdt);


	/*
	 * Keyword Description
	 */
	private:
	// Data type stored by keyword
	KeywordDataType type_;
	// Keyword string
	CharString keyword_;
	// Arguments string (for information)
	CharString arguments_;
	// Description of keyword, if any
	CharString description_;
	// Flags to apply if reinstated as a GenericListItem (i.e. in a Module)
	int genericItemFlags_;

	protected:
	// Whether the current data value has ever been set
	bool set_;

	public:
	// Set name, description, and item flags
	void set(const char* keyword, const char* description, const char* arguments, int genericItemFlags);
	// Return data type stored by keyword
	KeywordDataType type();
	// Return keyword name
	const char* keyword();
	// Return arguments string
	const char* arguments();
	// Return keyword description
	const char* description();
	// Return flags to apply if reinstated as a GenericListItem (i.e. in a Module)
	int genericItemFlags();
	// Return whether the data has ever been set
	bool isSet();


	/*
	 * Arguments
	 */
	public:
	// Return minimum number of arguments accepted
	virtual int minArguments() = 0;
	// Return maxnimum number of arguments accepted
	virtual int maxArguments() = 0;
	// Parse arguments from supplied LineParser, starting at given argument offset, utilising specified ProcessPool if required
	virtual bool read(LineParser& parser, int startArg, const CoreData& coreData, ProcessPool& procPool) = 0;
	// Write keyword data to specified LineParser
	virtual bool write(LineParser& parser, const char* prefix) = 0;


	/*
	 * Conversion
	 */
	public:
	// Return value (as bool)
	virtual bool asBool();
	// Return value (as int)
	virtual int asInt();
	// Return value (as double)
	virtual double asDouble();
	// Return value (as string)
	virtual const char* asString();
	// Return value as Vec3<int>
	virtual Vec3<int> asVec3Int();
	// Return value as Vec3<double>
	virtual Vec3<double> asVec3Double();
};

#endif
