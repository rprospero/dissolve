/*
	*** Procedure Node - Exclude
	*** src/procedure/nodes/exclude.cpp
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

#include "procedure/nodes/exclude.h"
#include "procedure/nodes/select.h"
#include "procedure/nodescopestack.h"
#include "classes/species.h"
#include "classes/site.h"
#include "base/lineparser.h"
#include "base/sysfunc.h"

// Constructor
ExcludeProcedureNode::ExcludeProcedureNode() : ProcedureNode(ProcedureNode::ExcludeNode)
{
	sameSiteA_ = NULL;
	sameSiteB_ = NULL;
}

// Destructor
ExcludeProcedureNode::~ExcludeProcedureNode()
{
}

/*
 * Identity
 */

// Return whether specified context is relevant for this node type
bool ExcludeProcedureNode::isContextRelevant(ProcedureNode::NodeContext context)
{
	return (context == ProcedureNode::AnalysisContext);
}

/*
 * Node Keywords
 */

// Return enum option info for ExcludeNodeKeyword
EnumOptions<ExcludeProcedureNode::ExcludeNodeKeyword> ExcludeProcedureNode::excludeNodeKeywords()
{
	static EnumOptionsList ExcludeNodeTypeKeywords = EnumOptionsList() <<
		EnumOption(ExcludeProcedureNode::EndExcludeKeyword,		"EndExclude") <<
		EnumOption(ExcludeProcedureNode::SameSiteKeyword,		"SameSite",	2);

	static EnumOptions<ExcludeProcedureNode::ExcludeNodeKeyword> options("ExcludeNodeKeyword", ExcludeNodeTypeKeywords);

	return options;
}

/*
 * Execute
 */

// Execute node, targetting the supplied Configuration
ProcedureNode::NodeExecutionResult ExcludeProcedureNode::execute(ProcessPool& procPool, Configuration* cfg, const char* prefix, GenericList& targetList)
{
	// Exclude based on Sites?
	if (sameSiteA_ && sameSiteB_)
	{
		if (sameSiteA_->currentSite() == sameSiteB_->currentSite()) return ProcedureNode::SomethingElse;
	}

	return ProcedureNode::Success;
}

/*
 * Read / Write
 */

// Read structure from specified LineParser
bool ExcludeProcedureNode::read(LineParser& parser, const CoreData& coreData, NodeScopeStack& scopeStack)
{
	// Read until we encounter the EndExclude keyword, or we fail for some reason
	while (!parser.eofOrBlank())
	{
		// Read and parse the next line
		if (parser.getArgsDelim() != LineParser::Success) return false;

		// Do we recognise this keyword and, if so, do we have the appropriate number of arguments?
		if (!excludeNodeKeywords().isValid(parser.argc(0))) return excludeNodeKeywords().errorAndPrintValid(parser.argc(0));
		ExcludeNodeKeyword nk = excludeNodeKeywords().enumeration(parser.argc(0));
		if (!excludeNodeKeywords().validNArgs(nk, parser.nArgs()-1)) return false;

		// All OK, so process it
		switch (nk)
		{
			case (ExcludeProcedureNode::EndExcludeKeyword):
				return true;
			case (ExcludeProcedureNode::SameSiteKeyword):
				// First Site argument
				sameSiteA_ = dynamic_cast<SelectProcedureNode*>(scopeStack.nodeInScope(parser.argc(1), ProcedureNode::SelectNode));
				if (!sameSiteA_) return Messenger::error("Unrecognised site reference '%s' given to %s keyword.\n", parser.argc(1), excludeNodeKeywords().keyword(ExcludeProcedureNode::SameSiteKeyword));

				// Second Site argument
				sameSiteB_ = dynamic_cast<SelectProcedureNode*>(scopeStack.nodeInScope(parser.argc(2), ProcedureNode::SelectNode));
				if (!sameSiteB_) return Messenger::error("Unrecognised site reference '%s' given to %s keyword.\n", parser.argc(2), excludeNodeKeywords().keyword(ExcludeProcedureNode::SameSiteKeyword));
				else 
				break;
			case (ExcludeProcedureNode::nExcludeNodeKeywords):
				return Messenger::error("Unrecognised Exclude node keyword '%s' found.\n", parser.argc(0));
				break;
			default:
				return Messenger::error("Epic Developer Fail - Don't know how to deal with the Exclude node keyword '%s'.\n", parser.argc(0));
		}
	}

	return true;
}

// Write structure to specified LineParser
bool ExcludeProcedureNode::write(LineParser& parser, const char* prefix)
{
	// Block Start
	if (!parser.writeLineF("%s%s\n", ProcedureNode::nodeTypes().keyword(type_))) return false;

	// Same Site Exclusion
	if (sameSiteA_ && sameSiteB_ && (!parser.writeLineF("%s  %s  '%s'  '%s'\n", prefix, excludeNodeKeywords().keyword(ExcludeProcedureNode::SameSiteKeyword), sameSiteA_->name(), sameSiteB_->name()))) return false;

	// Block End
	if (!parser.writeLineF("%s%s\n", excludeNodeKeywords().keyword(ExcludeProcedureNode::EndExcludeKeyword))) return false;

	return true;
}
