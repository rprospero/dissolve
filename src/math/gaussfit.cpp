/*
	*** Gaussian Function Approximation
	*** src/math/gaussfit.cpp
	Copyright T. Youngs 2018

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

#include "base/xydata.h"
#include "base/lineparser.h"
#include "math/gaussfit.h"
#include "math/mc.h"
#include "math/praxis.h"

// Constructor
GaussFit::GaussFit(const XYData& referenceData)
{
	referenceData_ = referenceData;

	alphaReal_ = true;
	nGaussians_ = 0;
}

/*
 * Data
 */

// Generate full approximation from current parameters
void GaussFit::generateApproximation(bool realSpace)
{
	approximateData_.templateFrom(referenceData_);

	// Sum defined Gaussians
	for (int n=0; n<nGaussians_; ++n) addFunction(approximateData_, realSpace, x_.value(n), A_.value(n), fwhm_.value(n));
}

// Add contribution to specified XYData
void GaussFit::addFunction(XYData& data, bool realSpace, double xCentre, double A, double fwhm) const
{
	// Functional form of function to add depends on whether we're fitting Gaussians or FTs of Gaussians
	double x;
	if (realSpace)
	{
		for (int m=0; m<data.nPoints(); ++m)
		{
			x = data.x(m);

			// Approximation to original function
			data.addY(m, gaussian(x, xCentre, A, fwhm));
		}
	}
	else
	{
		for (int m=0; m<data.nPoints(); ++m)
		{
			x = data.x(m);

			// Approximation to original function
			data.addY(m, gaussianFT(x, xCentre, A, fwhm));
		}
	}
}

// Return value of Gaussian at specified x value
double GaussFit::gaussian(double x, double xCentre, double A, double FWHM) const
{
	double c = FWHM / TWOSQRT2LN2;
	double gfac = 0.5 * (sqrt(0.5*PI) / PI) / FWHM;
	gfac /= x;
	return gfac * A * exp(-((x-xCentre)*(x-xCentre))/(2.0*c*c));
}

// Return Fourier transform of Gaussian at specified x value
double GaussFit::gaussianFT(double x, double xCentre, double A, double FWHM) const
{
	double c = FWHM / TWOSQRT2LN2;
	const double xCx = xCentre*x;
	return xCx > 0.0 ? A * exp(-(x*x*c*c)/2.0) * sin(xCx)/(xCx) : A * exp(-(x*x*c*c)/2.0);
}

// Set current parameters
bool GaussFit::set(const Array<double>& x, const Array<double>& A, const Array<double>& fwhm)
{
	// Check sizes of provided Arrays
	int nInputGauss = x.nItems();
	if (A.nItems() != nInputGauss) return Messenger::error("Amplitude array does not match the size of the supplied xCentres array (%i vs %i items).\n", nInputGauss, A.nItems()); 
	if (fwhm.nItems() != nInputGauss) return Messenger::error("FWHM array does not match the size of the supplied xCentres array (%i vs %i items).\n", nInputGauss, fwhm.nItems());

	// Copy data
	nGaussians_ = nInputGauss;
	x_ = x;
	A_ = A;
	fwhm_ = fwhm;

	return true;
}

// Return number of Gaussians in fit
int GaussFit::nGaussians() const
{
	return nGaussians_;
}

// Return current function centres
const Array<double>& GaussFit::x() const
{
	return x_;
}

// Return current amplitudes
const Array<double>& GaussFit::A() const
{
	return A_;
}

// Return current full-width half-maximum values
const Array<double>& GaussFit::fwhm() const
{
	return fwhm_;
}

// Save coefficients to specified file
bool GaussFit::saveCoefficients(const char* filename) const
{
	LineParser parser;
	if (!parser.openOutput(filename)) return false;

	parser.writeLineF("#  x  A  FWHM\n");
	for (int n=0; n<nGaussians_; ++n) parser.writeLineF("%f  %f  %f\n", x_.value(n), A_.value(n), fwhm_.value(n));

	parser.closeFiles();

	return true;
}

// Print coefficients
void GaussFit::printCoefficients() const
{
	Messenger::print("Fitted nGaussians = %i:\n", nGaussians_);
	Messenger::print(" Gauss     A         x         FWHM\n");
	for (int n=0; n<nGaussians_; ++n) Messenger::print("  %4i  =  %f %f %f\n", n, A_.value(n), x_.value(n), fwhm_.value(n));
}

// Save Fourier-transformed Gaussians to individual files
bool GaussFit::saveFTGaussians(const char* filenamePrefix, double xStep) const
{
	double xDelta = (xStep < 0.0 ? referenceData_.x(1) - referenceData_.xFirst() : xStep);
	for (int n=0; n<nGaussians_; ++n)
	{
		LineParser parser;
		if (!parser.openOutput(CharString("%s-%03i.gauss", filenamePrefix, n))) return false;

		double xCentre = x_.value(n);
		double A = A_.value(n);
		double fwhm = fwhm_.value(n);
		if (!parser.writeLineF("#  x=%f  A=%f  fwhm=%f\n", xCentre, A, fwhm)) return false;

		double x = referenceData_.xFirst();
		while (x < referenceData_.xLast())
		{
			parser.writeLineF("%f  %f\n", x, gaussianFT(x, xCentre, A, fwhm));
			x += xDelta;
		}

		parser.closeFiles();
	}

	return true;
}

// Return approximate function
const XYData& GaussFit::approximation() const
{
	return approximateData_;
}

// Calculate and return approximate function in requested space
XYData GaussFit::approximation(bool realSpace, double preFactor, double xMin, double xStep, double xMax, double fwhmFactor) const
{
	XYData ft;
	double x = xMin;
	while (x <= xMax)
	{
		ft.addPoint(x, 0.0);
		x += xStep;
	}

	// Loop over defined Gaussians
	for (int n=0; n<nGaussians_; ++n) addFunction(ft, realSpace, x_.value(n), A_.value(n), fwhm_.value(n)*fwhmFactor);

	ft.arrayY() *= preFactor;

	return ft;
}

/*
 * Fitting
 */

// Update precalculated function data (using A = 1.0)
void GaussFit::updatePrecalculatedFunctions()
{
	for (int n=0; n<nGaussians_; ++n)
	{
		for (int m=0; m<referenceData_.nPoints(); ++m) functions_.ref(n, m) = gaussianFT(referenceData_.x(m), x_[n], 1.0, fwhm_[n]);
	}
}

// Construct suitable representation with minimal Gaussians automatically
double GaussFit::constructReal(double requiredError, int maxGaussians)
{
	// Clear any existing data
	x_.clear();
	A_.clear();
	fwhm_.clear();
	nGaussians_ = 0;
	approximateData_.templateFrom(referenceData_);

	const int regionWidth = 5, regionDelta = regionWidth / 2;
	int lastSign = 0;
	double gradient, trialX, trialA, trialFWHM, lastX;
	XYData referenceDelta;

	// Set starting error
	currentError_ = 100.0;

	// Outer loop
	while ((nGaussians_ < maxGaussians) || (maxGaussians == -1))
	{
		// Calculate the delta function between the reference and current approximate data
		referenceDelta.clear();
		for (int n=0; n<referenceData_.nPoints(); ++n) referenceDelta.addPoint(referenceData_.x(n), referenceData_.y(n) - approximateData_.y(n));

		// Keep track of the number of Gaussians we add this cycle
		int nAdded = 0;
		lastX = -1.0;

		// Go over points in the delta, calculating the gradient as we go, and seeking gradient minima (actually, crossovers between -ve and +ve gradients)
		for (int n=regionDelta; n<referenceData_.nPoints() - regionDelta; ++n)
		{
			// Calculate gradient at this point
			gradient = 0.0;
			for (int m=-regionDelta; m<regionDelta; ++m) gradient += (referenceDelta.y(n+m+1) - referenceDelta.y(n+m)) / (referenceDelta.x(n+m+1) - referenceDelta.x(n+m));

			// If the x value of the current point is less than the last accepted Gaussian in this cycle, don't attempt any function addition
			if (referenceDelta.x(n) < lastX) continue;

			// Check sign of previous gradient vs the current one - do we add a Gaussian at this point?
			if ((lastSign != DissolveMath::sgn(gradient)))
			{
				trialX = referenceDelta.x(n);
				trialA = referenceDelta.y(n);
				trialFWHM = 0.25;

				Messenger::printVerbose("Attempting Gaussian addition for peak/trough located at x = %f\n", trialX);

				// Set up minimiser, minimising test Gaussian only
				PrAxis<GaussFit> gaussMinimiser(*this, &GaussFit::costAnalyticAFX);
				alphaReal_ = true;
				gaussMinimiser.addTarget(trialA);
				gaussMinimiser.addTarget(trialFWHM);
				gaussMinimiser.addTarget(trialX);
				double trialError = gaussMinimiser.minimise(0.01, 0.1);

				// Sanity check fitted parameters before we (potentially) accept the new function
				if (fabs(trialA) < 1.0e-4)
				{
					Messenger::printVerbose("Rejecting new Gaussian x = %f, A = %f, FWHM = %f - amplitude is too small\n", trialX, trialA, fabs(trialFWHM));
				}
				else if (fabs(trialFWHM) < 1.0e-2)
				{
					Messenger::printVerbose("Rejecting new Gaussian x = %f, A = %f, FWHM = %f - FWHM is too small\n", trialX, trialA, fabs(trialFWHM));
				}
				else if ((trialError < currentError_) || (nGaussians_ == 0))
				{
					if (nGaussians_ == 0) Messenger::printVerbose("Accepting first Gaussian at x = %f, A = %f, FWHM = %f - error is %f\n", trialX, trialA, fabs(trialFWHM), trialError);
					else Messenger::printVerbose("Accepting new Gaussian x = %f, A = %f, FWHM = %f - error reduced from %f to %f\n", trialX, trialA, fabs(trialFWHM), currentError_, trialError);
					currentError_ = trialError;

					A_.add(trialA);
					x_.add(trialX);
					fwhm_.add(fabs(trialFWHM));
					++nGaussians_;
					++nAdded;
					lastX = trialX;

					// Add the accepted Gaussian in to the approximate data, and remove it from the reference delta
					double x, y;
					for (int m=0; m<referenceData_.nPoints(); ++m)
					{
						x = referenceData_.x(m);
						y = gaussian(x, trialX, trialA, trialFWHM);
						approximateData_.addY(m, y);
						referenceDelta.addY(m, -y);
					}

					// Check on error / nGaussians
					if (currentError_ <= requiredError) break;
					if (nGaussians_ == maxGaussians) break;
				}
				else Messenger::printVerbose("Rejecting new Gaussian x = %f, A = %f, FWHM = %f - error increased from %f to %f\n", trialX, trialA, fabs(trialFWHM), currentError_, trialError);
			}

			// Store current sign of gradient
			lastSign = DissolveMath::sgn(gradient);

			// Check to see if we have reached the error threshold already
			if (currentError_ <= requiredError) break;
		}

		// Check on error
		if (currentError_ <= requiredError)
		{
			Messenger::printVerbose("Required error threshold (%f) achieved - current error is %f, nGaussians = %i\n", requiredError, currentError_, nGaussians_);
			break;
		}

		// Check on nGaussians
		if (nGaussians_ == maxGaussians)
		{
			Messenger::printVerbose("Maximum number of Gaussians (%i) reached - current error is %f\n", nGaussians_, currentError_);
			break;
		}

		// If we added no Gaussians this cycle, bail out now
		if (nAdded == 0)
		{
			Messenger::printVerbose("No Gaussians added during last cycle, so exiting now.\n");
			break;
		}
	}

	return currentError_;
}

// Construct function representation in reciprocal space, spacing Gaussians out evenly in real space up to rMax
double GaussFit::constructReciprocal(double rMax, int nGaussians, double sigmaQ)
{
	// Clear any existing data
	x_.clear();
	A_.clear();
	fwhm_.clear();
	nGaussians_ = nGaussians;
	approximateData_.templateFrom(referenceData_);

	functions_.initialise(nGaussians_, referenceData_.nPoints());
	double x, gDelta = rMax/nGaussians_;
	for (int n=0; n<nGaussians_; ++n)
	{
		x = (n+1)*gDelta;
		x_.add(x);
		A_.add(0.0);
		fwhm_.add(sigmaQ);
	}

	// Update the tabulated functions
	updatePrecalculatedFunctions();

	// Perform Monte Carlo minimisation on the amplitudes
	MonteCarloMinimiser<GaussFit> gaussMinimiser(*this, &GaussFit::costTabulatedA);
	gaussMinimiser.enableParameterSmoothing(100, 3, 3);
	alphaReal_ = false;

	for (int n=0; n<nGaussians_; ++n)
	{
		alphaIndex_.add(n);
		gaussMinimiser.addTarget(A_[n]);
	}

	// Optimise this set of Gaussians
	currentError_ = gaussMinimiser.minimise(5000, 0.01);

	// Regenerate approximation and calculate percentage error of fit
	generateApproximation(false);
	currentError_ = referenceData_.error(approximateData_);

	// Perform a final grouped refit of the amplitudes
	reFitA(false);

	return currentError_;
}

// Re-fit amplitudes in specified space, starting from current parameters
double GaussFit::reFitA(bool realSpace, int sampleSize, int overlap, int nLoops)
{
	/*
	 * Our strategy for optimising the parameters with respect to the fit between approximate and source data will be as
	 * follows. Since the number of Gaussians may be reasonably large (N=hundreds, with 2N parameters total) attempting a
	 * multidimensional fit of this scope is going to be very slow. Since the Gaussians are only locally-dependent (in x)
	 * on each other, we can reduce the problem to a series of local minimisations of a smaller number of Gaussians at a
	 * time. To prevent discontinuities at the extremes of these ranges, we overlap the fit regions a little, and to get
	 * a good global fit we perform several loops of fitting over the data range (shifting the starting point a little
	 * each time).
	 */

	currentError_ = 1.0e9;

	for (int loop=0; loop < nLoops; ++loop)
	{
		// Index of the Gaussian in the x_, A_, and fwhm_ arrays is given by 'g'
		int g = loop*(sampleSize/nLoops);
		while (g < nGaussians_)
		{
			// Generate the approximate data - we will subtract the Gaussians that we are fitting in the next loop
			generateApproximation(realSpace);

			// Clear the reference array of Gaussian indices
			alphaIndex_.clear();

			// Set up minimiser for the next batch
			MonteCarloMinimiser<GaussFit> gaussMinimiser(*this, &GaussFit::costAnalyticA);
			alphaReal_ = realSpace;

			// Add Gaussian parameters as fitting targets
			for (int n=0; n<sampleSize; ++n)
			{
				gaussMinimiser.addTarget(A_[g]);
// 				gaussMinimiser.addTarget(fwhm_[g]);
				alphaIndex_.add(g);

				// Remove this Gaussian from the approximate data
				addFunction(approximateData_, realSpace, x_[g], -A_[g], fwhm_[g]);

				// Increase Gaussian index - if that was the last one, break now
				++g;
				if (g == nGaussians_) break;
			}

			// Optimise this set of Gaussians
			currentError_ = gaussMinimiser.minimise(100, 0.01);
			printf("G = %i, error = %f\n", g, currentError_);

			// If we are not at the end of the Gaussian array, move the index backwards so the next set overlaps a little with this one
			if (g < nGaussians_) g -= overlap;
		}
	}

	// Calculate the approximate function
	generateApproximation(realSpace);

	return referenceData_.error(approximateData_);
}

/*
 * Cost Function Callbacks
 */

// One-parameter cost function (amplitude) with alpha array containing A values, including current approximate data into sum
double GaussFit::costAnalyticA(const Array<double>& alpha)
{
	double sose = 0.0;
	double multiplier = 1.0;

	double A, fwhm, xCentre;
	int g;

	// Loop over data points, add in our Gaussian contributions, and 
	double x, y, dy;
	for (int i=0; i<approximateData_.nPoints(); ++i)
	{
		// Get approximate data x and y for this point
		x = approximateData_.x(i);
		y = approximateData_.y(i);

		// Add in contributions from our Gaussians
		for (int n=0; n<alpha.nItems(); ++n)
		{
			g = alphaIndex_[n];
			A = alpha.value(n);

			y += alphaReal_ ? gaussian(x, x_[g], A, fwhm_[g]) : gaussianFT(x, x_[g], A, fwhm_[g]);
		}

		dy = referenceData_.y(i) - y;
		sose += dy*dy;
	}

	return sose * multiplier;
}

// Two-parameter cost function (amplitude and FWHM) with alpha array containing A and FWHM values, including current approximate data into sum
double GaussFit::costAnalyticAF(const Array<double>& alpha)
{
	const int nGauss = alpha.nItems() / 2;

	double sose = 0.0;
	double multiplier = 1.0;

	double A, fwhm, xCentre;

	// Loop over data points, add in our Gaussian contributions, and 
	double x, y, dy;
	for (int i=0; i<approximateData_.nPoints(); ++i)
	{
		// Get approximate data x and y for this point
		x = approximateData_.x(i);
		y = approximateData_.y(i);

		// Add in contributions from our Gaussians
		for (int n=0; n<nGauss; ++n)
		{
			xCentre = x_[alphaIndex_[n]];
			A = alpha.value(n*2);
			fwhm = alpha.value(n*2+1);

			// Must check for FWHM approaching zero and penalise accordingly
			if (fabs(fwhm) < 1.0e-5) ++multiplier;

			y += alphaReal_ ? gaussian(x, xCentre, A, fwhm) : gaussianFT(x, xCentre, A, fwhm);
		}

		dy = referenceData_.y(i) - y;
		sose += dy*dy;
	}

	return sose * multiplier;
}

// Two-parameter cost function (amplitude and xCentre) with alpha array containing A and FWHM values, including current approximate data into sum
double GaussFit::costAnalyticAX(const Array<double>& alpha)
{
	const int nGauss = alpha.nItems() / 2;

	double sose = 0.0;
	double multiplier = 1.0;

	double A, fwhm, xCentre;

	// Loop over data points, add in our Gaussian contributions, and 
	double x, y, dy;
	for (int i=0; i<approximateData_.nPoints(); ++i)
	{
		// Get approximate data x and y for this point
		x = approximateData_.x(i);
		y = approximateData_.y(i);

		// Add in contributions from our Gaussians
		for (int n=0; n<nGauss; ++n)
		{
			A = alpha.value(n*2);
			xCentre = alpha.value(n*2+1);
			fwhm = fwhm_[alphaIndex_[n]];

			// Must check for FWHM approaching zero and penalise accordingly
			if (fabs(fwhm) < 1.0e-5) ++multiplier;

			y += alphaReal_ ? gaussian(x, xCentre, A, fwhm) : gaussianFT(x, xCentre, A, fwhm);
		}

		dy = referenceData_.y(i) - y;
		sose += dy*dy;
	}

	return sose * multiplier;
}

// Three-parameter cost function (amplitude, FWHM, and xCentre) with alpha array containing A and FWHM values, including current approximate data into sum
double GaussFit::costAnalyticAFX(const Array<double>& alpha)
{
	const int nGauss = alpha.nItems() / 2;

	double sose = 0.0;
	double multiplier = 1.0;

	double A, fwhm, xCentre;

	// Loop over data points, add in our Gaussian contributions, and 
	double x, y, dy;
	for (int i=0; i<approximateData_.nPoints(); ++i)
	{
		// Get approximate data x and y for this point
		x = approximateData_.x(i);
		y = approximateData_.y(i);

		// Add in contributions from our Gaussians
		for (int n=0; n<nGauss; ++n)
		{
			A = alpha.value(n*2);
			fwhm = alpha.value(n*2+1);
			xCentre = alpha.value(n*2+2);

			// Must check for FWHM approaching zero and penalise accordingly
			if (fabs(fwhm) < 1.0e-5) ++multiplier;

			y += alphaReal_ ? gaussian(x, xCentre, A, fwhm) : gaussianFT(x, xCentre, A, fwhm);
		}

		dy = referenceData_.y(i) - y;
		sose += dy*dy;
	}

	return sose * multiplier;
}

// // Three-parameter cost function (amplitude, FWHM, and xCentre) in reciprocal space, with alpha array containing A and FWHM values, including current approximate data into sum
// double GaussFit::costAnalyticReciprocal3(const Array<double>& alpha)
// {
// 	int nGauss = alpha.nItems() / 3;
// 
// 	// Copy current approximate data into a temporary array
// 	XYData sum = approximateData_;
// 	
// 	double A, fwhm, xCentre;
// 	for (int n=0; n<nGauss; ++n)
// 	{
// 		A = alpha.value(n*3);
// 		fwhm = alpha.value(n*3+1);
// 		xCentre = alpha.value(n*3+2);
// 
// 		// Must check for FWHM approaching zero
// 		if (fabs(fwhm) < 1.0e-5) return currentError_*100.0;
// 
// 		for (int m=0; m<sum.nPoints(); ++m) sum.addY(m, gaussianFT(sum.x(m), xCentre, A, fwhm));
// 	}
// 
// 	return referenceData_.error(sum);
// }

// One-parameter cost function (amplitude) using pre-calculated function array, including current approximate data in sum
double GaussFit::costTabulatedA(const Array<double>& alpha)
{
	double sose = 0.0;

	// Loop over data points, add in our Gaussian contributions, and 
	double y, dy;
	int nAlpha = alpha.nItems();
	for (int i=0; i<approximateData_.nPoints(); ++i)
	{
		// Get approximate data x and y for this point
		y = approximateData_.y(i);

		// Add in contributions from our Gaussians
		for (int n=0; n<nAlpha; ++n) y += functions_.value(alphaIndex_[n], i) * alpha.value(n);

		dy = referenceData_.y(i) - y;
		sose += dy*dy;
	}

	return sose;
}
