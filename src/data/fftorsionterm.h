/*
	*** Forcefield Torsion Term
	*** src/data/fftorsionterm.h
	Copyright T. Youngs 2019

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

#ifndef DISSOLVE_FORCEFIELDTORSIONTERM_H
#define DISSOLVE_FORCEFIELDTORSIONTERM_H

#include "classes/speciestorsion.h"
#include "base/charstring.h"
#include "base/parameters.h"

// Forward Declarations
class Forcefield;
class ForcefieldAtomType;

// Forcefield Torsion Term
class ForcefieldTorsionTerm : public ListItem<ForcefieldTorsionTerm>
{
	public:
	// Constructor / Destructor
	ForcefieldTorsionTerm(Forcefield* parent = NULL, const char* typeI = NULL, const char* typeJ = NULL, const char* typeK = NULL, const char* typeL = NULL, SpeciesTorsion::TorsionFunction form = SpeciesTorsion::nTorsionFunctions, double data0 = 0.0, double data1 = 0.0, double data2 = 0.0, double data3 = 0.0);
	~ForcefieldTorsionTerm();


	/*
	 * Data
	 */
	private:
	// Type names involved in interaction
	CharString typeI_, typeJ_, typeK_, typeL_;
	// Functional form of interaction
	SpeciesTorsion::TorsionFunction form_;
	// Parameters for interaction
	vector<double> parameters_;

	public:
	// Return if this term matches the atom types supplied
	bool matches(const ForcefieldAtomType* i, const ForcefieldAtomType* j, const ForcefieldAtomType* k, const ForcefieldAtomType* l);
	// Return functional form index of interaction
	SpeciesTorsion::TorsionFunction form() const;
	// Return array of parameters
	const vector<double> parameters() const;
};

#endif
