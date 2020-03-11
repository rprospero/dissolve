/*
	*** EnergyKernel
	*** src/classes/energykernel.cpp
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

#include "classes/energykernel.h"
#include "classes/box.h"
#include "classes/cell.h"
#include "classes/configuration.h"
#include "classes/potentialmap.h"
#include "classes/molecule.h"
#include "classes/species.h"
#include "base/processpool.h"
#include <iterator>

// Constructor
EnergyKernel::EnergyKernel(ProcessPool& procPool, Configuration* config, const PotentialMap& potentialMap, double energyCutoff) : configuration_(config), cells_(config->cells()), potentialMap_(potentialMap), processPool_(procPool)
{
	box_ = configuration_->box();
	cutoffDistanceSquared_ = (energyCutoff < 0.0 ? potentialMap_.range()*potentialMap_.range() : energyCutoff*energyCutoff);
}

// Destructor
EnergyKernel::~EnergyKernel()
{
}

/*
 * Internal Routines
 */

// Return PairPotential energy between atoms provided as pointers, at the distance specified
double EnergyKernel::pairPotentialEnergy(const Atom* i, const Atom* j, double r)
{
	return potentialMap_.energy(i, j, r);
}

// Return PairPotential energy between atoms provided as pointers (no minimum image calculation)
double EnergyKernel::energyWithoutMim(const Atom* i, const Atom* j)
{
	return pairPotentialEnergy(i, j, (i->r() - j->r()).magnitude());
}

// Return PairPotential energy between atoms provided as pointers (minimum image calculation)
double EnergyKernel::energyWithMim(const Atom* i, const Atom* j)
{
// 	Messenger::print("EnergyKernel::atoms(*,*) - energy %i-%i is %f at %f mim\n", min(i->arrayIndex(),j->arrayIndex()), max(i->arrayIndex(),j->arrayIndex()), pairPotentialEnergy(i->masterTypeIndex(), j->masterTypeIndex(), box_->minimumDistance(j, i)), box_->minimumDistance(j, i));
	return pairPotentialEnergy(i, j, box_->minimumDistance(j, i));
}

/*
 * PairPotential Terms
 */

// Return PairPotential energy between atoms (provided as pointers)
double EnergyKernel::energy(const Atom* i, const Atom* j, bool applyMim, bool excludeIgeJ)
{
#ifdef CHECKS
	if (i == NULL)
	{
		Messenger::error("NULL_POINTER - NULL Atom pointer (i) passed to EnergyKernel::energy(Atom,Atom,bool,bool).\n");
		return 0.0;
	}
	if (j == NULL)
	{
		Messenger::error("NULL_POINTER - NULL Atom pointer (j) passed to EnergyKernel::energy(Atom,Atom,bool,bool).\n");
		return 0.0;
	}
#endif
	// If Atoms are the same, we refuse to calculate
	if (i == j)
	{
// 		printf("Warning: Refusing to calculate self-energy in EnergyKernel::energy(Atom,Atom,bool,bool).\n");
		return 0.0;
	}
	
	// Check indices of Atoms if required
	if (excludeIgeJ && (i->arrayIndex() >= j->arrayIndex())) return 0.0;

	if (applyMim) return energyWithMim(i, j);
	else return energyWithoutMim(i, j);
}

// Return PairPotential energy between atoms in supplied cells
double EnergyKernel::energy(Cell* centralCell, Cell* otherCell, bool applyMim, bool excludeIgeJ, bool interMolecular, ProcessPool::DivisionStrategy strategy, bool performSum)
{
#ifdef CHECKS
	if (centralCell == NULL)
	{
		Messenger::error("NULL_POINTER - NULL central Cell pointer passed to EnergyKernel::energy(Cell,Cell,bool,bool).\n");
		return 0.0;
	}
	if (otherCell == NULL)
	{
		Messenger::error("NULL_POINTER - NULL other Cell pointer passed to EnergyKernel::energy(Cell,Cell,bool,bool).\n");
		return 0.0;
	}
#endif
	double totalEnergy = 0.0;
	std::set<Atom*>& centralAtoms = centralCell->atoms();
	std::set<Atom*>& otherAtoms = otherCell->atoms();
	Atom* ii, *jj;
	Vec3<double> rI;
	Molecule* molI;
	int i, j;
	double rSq, scale;
	auto central = centralAtoms.begin();

	// Get start/stride for specified loop context
	int start = processPool_.interleavedLoopStart(strategy);
	int stride = processPool_.interleavedLoopStride(strategy);

	// Loop over central cell atoms
	if (applyMim)
	{
		for (i = start; i < centralAtoms.size(); i += stride)
		{
			ii = *central;
			std::advance(central, stride);
			molI = ii->molecule();
			rI = ii->r();

			// Straight loop over other cell atoms
			for (auto* jj : otherAtoms)
			{
				// Check exclusion of I >= J
				if (excludeIgeJ && (ii >= jj)) continue;

				// Calculate rSquared distance betwenn atoms, and check it against the stored cutoff distance
				rSq = box_->minimumDistanceSquared(rI, jj->r());
				if (rSq > cutoffDistanceSquared_) continue;

				// Check for atoms in the same species
				if (molI != jj->molecule()) totalEnergy += pairPotentialEnergy(ii, jj, sqrt(rSq));
				else if (!interMolecular)
				{
					scale = ii->scaling(jj);
					if (scale > 1.0e-3) totalEnergy += pairPotentialEnergy(ii, jj, sqrt(rSq)) * scale;
				}
			}
		}
	}
	else
	{
		for (i = start; i < centralCell->atoms().size(); i += stride)
		{
			ii = *central;
			std::advance(central, stride);
			molI = ii->molecule();
			rI = ii->r();

			// Straight loop over other cell atoms
			for (auto* jj : otherAtoms)
			{
				// Check exclusion of I >= J
				if (excludeIgeJ && (ii >= jj)) continue;

				// Calculate rSquared distance betwenn atoms, and check it against the stored cutoff distance
				rSq = (rI - jj->r()).magnitudeSq();
				if (rSq > cutoffDistanceSquared_) continue;

				// Check for atoms in the same molecule
				if (molI != jj->molecule()) totalEnergy += pairPotentialEnergy(ii, jj, sqrt(rSq));
				else if (!interMolecular)
				{
					scale = ii->scaling(jj);
					if (scale > 1.0e-3) totalEnergy += pairPotentialEnergy(ii, jj, sqrt(rSq)) * scale;
				}
			}
		}
	}

	// Perform relevant sum if requested
	if (performSum) processPool_.allSum(&totalEnergy, 1, strategy);

	return totalEnergy;
}

// Return PairPotential energy between cell and atomic neighbours
double EnergyKernel::energy(Cell* centralCell, bool excludeIgeJ, bool interMolecular, ProcessPool::DivisionStrategy strategy, bool performSum)
{
	double totalEnergy = 0.0;
	std::set<Atom*>& centralAtoms = centralCell->atoms();
	Atom* ii, *jj;
	Vec3<double> rJ;
	int i, j;
	Molecule* molJ;
	double rSq, scale;

	// Get start/stride for specified loop context
	int start = processPool_.interleavedLoopStart(strategy);
	int stride = processPool_.interleavedLoopStride(strategy);

	// Straight loop over Cells *not* requiring mim
	for (auto* otherCell : centralCell->cellNeighbours())
	{
		std::set<Atom*>& otherAtoms = otherCell->atoms();

		for (auto* jj : otherAtoms)
		{
			molJ = jj->molecule();
			rJ = jj->r();

			auto central = centralAtoms.begin();
			// Loop over central cell atoms
			std::advance(central, start);
			for (i = start; i < centralAtoms.size(); i += stride)
			{
                                ii = *central;
				std::advance(central, stride);

				// Check exclusion of I >= J (comparison by pointer)
				if (excludeIgeJ && (ii >= jj)) continue;

				// Calculate rSquared distance betwenn atoms, and check it against the stored cutoff distance
				rSq = (ii->r() - rJ).magnitudeSq();
				if (rSq > cutoffDistanceSquared_) continue;

				// Check for atoms in the same species
				if (ii->molecule() != molJ) totalEnergy += pairPotentialEnergy(jj, ii, sqrt(rSq));
				else if (!interMolecular)
				{
					scale = ii->scaling(jj);
					if (scale > 1.0e-3) totalEnergy += pairPotentialEnergy(jj, ii, sqrt(rSq)) * scale;
				}
			}
		}
	}

	// Straight loop over Cells requiring mim
	for (auto* otherCell : centralCell->mimCellNeighbours())
	{
		std::set<Atom*>& otherAtoms = otherCell->atoms();

		for (auto* jj : otherAtoms)
		{
			molJ = jj->molecule();
			rJ = jj->r();

			auto central = centralAtoms.begin();
			// Loop over central cell atoms
			std::advance(central, start);
			for (i = start; i < centralAtoms.size(); i += stride)
			{
				ii = *central;
				std::advance(central, stride);

				// Check exclusion of I >= J (comparison by pointer)
				if (excludeIgeJ && (ii >= jj)) continue;

				// Calculate rSquared distance betwenn atoms, and check it against the stored cutoff distance
				rSq = box_->minimumDistanceSquared(ii->r(), rJ);
				if (rSq > cutoffDistanceSquared_) continue;

				// Check for atoms in the same species
				if (ii->molecule() != molJ) totalEnergy += pairPotentialEnergy(jj, ii, sqrt(rSq));
				else if (!interMolecular)
				{
					scale = ii->scaling(jj);
					if (scale > 1.0e-3) totalEnergy += pairPotentialEnergy(jj, ii, sqrt(rSq)) * scale;
				}
			}
		}
	}

	// Perform relevant sum if requested
	if (performSum) processPool_.allSum(&totalEnergy, 1, strategy);

	return totalEnergy;
}

// Return PairPotential energy between Atom and Cell contents
double EnergyKernel::energy(const Atom* i, Cell* cell, int flags, ProcessPool::DivisionStrategy strategy, bool performSum)
{
#ifdef CHECKS
	if (i == NULL)
	{
		Messenger::error("NULL_POINTER - NULL atom pointer passed to EnergyKernel::energy(Atom,Cell).\n");
		return 0.0;
	}
	if (cell == NULL)
	{
		Messenger::error("NULL_POINTER - NULL Cell pointer passed to EnergyKernel::energy(Atom,Cell).\n");
		return 0.0;
	}
#endif
	double totalEnergy = 0.0;
	Atom* jj;
	int j;
	double rSq, scale;
	std::set<Atom*>& otherAtoms = cell->atoms();
	auto other = otherAtoms.begin();
	int nOtherAtoms = cell->nAtoms();
	
	// Grab some information on the supplied Atom
	Molecule* moleculeI = i->molecule();
	const Vec3<double> rI = i->r();

	// Get start/stride for specified loop context
	int start = processPool_.interleavedLoopStart(strategy);
	int stride = processPool_.interleavedLoopStride(strategy);
	std::advance(other, start);

	if (flags&KernelFlags::ApplyMinimumImageFlag)
	{
		// Loop over other Atoms
	  if (flags&KernelFlags::ExcludeSelfFlag) for (j=start; j<nOtherAtoms; j += stride)
		{
			// Grab other Atom pointer
			jj = *other;
			std::advance(other, stride);

			// Check for same atom
			if (i == jj) continue;
			
			// Calculate rSquared distance between atoms, and check it against the stored cutoff distance
			rSq = box_->minimumDistanceSquared(rI, jj->r());
			if (rSq > cutoffDistanceSquared_) continue;

			// Check for atoms in the same species
			if (moleculeI != jj->molecule()) totalEnergy += pairPotentialEnergy(i, jj, sqrt(rSq));
			else
			{
				scale = i->scaling(jj);
				if (scale > 1.0e-3) totalEnergy += pairPotentialEnergy(i, jj, sqrt(rSq)) * scale;
			}
		}
		else if (flags&KernelFlags::ExcludeIGEJFlag) for (j=start; j<nOtherAtoms; j += stride)
		{
			// Grab other Atom pointer
			jj = *other;
			std::advance(other, stride);

			// Pointer comparison for i >= jj
			if (i >= jj) continue;

			// Calculate rSquared distance between atoms, and check it against the stored cutoff distance
			rSq = box_->minimumDistanceSquared(rI, jj->r());
			if (rSq > cutoffDistanceSquared_) continue;

			// Check for atoms in the same species
			if (moleculeI != jj->molecule()) totalEnergy += pairPotentialEnergy(i, jj, sqrt(rSq));
			else
			{
				scale = i->scaling(jj);
				if (scale > 1.0e-3) totalEnergy += pairPotentialEnergy(i, jj, sqrt(rSq)) * scale;
			}
		}
		else if (flags&KernelFlags::ExcludeIntraIGEJFlag) for (j=start; j<nOtherAtoms; j += stride)
		{
			// Grab other Atom pointer
			jj = *other;
			std::advance(other, stride);

			// Calculate rSquared distance between atoms, and check it against the stored cutoff distance
			rSq = box_->minimumDistanceSquared(rI, jj->r());
			if (rSq > cutoffDistanceSquared_) continue;

			// Check for atoms in the same species
			if (moleculeI != jj->molecule()) totalEnergy += pairPotentialEnergy(i, jj, sqrt(rSq));
			else
			{
				// Pointer comparison for i >= jj
				if (i >= jj) continue;

				scale = i->scaling(jj);
				if (scale > 1.0e-3) totalEnergy += pairPotentialEnergy(i, jj, sqrt(rSq)) * scale;
			}
		}
		else for (j=start; j<nOtherAtoms; j += stride)
		{
			// Grab other Atom pointer
			jj = *other;
			std::advance(other, stride);

			// Calculate rSquared distance between atoms, and check it against the stored cutoff distance
			rSq = box_->minimumDistanceSquared(rI, jj->r());
			if (rSq > cutoffDistanceSquared_) continue;

			// Check for atoms in the same species
			if (moleculeI != jj->molecule()) totalEnergy += pairPotentialEnergy(i, jj, sqrt(rSq));
			else
			{
				scale = i->scaling(jj);
				if (scale > 1.0e-3) totalEnergy += pairPotentialEnergy(i, jj, sqrt(rSq)) * scale;
			}
		}
	}
	else
	{
		// Loop over atom neighbours
		if (flags&KernelFlags::ExcludeSelfFlag) for (j=start; j<nOtherAtoms; j += stride)
		{
			// Grab other Atom pointer
			jj = *other;
			std::advance(other, stride);

			// Check for same atom
			if (i == jj) continue;

			// Calculate rSquared distance between atoms, and check it against the stored cutoff distance
			rSq = (rI - jj->r()).magnitudeSq();
			if (rSq > cutoffDistanceSquared_) continue;

			// Check for atoms in the same species
			if (moleculeI != jj->molecule()) totalEnergy += pairPotentialEnergy(i, jj, sqrt(rSq));
			else
			{
				scale = i->scaling(jj);
				if (scale > 1.0e-3) totalEnergy += pairPotentialEnergy(i, jj, sqrt(rSq)) * scale;
			}
		}
		else if (flags&KernelFlags::ExcludeIGEJFlag) for (j=start; j<nOtherAtoms; j += stride)
		{
			// Grab other Atom pointer
			jj = *other;
			std::advance(other, stride);

			// Pointer comparison for i >= jj
			if (i >= jj) continue;

			// Calculate rSquared distance between atoms, and check it against the stored cutoff distance
			rSq = (rI - jj->r()).magnitudeSq();
			if (rSq > cutoffDistanceSquared_) continue;

			// Check for atoms in the same species
			if (moleculeI != jj->molecule()) totalEnergy += pairPotentialEnergy(i, jj, sqrt(rSq));
			else
			{
				scale = i->scaling(jj);
				if (scale > 1.0e-3) totalEnergy += pairPotentialEnergy(i, jj, sqrt(rSq)) * scale;
			}
		}
		else if (flags&KernelFlags::ExcludeIntraIGEJFlag) for (j=start; j<nOtherAtoms; j += stride)
		{
			// Grab other Atom pointer
			jj = *other;
			std::advance(other, stride);

			// Calculate rSquared distance between atoms, and check it against the stored cutoff distance
			rSq = (rI - jj->r()).magnitudeSq();
			if (rSq > cutoffDistanceSquared_) continue;

			// Check for atoms in the same species
			if (moleculeI != jj->molecule()) totalEnergy += pairPotentialEnergy(i, jj, sqrt(rSq));
			else
			{
				// Pointer comparison for i >= jj
				if (i >= jj) continue;

				scale = i->scaling(jj);
				if (scale > 1.0e-3) totalEnergy += pairPotentialEnergy(i, jj, sqrt(rSq)) * scale;
			}
		}
		else for (j=start; j<nOtherAtoms; j += stride)
		{
			// Grab other Atom pointer
			jj = *other;
			std::advance(other, stride);

			// Calculate rSquared distance between atoms, and check it against the stored cutoff distance
			rSq = (rI - jj->r()).magnitudeSq();
			if (rSq > cutoffDistanceSquared_) continue;

			// Check for atoms in the same species
			if (moleculeI != jj->molecule()) totalEnergy += pairPotentialEnergy(i, jj, sqrt(rSq));
			else
			{
				scale = i->scaling(jj);
				if (scale > 1.0e-3) totalEnergy += pairPotentialEnergy(i, jj, sqrt(rSq)) * scale;
			}
		}
	}

	// Perform relevant sum if requested
	if (performSum) processPool_.allSum(&totalEnergy, 1, strategy);

	return totalEnergy;
}

// Return PairPotential energy of Atom with world
double EnergyKernel::energy(const Atom* i, ProcessPool::DivisionStrategy strategy, bool performSum)
{
#ifdef CHECKS
	if (i == NULL)
	{
		Messenger::error("NULL_POINTER - NULL Atom pointer passed to EnergyKernel::energy(Atom,ParallelStyle).\n");
		return 0.0;
	}
#endif
	Cell* cellI = i->cell();

	// This Atom with its own Cell
	double totalEnergy = energy(i, cellI, KernelFlags::ExcludeSelfFlag, strategy, false);

	// Cell neighbours not requiring minimum image
	for (auto* neighbour : cellI->cellNeighbours()) totalEnergy += energy(i, neighbour, KernelFlags::NoFlags, strategy, false);

	// Cell neighbours requiring minimum image
	for (auto* neighbour : cellI->mimCellNeighbours()) totalEnergy += energy(i, neighbour, KernelFlags::ApplyMinimumImageFlag, strategy, false);

	// Perform relevant sum if requested
	if (performSum) processPool_.allSum(&totalEnergy, 1, strategy);

	return totalEnergy;
}

// Return PairPotential energy of Molecule with world
double EnergyKernel::energy(const Molecule* mol, ProcessPool::DivisionStrategy strategy, bool performSum)
{
	Atom* ii;
	Cell* cellI;
	double totalEnergy = 0.0;

	for (int i = 0; i<mol->nAtoms(); ++i)
	{
		ii = mol->atom(i);
		cellI = ii->cell();

		// This Atom with its own Cell
		totalEnergy += energy(ii, cellI, KernelFlags::ExcludeIntraIGEJFlag, strategy, false);

		// Cell neighbours not requiring minimum image
		for (auto* neighbour : cellI->cellNeighbours()) totalEnergy += energy(ii, neighbour, KernelFlags::ExcludeIntraIGEJFlag, strategy, false);

		// Cell neighbours requiring minimum image
		for (auto* neighbour : cellI->mimCellNeighbours()) totalEnergy += energy(ii, neighbour, KernelFlags::ApplyMinimumImageFlag | KernelFlags::ExcludeIntraIGEJFlag, strategy, false);
	}

	// Perform relevant sum if requested
	if (performSum) processPool_.allSum(&totalEnergy, 1, strategy);

	return totalEnergy;
}

// Return molecular correction energy related to intramolecular terms involving supplied atom
double EnergyKernel::correct(const Atom* i)
{
	// Loop over atoms in molecule
	int nMolAtoms = i->molecule()->nAtoms();
	Atom* j;
	std::vector<Atom*> atoms = i->molecule()->atoms();
	double scale, r, correctionEnergy = 0.0;
	Vec3<double> rI = i->r();

	for (auto* j : atoms)
	{
		if (i == j) continue;
		scale = 1.0 - i->scaling(j);
		if (scale > 1.0e-3)
		{
			r = box_->minimumDistance(rI, j->r());
			correctionEnergy += pairPotentialEnergy(i, j, r) * scale;
		}
	}

	return -correctionEnergy;
}

// Return total interatomic PairPotential energy of the system
double EnergyKernel::energy(const CellArray& cellArray, bool interMolecular, ProcessPool::DivisionStrategy strategy, bool performSum)
{
	// Get sub-strategy to use
	ProcessPool::DivisionStrategy subStrategy = ProcessPool::subDivisionStrategy(strategy);

	// Set start/stride for parallel loop
	int start = processPool_.interleavedLoopStart(strategy);
	int stride = processPool_.interleavedLoopStride(strategy);

	double totalEnergy = 0.0;
	Cell* cell;
	for (int cellId = start; cellId<cellArray.nCells(); cellId += stride)
	{
		cell = cellArray.cell(cellId);

		// This cell with itself
		totalEnergy += energy(cell, cell, false, true, interMolecular, subStrategy, performSum);

		// Interatomic interactions between atoms in this cell and its neighbours
		totalEnergy += energy(cell, true, interMolecular, subStrategy, performSum);
	}

	return totalEnergy;
}

/*
 * Intramolecular Terms
 */

// Return SpeciesBond energy
double EnergyKernel::energy(const SpeciesBond* b, const Atom* i, const Atom* j)
{
#ifdef CHECKS
	// Check for spurious bond distances
	double distance = i->cell()->mimRequired(j->cell()) ? box_->minimumDistance(i, j) : (i->r() - j->r()).magnitude();
	if (distance > 5.0) printf("!!! Long bond: %i-%i = %f Angstroms\n", i->arrayIndex(), j->arrayIndex(), distance);
#endif
	// Determine whether we need to apply minimum image to the distance calculation
	if (i->cell()->mimRequired(j->cell())) return b->energy(box_->minimumDistance(i, j));
	else return b->energy((i->r() - j->r()).magnitude());
}

// Return SpeciesAngle energy
double EnergyKernel::energy(const SpeciesAngle* a, const Atom* i, const Atom* j, const Atom* k)
{
	Vec3<double> vecji, vecjk;

	// Determine whether we need to apply minimum image between 'j-i' and 'j-k'
	if (j->cell()->mimRequired(i->cell())) vecji = box_->minimumVector(j, i);
	else vecji = i->r() - j->r();
	if (j->cell()->mimRequired(k->cell())) vecjk = box_->minimumVector(j, k);
	else vecjk = k->r() - j->r();

	// Normalise vectors
	vecji.normalise();
	vecjk.normalise();

	// Determine Angle energy
	return a->energy(Box::angleInDegrees(vecji, vecjk));
}

// Return SpeciesTorsion energy
double EnergyKernel::energy(const SpeciesTorsion* t, const Atom* i, const Atom* j, const Atom* k, const Atom* l)
{
	Vec3<double> vecji, vecjk, veckl, xpj, xpk, dcos_dxpj, dcos_dxpk, temp, force;
	Matrix3 dxpj_dij, dxpj_dkj, dxpk_dkj, dxpk_dlk;
	
	// Calculate vectors, ensuring we account for minimum image
	if (j->cell()->mimRequired(i->cell())) vecji = box_->minimumVector(j, i);
	else vecji = i->r() - j->r();
	if (j->cell()->mimRequired(k->cell())) vecjk = box_->minimumVector(j, k);
	else vecjk = k->r() - j->r();
	if (k->cell()->mimRequired(l->cell())) veckl = box_->minimumVector(k, l);
	else veckl = l->r() - k->r();

	return t->energy(Box::torsionInDegrees(vecji, vecjk, veckl));
}

// Return intramolecular energy for the supplied Atom
double EnergyKernel::intramolecularEnergy(const Molecule* mol, const Atom* i)
{
#ifdef CHECKS
	if (i == NULL)
	{
		Messenger::error("NULL Atom given to EnergyKernel::intraEnergy().\n");
		return 0.0;
	}
	if (i->speciesAtom() == NULL)
	{
		Messenger::error("NULL SpeciesAtom in Atom given to EnergyKernel::intraEnergy().\n");
		return 0.0;
	}
#endif
	// Get the SpeciesAtom
	const SpeciesAtom* spAtom = i->speciesAtom();

	// If no terms are present, return zero
	if ((spAtom->nBonds() == 0) && (spAtom->nAngles() == 0) && (spAtom->nTorsions() == 0)) return 0.0;

	double intraEnergy = 0.0;

	// Add energy from SpeciesBond terms
	for (const auto* bond : spAtom->bonds())
	{
		intraEnergy += energy(bond, mol->atom(bond->indexI()), mol->atom(bond->indexJ()));
	}

	// Add energy from SpeciesAngle terms
	for (const auto* angle : spAtom->angles())
	{
		intraEnergy += energy(angle, mol->atom(angle->indexI()), mol->atom(angle->indexJ()), mol->atom(angle->indexK()));
	}

	// Add energy from SpeciesTorsion terms
	for (const auto* torsion : spAtom->torsions())
	{
		intraEnergy += energy(torsion, mol->atom(torsion->indexI()), mol->atom(torsion->indexJ()), mol->atom(torsion->indexK()), mol->atom(torsion->indexL()));
	}

	return intraEnergy;
}

// Return intramolecular energy for the supplied Molecule
double EnergyKernel::intramolecularEnergy(const Molecule* mol)
{
#ifdef CHECKS
	if (mol == NULL)
	{
		Messenger::error("NULL Molecule pointer given to EnergyKernel::intramolecularEnergy.\n");
		return 0.0;
	}
#endif

	double intraEnergy = 0.0;

	const Species* sp = mol->species();

	// Loop over Bonds
	DynamicArrayConstIterator<SpeciesBond> bondIterator(mol->species()->constBonds());
	while (const SpeciesBond* b = bondIterator.iterate()) energy(b, mol->atom(b->indexI()), mol->atom(b->indexJ()));

	// Loop over Angles
	DynamicArrayConstIterator<SpeciesAngle> angleIterator(mol->species()->constAngles());
	while (const SpeciesAngle* a = angleIterator.iterate()) energy(a, mol->atom(a->indexI()), mol->atom(a->indexJ()), mol->atom(a->indexK()));

	// Loop over Torsions
	DynamicArrayConstIterator<SpeciesTorsion> torsionIterator(mol->species()->constTorsions());
	while (const SpeciesTorsion* t = torsionIterator.iterate()) energy(t, mol->atom(t->indexI()), mol->atom(t->indexJ()), mol->atom(t->indexK()), mol->atom(t->indexL()));

	return intraEnergy;
}
