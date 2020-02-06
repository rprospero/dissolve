/*
	*** Species Definition - Transforms
	*** src/classes/species_transform.cpp
	Copyright T. Youngs 2012-2020

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

#include "classes/species.h"
#include "classes/box.h"

// Calculate and return centre of geometry
Vec3<double> Species::centreOfGeometry(const Box* box) const
{
	if (atoms_.size() == 0) return Vec3<double>();
	
	// Calculate center relative to first atom in molecule
	Vec3<double> cog = atoms_.front()->r();
	for (auto i = atoms_.begin()+1; i != atoms_.end(); ++i) cog += box->minimumImage((*i)->r(), cog);

	return (cog / atoms_.size());
}

// Set centre of geometry of species
void Species::setCentre(const Box* box, const Vec3<double> newCentre)
{
	// Calculate Molecule centre of geometry
	Vec3<double> newR, cog = centreOfGeometry(box);

	// Apply transform
	for (int n=0; n<atoms_.size(); ++n)
	for (SpeciesAtom* i : atoms_)
	{
		newR = box->minimumVector(i->r(), cog) + newCentre;
		i->setCoordinates(newR);
	}
}

// Centre coordinates at origin
void Species::centreAtOrigin()
{
	Vec3<double> centre;
	for (SpeciesAtom* i : atoms_) centre += i->r();
	centre /= atoms_.size();
	for (SpeciesAtom* i : atoms_) i->translateCoordinates(-centre);
}

