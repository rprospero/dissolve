/*
	*** Isotopologues Definition
	*** src/classes/isotopologues.cpp
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

#include "classes/isotopologues.h"
#include "classes/coredata.h"
#include "classes/species.h"
#include "base/processpool.h"
#include "base/lineparser.h"
#include "base/sysfunc.h"

// Constructor
Isotopologues::Isotopologues() : ListItem<Isotopologues>()
{
	species_ = NULL;
	speciesPopulation_ = 0;
}

// Destructor
Isotopologues::~Isotopologues()
{
}

/*
 * Isotopologue Mix
 */

// Set associated Species
void Isotopologues::setSpecies(Species* sp, int population)
{
	species_ = sp;
	speciesPopulation_ = population;
}

// Return associated Species
Species* Isotopologues::species() const
{
	return species_;
}

// Return associated Species population
int Isotopologues::speciesPopulation() const
{
	return speciesPopulation_;
}

// Update Isotopologue RefList
void Isotopologues::update()
{
	// Go through list of Isotopologues present in this mix, removing any that no longer exist
	IsotopologueWeight* isoWeight = mix_.first(), *next;
	while (isoWeight)
	{
		next = isoWeight->next();

		// Check parent Species for presence of this Isotopologue
		if (!species_->hasIsotopologue(isoWeight->isotopologue()))
		{
			mix_.remove(isoWeight);
			Messenger::print("Removed Isotopologue from mixture for Species '%s' since it no longer existed.\n", species_->name());
		}

		isoWeight = next;
	}
}

// Add next available Isotopologue to list
bool Isotopologues::addNext()
{
	// NULL Pointer?
	if (species_ == NULL)
	{
		Messenger::error("NULL_POINTER - NULL Species pointer in Isotopologues::addNextIsotopologue().\n");
		return false;
	}
	
	// Check to see if the are any Isotopologues available to add
	if (mix_.nItems() == species_->nIsotopologues())
	{
		Messenger::warn("Can't add another Isotopologue to the mixture since there are none left for Species '%s'.\n", species_->name());
		return false;
	}
	
	// Find unique (unused) Isotopologue
	Isotopologue* iso;
	for (iso = species_->isotopologues().first(); iso != NULL; iso = iso->next()) if (!contains(iso)) break;

	if (iso == NULL)
	{
		Messenger::error("Couldn't find an unused Isotopologue in Species '%s'.\n", species_->name());
		return false;
	}
	
	IsotopologueWeight* newWeight = mix_.add();
	newWeight->set(iso, 1.0);
	
	return true;
}

// Add specific Isotopologue to list
bool Isotopologues::add(const Isotopologue* iso, double relativeWeight)
{
	// Search current list to see if the specified Isotopologue already exists
	if (contains(iso)) Messenger::warn("Isotopologue '%s' (of Species '%s') already exists in Isotopologues, and it is being added again...\n", iso->name(), species_->name());

	IsotopologueWeight* newWeight = mix_.add();
	newWeight->set(iso, relativeWeight);

	return true;
}

// Set Isotopologue component in list
bool Isotopologues::set(const Isotopologue* iso, double relativeWeight)
{
	// NULL Pointer?
	if (iso == NULL)
	{
		Messenger::error("NULL_POINTER - NULL Isotopologue passed to Isotopologues::setIsotopologue().\n");
		return false;
	}
	
	// Find this Isotopologue in the list
	IsotopologueWeight* isoWeight = NULL;
	for (isoWeight = mix_.first(); isoWeight != NULL; isoWeight = isoWeight->next()) if (isoWeight->isotopologue() == iso) break;
	if (isoWeight == NULL)
	{
		Messenger::warn("Warning: Isotopologues does not contain the Isotopologue '%s', so its relative weight can't be set.\n", iso->name());
		return false;
	}

	isoWeight->setWeight(relativeWeight);

	return true;
}

// Remove references to the specified Isotopologue
void Isotopologues::remove(const Isotopologue* iso)
{
	// Search the mix for the specified Isotopologue - it may appear more than once...
	IsotopologueWeight* isoWeight = mix_.first(), *nextWeight;
	while (isoWeight)
	{
		nextWeight = isoWeight->next();

		if (isoWeight->isotopologue() == iso) mix_.remove(isoWeight);

		isoWeight = nextWeight;
	}
}

// Remove the specified IsotopologueWeight
void Isotopologues::remove(IsotopologueWeight* isoWeight)
{
	mix_.remove(isoWeight);
}

// Return whether the mix contains the specified Isotopologue
const IsotopologueWeight* Isotopologues::contains(const Isotopologue* iso) const
{
	for (IsotopologueWeight* isoWeight = mix_.first(); isoWeight != NULL; isoWeight = isoWeight->next()) if (isoWeight->isotopologue() == iso) return isoWeight;

	return NULL;
}

// Return Isotopologue mix
const List<IsotopologueWeight>& Isotopologues::mix() const
{
	return mix_;
}

// Return number of Isotopologues in list
int Isotopologues::nIsotopologues() const
{
	return mix_.nItems();
}

// Return total relative population
double Isotopologues::totalRelative() const
{
	double total = 0.0;

	for (IsotopologueWeight* isoWeight : mix_) total += isoWeight->weight();

	return total;
}

// Normalise total relative population to 1.0
void Isotopologues::normalise()
{
	double total = totalRelative();

	for (IsotopologueWeight* isoWeight : mix_) isoWeight->setWeight(isoWeight->weight() / total);
}

/*
 * GenericItemBase Implementations
 */

// Return class name
const char* Isotopologues::itemClassName()
{
	return "Isotopologues";
}

// Read data through specified LineParser
bool Isotopologues::read(LineParser& parser, const CoreData& coreData)
{
	// Read Species name
	if (parser.getArgsDelim() != LineParser::Success) return false;
	species_ = coreData.findSpecies(parser.argc(0));
	if (species_ == NULL)
	{
		Messenger::error("Failed to find Species '%s' while reading Isotopologues.\n", parser.argc(0));
		return false;
	}
	speciesPopulation_ = parser.argi(1);
	int nIso = parser.argi(2);
	mix_.clear();
	for (int n=0; n<nIso; ++n)
	{
		if (parser.getArgsDelim() != LineParser::Success) return false;

		// Search for the named Isotopologue in the Species
		Isotopologue* iso = species_->findIsotopologue(parser.argc(0));
		if (!iso)
		{
			Messenger::error("Failed to find Isotopologue '%s' for Species '%s' while reading Isotopologues.\n", parser.argc(0), species_->name());
			return false;
		}

		if (!add(iso, parser.argd(1))) return false;
	}

	return true;
}

// Write data through specified LineParser
bool Isotopologues::write(LineParser& parser)
{
	// Write Species name, integer population, and number of isotopologues in the mix
	if (!parser.writeLineF("'%s'  %i  %i\n", species_->name(), speciesPopulation_, mix_.nItems())) return false;

	// Write Isotopologues
	for (IsotopologueWeight* isoWeight : mix_) if (!parser.writeLineF("%s  %f\n", isoWeight->isotopologue()->name(), isoWeight->weight())) return false;

	return true;
}

/*
 * Parallel Comms
 */

// Broadcast data
bool Isotopologues::broadcast(ProcessPool& procPool, const int root, const CoreData& coreData)
{
#ifdef PARALLEL
	CharString speciesName;
	if (procPool.poolRank() == root) speciesName = species_->name();
	procPool.broadcast(speciesName, root);
	species_ = coreData.findSpecies(speciesName);

	if (!procPool.broadcast(speciesPopulation_, root)) return false;

	// Isotopologue list (mix_)
	int nIso = mix_.nItems();
	if (!procPool.broadcast(nIso, root)) return false;
	int topIndex;
	double weight;
	if (procPool.poolRank() == root)
	{
		for (int n=0; n<nIso; ++n)
		{
			// Broadcast Isotopologue index data
			topIndex = species_->indexOfIsotopologue(mix_[n]->isotopologue());
			if (!procPool.broadcast(topIndex, root)) return false;

			// Broadcast relative population data
			weight = mix_[n]->weight();
			if (!procPool.broadcast(weight, root)) return false;
		}
	}
	else
	{
		mix_.clear();
		for (int n=0; n<nIso; ++n)
		{
			// Broadcast Isotopologue index data
			if (!procPool.broadcast(topIndex, root)) return false;

			// Broadcast relative population data
			if (!procPool.broadcast(weight, root)) return false;

			// Add mix data
			mix_.add()->set(species_->isotopologue(topIndex), weight);
		}
	}
#endif
	return true;
}

// Check item equality
bool Isotopologues::equality(ProcessPool& procPool)
{
#ifdef PARALLEL
	if (!procPool.equality(species_->name())) return Messenger::error("Isotopologues species is not equivalent (process %i has '%s').\n", procPool.poolRank(), species_->name());
	if (!procPool.equality(speciesPopulation_)) return Messenger::error("Isotopologues species population is not equivalent (process %i has %i).\n", procPool.poolRank(), speciesPopulation_);
	// Check number of isotopologues in mix
	if (!procPool.equality(mix_.nItems())) return Messenger::error("Isotopologues mix nItems is not equivalent (process %i has %i).\n", procPool.poolRank(), mix_.nItems());
	int count = 0;
	for (const IsotopologueWeight* isoWeight : mix_)
	{
		// Just check the name and the relative population
		if (!procPool.equality(isoWeight->isotopologue()->name())) return Messenger::error("Isotopologues isotopologue %i name is not equivalent (process %i has '%s').\n", count, procPool.poolRank(), isoWeight->isotopologue()->name());
		if (!procPool.equality(isoWeight->weight())) return Messenger::error("Isotopologues isotopologue %i relative population is not equivalent (process %i has '%s').\n", count, procPool.poolRank(), isoWeight->weight());
		++count;
	}
#endif
	return true;
}
