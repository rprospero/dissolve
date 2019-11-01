/*
	*** Forcefield Torsion Term
	*** src/data/fftorsionterm.cpp
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

#include "data/fftorsionterm.h"
#include "data/ffatomtype.h"
#include "data/ff.h"

// Constructor
ForcefieldTorsionTerm::ForcefieldTorsionTerm(Forcefield* parent, const char* typeI, const char* typeJ, const char* typeK, const char* typeL, SpeciesTorsion::TorsionFunction form, double data0, double data1, double data2, double data3)
{
	typeI_ = typeI;
	typeJ_ = typeJ;
	typeK_ = typeK;
	typeL_ = typeL;
	form_ = form;
	parameters_[0] = data0;
	parameters_[1] = data1;
	parameters_[2] = data2;
	parameters_[3] = data3;

	// Register this atom type with the parent forcefield
	if (parent) parent->registerTorsionTerm(this);
}

// Destructor
ForcefieldTorsionTerm::~ForcefieldTorsionTerm()
{
}

/*
 * Data
 */

// Return if this term matches the atom types supplied
bool ForcefieldTorsionTerm::matches(const ForcefieldAtomType* i, const ForcefieldAtomType* j, const ForcefieldAtomType* k, const ForcefieldAtomType* l)
{
	if ((typeI_ == i->typeName()) && (typeJ_ == j->typeName()) && (typeK_ == k->typeName()) && (typeL_ == l->typeName())) return true;
	if ((typeL_ == i->typeName()) && (typeK_ == j->typeName()) && (typeJ_ == k->typeName()) && (typeI_ == l->typeName())) return true;

	return false;
}

// Return functional form index of interaction
SpeciesTorsion::TorsionFunction ForcefieldTorsionTerm::form() const
{
	return form_;
}

// Return array of parameters
const vector<double> ForcefieldTorsionTerm::parameters() const
{
	return parameters_;
}
