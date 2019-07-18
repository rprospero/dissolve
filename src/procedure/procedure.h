/*
	*** Procedure
	*** src/procedure/procedure.h
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

#ifndef DISSOLVE_PROCEDURE_H
#define DISSOLVE_PROCEDURE_H

#include "procedure/nodes/sequence.h"
#include "procedure/nodecontextstack.h"

// Forward Declarations
class Configuration;
class LineParser;
class ProcessPool;

// Procedure
class Procedure
{
	public:
	// Constructor
	Procedure();
	// Destructor
	~Procedure();


	/*
	 * Data
	 */
	private:
	// Context stack
	NodeContextStack contextStack_;
	// Sequence node from which the analysis starts
	SequenceProcedureNode rootSequence_;

	public:
	// Clear all data
	void clear();
	// Set End keyword for root sequence
	void setEndKeyword(const char* keyword);
	// Add (own) specified node to root sequence
	void addRootSequenceNode(ProcedureNode* node);
	// Return the context stack
	const NodeContextStack& contextStack() const;


	/*
	 * Execute
	 */
	private:
	// List of Configurations and the coordinate indices at which they were last analysed
	RefList<Configuration,int> configurationPoints_;

	public:
	// Run analysis for specified Configuration, storing / retrieving generated data from supplied list 
	bool execute(ProcessPool& procPool, Configuration* cfg, const char* prefix, GenericList& targetList);


	/*
	 * Read / Write
	 */
	public:
	// Read structure from specified LineParser
	bool read(LineParser& parser, const CoreData& coreData);
	// Write structure to specified LineParser
	bool write(LineParser& parser, const char* prefix);
};

#endif