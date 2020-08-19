/*
    *** Forcefield Angle Term
    *** src/data/ffangleterm.h
    Copyright T. Youngs 2019-2020

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

#pragma once

#include "base/charstring.h"
#include "base/parameters.h"
#include "classes/speciesangle.h"

// Forward Declarations
class Forcefield;
class ForcefieldAtomType;

// Forcefield Angle Term
class ForcefieldAngleTerm
{
    public:
    ForcefieldAngleTerm(const char *typeI = NULL, const char *typeJ = NULL, const char *typeK = NULL,
                        SpeciesAngle::AngleFunction form = SpeciesAngle::NoForm, const std::vector<double> parameters = {});
    ~ForcefieldAngleTerm();

    /*
     * Data
     */
    private:
    // Type names involved in interaction
    CharString typeI_, typeJ_, typeK_;
    // Functional form of interaction
    SpeciesAngle::AngleFunction form_;
    // Parameters for interaction
    std::vector<double> parameters_;

    public:
    // Return if this term matches the atom types supplied
    bool isMatch(const ForcefieldAtomType &i, const ForcefieldAtomType &j, const ForcefieldAtomType &k) const;
    // Return functional form index of interaction
    SpeciesAngle::AngleFunction form() const;
    // Return array of parameters
    const std::vector<double> &parameters() const;
};
