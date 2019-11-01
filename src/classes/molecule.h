/*
	vector<**> Molecule
	vector<**> src/classes/molecule.h
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

#ifndef DISSOLVE_MOLECULE_H
#define DISSOLVE_MOLECULE_H

#include "templates/array.h"
#include "templates/dynamicarrayobject.h"
#include "templates/reflist.h"
#include "templates/vector3.h"
#include <vector>

// Forward Declarations
class Angle;
class Atom;
class Bond;
class Box;
class Grain;
class Species;
class SpeciesAtom;
class SpeciesGrain;
class Matrix3;
class Torsion;

// Molecule Definition
class Molecule : public DynamicArrayObject<Molecule>
{
	/*
	 * A Molecule can be thought of as a collection of Atoms / Grains that belong together, and can be moved as such.
	 * A Molecule does not itself own its Atoms or Grains. Instead, pointers to the relevant Atoms/Grains in the parent
	 * Configuration are stored.
	 */
	public:
	// Constructor
	Molecule();
	// Destructor
	~Molecule();


	/*
	 * DynamicArrayObject Virtuals
	 */
	public:
	// Clear object, ready for re-use
	void clear();


	/*
	 * Contents
	 */
	private:
	// Species which the Molecule represents
	Species* species_;
	// Array of pointers to Atoms
	Array<Atom*> atoms_;
	// Array of Grain pointers
	Array<Grain*> grains_;
	// Array of Bonds this Molecule contains
	Array<Bond*> bonds_;
	// Array of Angles this Molecule contains
	Array<Angle*> angles_;
	// Array of Torsions this Molecule contains
	Array<Torsion*> torsions_;

	public:
	// Set Species which the Molecule represents
	void setSpecies(Species* sp);
	// Return Species which the Molecule represents
	Species* species() const;
	// Add Atom to Molecule
	void addAtom(Atom* i);
	// Return size of Atom array
	int nAtoms() const;
	// Return Atoms array
	vector<Atom*> atoms();
	// Return nth Atom pointer
	Atom* atom(int n) const;
	// Add Grain to Molecule
	void addGrain(Grain* grain);
	// Return size of Grain array
	int nGrains() const;
	// Return nth Grain pointer
	Grain* grain(int n);
	// Add Bond to Molecule
	void addBond(Bond* bond);
	// Return size of Bond array
	int nBonds() const;
	// Return Bonds array
	Array<Bond*> bonds();
	// Return nth Bond pointer
	Bond* bond(int n) const;
	// Add Angle to Molecule
	void addAngle(Angle* angle);
	// Return size of Angle array
	int nAngles() const;
	// Return Angles array
	Array<Angle*> angles();
	// Return nth Angle pointer
	Angle* angle(int n) const;
	// Add Torsion to Molecule
	void addTorsion(Torsion* torsion);
	// Return size of Torsion array
	int nTorsions() const;
	// Return Torsions array
	Array<Torsion*> torsions();
	// Return nth Torsion pointer
	Torsion* torsion(int n) const;


	/*
	 * Upkeep
	 */
	public:
	// Select Atoms along any path from the specified one
	void selectFromAtom(Atom* i, RefList<Atom>& selectedAtoms, Bond* excludedBond1 = NULL, Bond* excludedBond2 = NULL);


	/*
	 * Manipulations
	 */
	public:
	// Set centre of geometry
	void setCentreOfGeometry(const Box* box, const Vec3<double> newCentre);
	// Calculate and return centre of geometry
	Vec3<double> centreOfGeometry(const Box* box) const;
	// Transform molecule with supplied matrix, using centre of geometry as the origin
	void transform(const Box* box, const Matrix3& transformationMatrix);
	// Transform selected atoms with supplied matrix, around specified origin
	void transform(const Box* box, const Matrix3& transformationMatrix, const Vec3< double >& origin, int nTargetAtoms, int* targetAtoms);
	// Translate whole molecule by the delta specified
	void translate(const Vec3<double> delta);
	// Translate specified atoms by the delta specified
	void translate(const Vec3<double>& delta, int nTargetAtoms, int* targetAtoms);
};

#endif
