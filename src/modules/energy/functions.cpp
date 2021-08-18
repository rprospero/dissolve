// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "classes/configuration.h"
#include "classes/energykernel.h"
#include "classes/potentialmap.h"
#include "classes/species.h"
#include "math/combinations.h"
#include "modules/energy/energy.h"
#include "templates/algorithms.h"
#include "templates/parallel_defs.h"
#include <atomic>
#include <numeric>
#include <CL/sycl.hpp>

namespace
{
// Structure to store energy values
struct Energies
{
    Energies() = default;
    double bondEnergy;
    double angleEnergy;
    double torsionEnergy;
    double improperEnergy;

    Energies operator+(const Energies &other) const
    {
        return {this->bondEnergy + other.bondEnergy, this->angleEnergy + other.angleEnergy,
                this->torsionEnergy + other.torsionEnergy, this->improperEnergy + other.improperEnergy};
    }
};
} // namespace

// Return total interatomic energy of Configuration
double EnergyModule::interAtomicEnergy(ProcessPool &procPool, const Configuration *cfg, const PotentialMap &potentialMap)
{
    /*
     * Calculates the total interatomic energy of the system, i.e. the energy contributions from PairPotential
     * interactions between individual Atoms, accounting for intramolecular terms
     *
     * This is a parallel routine, with processes operating as process groups.
     */

    // Create an EnergyKernel
    EnergyKernel kernel(procPool, cfg, potentialMap);

    // Set the strategy
    ProcessPool::DivisionStrategy strategy = ProcessPool::PoolStrategy;

    // Grab the Cell array and calculate total energy
    const auto &cellArray = cfg->cells();
    double totalEnergy = kernel.energy(cellArray, false, strategy, false);

    // Print process-local energy
    Messenger::printVerbose("Interatomic Energy (Local) is {:15.9e}\n", totalEnergy);

    // Sum energy over all processes in the pool and print
    procPool.allSum(&totalEnergy, 1, strategy);
    Messenger::printVerbose("Interatomic Energy (World) is {:15.9e}\n", totalEnergy);

    return totalEnergy;
}

// Return total interatomic energy of Species
double EnergyModule::interAtomicEnergy(ProcessPool &procPool, const Species *sp, const PotentialMap &potentialMap)
{
    const auto cutoff = potentialMap.range();

    // Get start/end for loop
    Combinations comb(sp->nAtoms(), 2);
    auto offset = procPool.interleavedLoopStart(ProcessPool::PoolStrategy);
    auto nChunks = procPool.interleavedLoopStride(ProcessPool::PoolStrategy);
    auto [loopStart, loopEnd] = chop_range(0, comb.getNumCombinations(), nChunks, offset);
    double energy =
        dissolve::transform_reduce(ParallelPolicies::par, dissolve::counting_iterator<int>(loopStart),
                                   dissolve::counting_iterator<int>(loopEnd), 0.0, std::plus<double>(), [&](const auto idx) {
                                       auto [n, m] = comb.nthCombination(idx);
                                       auto &i = sp->atom(n);
                                       auto &j = sp->atom(m);
                                       auto &rI = i.r();
                                       auto &rJ = j.r();

                                       // Get interatomic distance
                                       double r = (rJ - rI).magnitude();
                                       if (r > cutoff)
                                           return 0.0;

                                       // Get intramolecular sca&ng of atom pair
                                       double scale = i.scaling(&j);
                                       if (scale < 1.0e-3)
                                           return 0.0;

                                       return potentialMap.energy(i, j, r) * scale;
                                   });
    return energy;

}

// Return total intermolecular energy of Configuration
double EnergyModule::interMolecularEnergy(ProcessPool &procPool, const Configuration *cfg, const PotentialMap &potentialMap)
{
    /*
     * Calculates the total intermolecular energy of the system, i.e. the energy contributions from PairPotential
     * interactions between individual Atoms of different Molecules, thus neglecting intramolecular terms
     *
     * This is a parallel routine, with processes operating as process groups.
     */

    // Create an EnergyKernel
    EnergyKernel kernel(procPool, cfg, potentialMap);

    // Set the strategy
    ProcessPool::DivisionStrategy strategy = ProcessPool::PoolStrategy;

    // Grab the Cell array and calculate total energy
    const auto &cellArray = cfg->cells();
    double totalEnergy = kernel.energy(cellArray, true, strategy, false);

    // Print process-local energy
    Messenger::printVerbose("Intermolecular Energy (Local) is {:15.9e}\n", totalEnergy);

    // Sum energy over all processes in the pool and print
    procPool.allSum(&totalEnergy, 1, strategy);
    Messenger::printVerbose("Intermolecular Energy (World) is {:15.9e}\n", totalEnergy);

    return totalEnergy;
}

// Return total intramolecular energy of Configuration
double EnergyModule::intraMolecularEnergy(ProcessPool &procPool, const Configuration *cfg, const PotentialMap &potentialMap)
{
    double bondEnergy, angleEnergy, torsionEnergy, improperEnergy;

    return intraMolecularEnergy(procPool, cfg, potentialMap, bondEnergy, angleEnergy, torsionEnergy, improperEnergy);
}

// Return total intramolecular energy of Configuration, storing components in provided variables
double EnergyModule::intraMolecularEnergy(ProcessPool &procPool, const Configuration *cfg, const PotentialMap &potentialMap,
                                          double &bondEnergy, double &angleEnergy, double &torsionEnergy,
                                          double &improperEnergy)
{
    /*
     * Calculate the total intramolecular energy of the system, arising from Bond, Angle, and Torsion
     * terms in all Molecules.
     *
     * This is a parallel routine, with processes operating as a standard world group.
     */

    // Create an EnergyKernel
    EnergyKernel kernel(procPool, cfg, potentialMap);

    bondEnergy = 0;
    angleEnergy = 0;
    torsionEnergy = 0;
    improperEnergy = 0;

    std::vector<Energies> energies(cfg->molecules().size());
    {
    cl::sycl::queue q;
    std::vector<Molecule> mols(cfg->molecules().size());
    std::transform(cfg->molecules().begin(), cfg->molecules().end(), mols.begin(), [](auto mol){return *mol;});

    cl::sycl::buffer<Molecule> buff_molecule(mols.data(), mols.size());
    cl::sycl::buffer<Energies> buff_energies(energies.data(), energies.size());


    q.submit([&](cl::sycl::handler &cgh){
      auto access_mol = buff_molecule.get_access<cl::sycl::access::mode::read>(cgh);
      auto access_eng = buff_energies.get_access<cl::sycl::access::mode::write>(cgh);
      auto unaryOp = [access_mol, access_eng, kernel](cl::sycl::id<1> tid) {
	auto &localEnergies = access_eng[tid];
	auto mol = access_mol[tid];

	// Loop over Bond
	localEnergies.bondEnergy +=
	  std::accumulate(mol.species()->bonds().cbegin(), mol.species()->bonds().cend(), 0.0,
			  [&mol, &kernel](auto const acc, const auto &t) {
			    return acc + kernel.energy(t, *mol.atom(t.indexI()), *mol.atom(t.indexJ()));
			  });

	// Loop over Angle
	localEnergies.angleEnergy += std::accumulate(
						     mol.species()->angles().cbegin(), mol.species()->angles().cend(), 0.0,
						     [&mol, &kernel](auto const acc, const auto &t) {
						       return acc + kernel.energy(t, *mol.atom(t.indexI()), *mol.atom(t.indexJ()), *mol.atom(t.indexK()));
						     });

	// Loop over Torsions
	localEnergies.torsionEnergy +=
	  std::accumulate(mol.species()->torsions().cbegin(), mol.species()->torsions().cend(), 0.0,
			  [&mol, &kernel](auto const acc, const auto &t) {
			    return acc + kernel.energy(t, *mol.atom(t.indexI()), *mol.atom(t.indexJ()),
						       *mol.atom(t.indexK()), *mol.atom(t.indexL()));
			  });

	localEnergies.improperEnergy +=
	  std::accumulate(mol.species()->impropers().cbegin(), mol.species()->impropers().cend(), 0.0,
			  [&mol, &kernel](auto const acc, const auto &imp) {
			    return acc + kernel.energy(imp, *mol.atom(imp.indexI()), *mol.atom(imp.indexJ()),
						       *mol.atom(imp.indexK()), *mol.atom(imp.indexL()));
			  });

      };
      cl::sycl::range<1> work_items(mols.size());

      cgh.parallel_for<class energy_mol>(work_items, unaryOp);
    });
    }

    auto binaryOp = [](const Energies a, const Energies b) {
      Energies result{0,0,0,0};
      result.bondEnergy = a.bondEnergy + b.bondEnergy;
      result.angleEnergy = a.angleEnergy + b.angleEnergy;
      result.torsionEnergy = a.torsionEnergy + b.torsionEnergy;
      result.improperEnergy = a.improperEnergy + b.improperEnergy;
      return result;
    };

    Energies eng{0, 0, 0, 0};
    eng = std::accumulate(energies.begin(), energies.end(), eng, binaryOp);

    bondEnergy = eng.bondEnergy;
    angleEnergy = eng.angleEnergy;
    improperEnergy = eng.improperEnergy;
    torsionEnergy = eng.torsionEnergy;
    double totalIntra = bondEnergy + angleEnergy + torsionEnergy + improperEnergy;

    Messenger::printVerbose("Intramolecular Energy (Local) is {:15.9e} kJ/mol ({:15.9e} bond + {:15.9e} angle + {:15.9e} "
                            "torsion + {:15.9e} improper)\n",
                            totalIntra, bondEnergy, angleEnergy, torsionEnergy, improperEnergy);

    Messenger::printVerbose("Intramolecular Energy (World) is {:15.9e} kJ/mol ({:15.9e} bond + {:15.9e} angle + {:15.9e} "
                            "torsion + {:15.9e} improper)\n",
                            totalIntra, bondEnergy, angleEnergy, torsionEnergy, improperEnergy);

    return totalIntra;
}

// Return total intramolecular energy of Species
double EnergyModule::intraMolecularEnergy(ProcessPool &procPool, const Species *sp)
{
    auto energy = 0.0;

    // Loop over bonds
    for (const auto &bond : sp->bonds())
        energy += EnergyKernel::energy(bond);

    // Loop over angles
    for (const auto &angle : sp->angles())
        energy += EnergyKernel::energy(angle);

    // Loop over torsions
    for (const auto &torsion : sp->torsions())
        energy += EnergyKernel::energy(torsion);

    // Loop over impropers
    for (const auto &improper : sp->impropers())
        energy += EnergyKernel::energy(improper);

    return energy;
}

// Return total energy (interatomic and intramolecular) of Configuration
double EnergyModule::totalEnergy(ProcessPool &procPool, const Configuration *cfg, const PotentialMap &potentialMap)
{
    return (interAtomicEnergy(procPool, cfg, potentialMap) + intraMolecularEnergy(procPool, cfg, potentialMap));
}

// Return total energy (interatomic and intramolecular) of Configuration, storing components in provided variables
double EnergyModule::totalEnergy(ProcessPool &procPool, const Configuration *cfg, const PotentialMap &potentialMap,
                                 double &interEnergy, double &bondEnergy, double &angleEnergy, double &torsionEnergy,
                                 double &improperEnergy)
{
    interEnergy = interAtomicEnergy(procPool, cfg, potentialMap);
    intraMolecularEnergy(procPool, cfg, potentialMap, bondEnergy, angleEnergy, torsionEnergy, improperEnergy);

    return interEnergy + bondEnergy + angleEnergy + torsionEnergy + improperEnergy;
}

// Return total energy (interatomic and intramolecular) of Species
double EnergyModule::totalEnergy(ProcessPool &procPool, const Species *sp, const PotentialMap &potentialMap)
{
    return (interAtomicEnergy(procPool, sp, potentialMap) + intraMolecularEnergy(procPool, sp));
}

// Check energy stability of specified Configuration
EnergyModule::EnergyStability EnergyModule::checkStability(GenericList &processingData, const Configuration *cfg)
{
    // First, check if the Configuration is targetted by an EnergyModule
    if (!processingData.valueOr<bool>("IsEnergyModuleTarget", cfg->niceName(), false))
    {
        Messenger::error("Configuration '{}' is not targeted by any EnergyModule, so stability cannot be assessed. "
                         "Check your setup!\n",
                         cfg->name());
        return EnergyModule::NotAssessable;
    }

    // Retrieve the EnergyStable flag from the Configuration's module data
    if (processingData.contains("EnergyStable", cfg->niceName()))
    {
        auto stable = processingData.value<bool>("EnergyStable", cfg->niceName());
        if (!stable)
        {
            Messenger::print("Energy for Configuration '{}' is not yet stable.\n", cfg->name());
            return EnergyModule::EnergyUnstable;
        }
    }
    else
    {
        Messenger::warn("No energy stability information is present for Configuration '{}' (yet) - check your setup.\n",
                        cfg->name());
        return EnergyModule::NotAssessable;
    }

    return EnergyModule::EnergyStable;
}

// Check energy stability of specified Configurations, returning the number that failed
int EnergyModule::nUnstable(GenericList &processingData, const std::vector<Configuration *> &configurations)
{
    auto nFailed = 0;

    for (auto *cfg : configurations)
    {
        // Check the stability of this Configuration
        auto result = checkStability(processingData, cfg);

        if (result == EnergyModule::EnergyStable)
            ++nFailed;
        else if (result == EnergyModule::NotAssessable)
            return EnergyModule::NotAssessable;
    }

    return nFailed;
}
