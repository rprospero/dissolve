/*
	*** Partials Module - Options
	*** src/modules/partials/options.cpp
	Copyright T. Youngs 2012-2017

	This file is part of dUQ.

	dUQ is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	dUQ is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with dUQ.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "modules/partials/partials.h"
#include "main/duq.h"
#include "classes/species.h"
#include "base/lineparser.h"
#include "templates/enumhelpers.h"

// Partial Calculation Method enum
const char* PartialsMethodKeywords[] = { "Auto", "Test", "Simple", "Cells" };

// Convert character string to PartialsMethod
PartialsModule::PartialsMethod PartialsModule::partialsMethod(const char* s)
{
	for (int n=0; n<nPartialsMethods; ++n) if (DUQSys::sameString(s, PartialsMethodKeywords[n])) return (PartialsModule::PartialsMethod) n;
	return PartialsModule::nPartialsMethods;
}

// Return character string for PartialsMethod
const char* PartialsModule::partialsMethod(PartialsModule::PartialsMethod pm)
{
	return PartialsMethodKeywords[pm];
}

// Weighting Type enum
const char* WeightingTypeKeywords[] = { "None", "Neutron" };

// Convert character string to WeightingType
PartialsModule::WeightingType PartialsModule::weightingType(const char* s)
{
	for (int n=0; n<nWeightingTypes; ++n) if (DUQSys::sameString(s, WeightingTypeKeywords[n])) return (PartialsModule::WeightingType) n;
	return PartialsModule::nWeightingTypes;
}

// Return character string for WeightingType
const char* PartialsModule::weightingType(PartialsModule::WeightingType wt)
{
	return WeightingTypeKeywords[wt];
}

// Normalisation Type enum
const char* NormalisationTypeKeywords[] = { "None", "AverageSquared", "SquaredAverage" };

// Convert character string to NormalisationType
PartialsModule::NormalisationType PartialsModule::normalisationType(const char* s)
{
	for (int n=0; n<PartialsModule::nNormalisationTypes; ++n) if (DUQSys::sameString(s, NormalisationTypeKeywords[n])) return (PartialsModule::NormalisationType) n;
	return PartialsModule::nNormalisationTypes;
}

// Return character string for NormalisationType
const char* PartialsModule::normalisationType(PartialsModule::NormalisationType nt)
{
	return NormalisationTypeKeywords[nt];
}

// Bragg Broadening enum
const char* BraggBroadeningKeywords[] = { "None", "Gaussian" };

// Convert character string to BraggBroadening
PartialsModule::BraggBroadening PartialsModule::braggBroadening(const char* s)
{
	for (int n=0; n<nBroadeningTypes; ++n) if (DUQSys::sameString(s, BraggBroadeningKeywords[n])) return (PartialsModule::BraggBroadening) n;
	return PartialsModule::nBroadeningTypes;
}

// Return character string for BraggBroadening
const char* PartialsModule::braggBroadening(PartialsModule::BraggBroadening bt)
{
	return BraggBroadeningKeywords[bt];
}

// Averaging scheme enum
const char* AveragingSchemeKeywords[] = { "Simple", "Exponential" };

// Convert character string to AveragingScheme
PartialsModule::AveragingScheme PartialsModule::averagingScheme(const char* s)
{
	for (int n=0; n<nAveragingSchemes; ++n) if (DUQSys::sameString(s, AveragingSchemeKeywords[n])) return (PartialsModule::AveragingScheme) n;
	return PartialsModule::nAveragingSchemes;
}

// Return character string for AveragingScheme
const char* PartialsModule::averagingScheme(PartialsModule::AveragingScheme as)
{
	return AveragingSchemeKeywords[as];
}

// Setup options for module
void PartialsModule::setupOptions()
{
	// Boolean options must be set as 'bool(false)' or 'bool(true)' rather than just 'false' or 'true' so that the correct overloaded add() function is called
	frequency_ = 5;
	options_.add("AllIntra", bool(false), "Consider all intramolecular pairs in intra partials");
	options_.add("Averaging", 5, "Number of historical partial sets to combine into final partials")->setValidationMin(0);
	options_.add("AveragingScheme", "Exponential", "Weighting scheme to use when averaging partials")->setValidation(PartialsModule::nAveragingSchemes, AveragingSchemeKeywords);
	options_.add("Bragg", bool(false), "Enable calculation of Bragg scattering");
	options_.add("BraggBroadening", "None", "Broadening function to apply to calculated Bragg scattering")->setValidation(PartialsModule::nBroadeningTypes, BraggBroadeningKeywords);
	options_.add("BraggQDepBroadening", 0.0063, "FWHM of Gaussian for Q-dependent Bragg broadening function");
	options_.add("BraggQIndepBroadening", 0.0, "FWHM of Gaussian for Q-independent Bragg broadening function");
	options_.add("BraggQResolution", 0.001, "Binwidth in Q to use when calculating Bragg peaks");
	options_.add("InternalTest", bool(false), "Perform internal check of calculated partials (relative to Test method)");
	options_.add("Method", "Auto", "Calculation method for partial radial distribution functions")->setValidation(PartialsModule::nPartialsMethods, PartialsMethodKeywords);
	options_.add("Normalisation", "None", "Normalisation to apply to total weighted F(Q)")->setValidation(PartialsModule::nNormalisationTypes, NormalisationTypeKeywords);
	options_.add("QDelta", 0.05, "Step size in Q for S(Q) calculation")->setValidationMin(1.0e-5);
	options_.add("QDepBroadening", 0.0, "FWHM of Gaussian for Q-dependent instrument broadening function when calculating S(Q)")->setValidationMin(0.0);
	options_.add("QIndepBroadening", 0.0, "FWHM of Gaussian for Q-independent instrument broadening function when calculating S(Q)")->setValidationMin(0.0);
	options_.add("QMax", -1.0, "Maximum Q for calculated S(Q)")->setValidationMin(-1.0);
	options_.add("QMin", 0.0, "Minimum Q for calculated S(Q)")->setValidationMin(0.0);
	options_.add("Save", bool(false), "Whether to save partials to disk after calculation");
	options_.add("Smoothing", 0, "Specifies the degree of smoothing 'n' to apply to calculated RDFs, where 2n+1 controls the length in the applied Spline smooth")->setValidationRange(0, 100);
	options_.add("StructureFactor", bool(false), "Determines whether S(Q) are to be calculated from F.T. of the g(r)");
	options_.add("Test", bool(false), "Test calculated total and partials against supplied reference data");
	options_.add("TestThreshold", 0.1, "Test threshold (%%error) above which test fails")->setValidationMin(1.0e-5);
	options_.add("Weights", "None", "Weighting scheme to use for calculated partials (None,Neutron)")->setValidation(PartialsModule::nWeightingTypes, WeightingTypeKeywords);
}

// Parse keyword line, returning true (1) on success, false (0) for recognised but failed, and -1 for not recognised
int PartialsModule::parseKeyword(LineParser& parser, DUQ* duq, GenericList& targetList)
{
	if (DUQSys::sameString(parser.argc(0), "BraggBroadening"))
	{
		// Line broadening to apply to Bragg calculation
		PartialsModule::BraggBroadening bb = PartialsModule::braggBroadening(parser.argc(1));
		if (bb == PartialsModule::nBroadeningTypes)
		{
			Messenger::error("Unrecognised broadening type supplied.\n");
			return false;
		}
		GenericListHelper<CharString>::add(targetList, "BraggBroadening", uniqueName()) = PartialsModule::braggBroadening(bb);

		// Get any parameters supplied
		for (int n=2; n<parser.nArgs(); ++n) GenericListHelper<double>::add(targetList, CharString("BraggBroadeningParameter%i", n-1), uniqueName()) = parser.argd(n);
	}
	else if (DUQSys::sameString(parser.argc(0), "Exchangeable"))
	{
		// Define an exchangeable group of atoms
		// Loop over all provided arguments (which are atom type names) and add them to our list
		AtomTypeList exchangeableAtoms;
		for (int n=1; n<parser.nArgs(); ++n)
		{
			AtomType* atomType = duq->findAtomType(parser.argc(n));
			if (!atomType)
			{
				Messenger::error("Unrecognised AtomType '%s' given in Exchangeable keyword.\n", parser.argc(n));
				return false;
			}
			exchangeableAtoms.add(atomType);
		}

		GenericListHelper<AtomTypeList>::add(targetList, "Exchangeable", uniqueName()) = exchangeableAtoms;
	}
	else if (DUQSys::sameString(parser.argc(0), "Isotopologue"))
	{
		// Essentially a shortcut for setting a variable in a target Configuration / Sample
		// Find target Configuration
		Configuration* targetCfg = duq->findConfiguration(parser.argc(1));
		if (!targetCfg)
		{
			Messenger::error("Error defining Isotopologue - no Configuration named '%s' exists.\n", parser.argc(1));
			return false;
		}

		// Raise an error if this Configuration is not targetted by the Module
		if (!isTargetConfiguration(targetCfg)) 
		{
			Messenger::error("Configuration '%s' is not targetted by the Module '%s'.\n", targetCfg->name(), name());
			return false;
		}

		// Find specified Species - must be present in the target Configuration
		Species* sp = duq->findSpecies(parser.argc(2));
		if (!sp)
		{
			Messenger::error("Error defining Isotopologue - no Species named '%s' exists.\n", parser.argc(2));
			return false;
		}

		if (!targetCfg->hasUsedSpecies(sp))
		{
			Messenger::error("Error defining Isotopologue - Species '%s' is not present in Configuration '%s'.\n", sp->name(), targetCfg->name());
			return false;
		}

		// Finally, locate isotopologue definition for species
		Isotopologue* tope = sp->findIsotopologue(parser.argc(3));
		if (!tope)
		{
			Messenger::error("Error defining Isotopologue - no Isotopologue named '%s' exists for Species '%s'.\n", parser.argc(3), sp->name());
			return false;
		}

		// Ready - add a suitable variable to the Configuration
		CharString varName("Isotopologue/%s/%s", sp->name(), tope->name());
		GenericListHelper<double>::add(targetList, varName, uniqueName()) = parser.argd(4);
	}
	else if (DUQSys::sameString(parser.argc(0), "TestReference"))
	{
		Messenger::print("Reading test reference g(r) / G(r) / S(Q) / F(Q) data...\n");

		// Realise an XYData to store the reference data in
		XYData& data = GenericListHelper<XYData>::realise(targetList, CharString("TestReference%s", parser.argc(2)), uniqueName());

		// Fourth and fifth arguments are x and y columns respectively (defaulting to 0,1 if not given)
		int xcol = parser.hasArg(3) ? parser.argi(3)-1 : 0;
		int ycol = parser.hasArg(4) ? parser.argi(4)-1 : 1;

		LineParser fileParser(&duq->worldPool());
		if ((!fileParser.openInput(parser.argc(1))) || (!data.load(fileParser, xcol, ycol))) return false;
	}
	else return -1;

	return true;
}

