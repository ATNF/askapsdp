/// Wavelet2D1D.h
///
/// Lars Floer's 2D1D wavelet reconstruction algorithm
///
/// @copyright (c) 2011 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///

#include <askap_analysis.h>
#include <preprocessing/Wavelet2D1D.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <math.h>
#include <fitsio.h>
#include <vector>
#include <iostream>
#include <algorithm>

#include <duchamp/param.hh>
#include <duchamp/Utils/utils.hh>
#include <duchamp/Utils/Statistics.hh>

using namespace std;

typedef unsigned long ulong;
typedef unsigned int uint;

ASKAP_LOGGER(logger, ".2d1drecon");

// Convenience function for reflective boundary conditions
long inline reflectIndex(long index, size_t &dim)
{
    while((index < 0) || (index >= long(dim)))
    {
        if(index < 0)
            index = -index;

        if(index >= long(dim))
            index = 2 * (long(dim) - 1) - index;
    }

    return index;
}

Recon2D1D::Recon2D1D()
{
    this->itsCube = 0;
    this->itsFlagPositivity = true;
    this->itsFlagDuchampStats = false;
    this->itsReconThreshold = 3.;
    this->itsMinXYScale = 1;
    this->itsMaxXYScale = 0;
    this->itsMinZScale = 1;
    this->itsMaxZScale = 0;
    this->itsNumIterations = 1;
}

Recon2D1D::Recon2D1D(const LOFAR::ParameterSet &parset)
{
    this->itsFlagPositivity = parset.getBool("enforcePositivity",true);
    this->itsFlagDuchampStats = parset.getBool("useDuchampStats",false);
    this->itsReconThreshold = parset.getFloat("snrRecon",3.0);
    this->itsMinXYScale = parset.getUint16("minXYscale",1);
    this->itsMaxXYScale = parset.getUint16("maxXYscale",-1);
    this->itsMinZScale = parset.getUint16("minZscale",1);
    this->itsMaxZScale = parset.getUint16("maxZscale",-1);
    this->itsNumIterations = parset.getUint16("maxIter",1);
}

Recon2D1D::Recon2D1D(const Recon2D1D& other)
{
    this->operator=(other);
}

Recon2D1D& Recon2D1D::operator= (const Recon2D1D& other)
{
    if(this == &other) return *this;
    this->itsCube = other.itsCube;
    this->itsFlagPositivity = other.itsFlagPositivity;
    this->itsFlagDuchampStats = other.itsFlagDuchampStats;
    this->itsReconThreshold = other.itsReconThreshold;
    this->itsMinXYScale = other.itsMinXYScale;
    this->itsMaxXYScale = other.itsMaxXYScale;
    this->itsMinZScale = other.itsMinZScale;
    this->itsMaxZScale = other.itsMaxZScale;
    this->itsNumIterations = other.itsNumIterations;
    this->itsXdim = other.itsXdim;
    this->itsYdim = other.itsYdim;
    this->itsZdim = other.itsZdim;
    return *this;
}

void Recon2D1D::setCube(duchamp::Cube *cube)
{
    this->itsCube = cube;
    this->itsXdim = cube->getDimX();
    this->itsYdim = cube->getDimY();
    this->itsZdim = cube->getDimZ();
  
    // NB: add +1 here since we have moved to assuming minimum scale = 1, not 0 as in Lars' original code.
    uint xyScaleLimit = int(floor(log(std::min(this->itsXdim,this->itsYdim))/M_LN2));
    uint zScaleLimit = int(floor(log(this->itsZdim)/M_LN2));

    if(this->itsMaxXYScale > 0) //parset provided a value
	this->itsMaxXYScale = std::min(xyScaleLimit, this->itsMaxXYScale);
    else this->itsMaxXYScale = xyScaleLimit;
    if(this->itsMinXYScale > xyScaleLimit){
	ASKAPLOG_WARN_STR(logger, "2D1D Recon: Requested minXYScale="<<this->itsMinXYScale<< " exceeds maximum possible ("<<xyScaleLimit<<"). Setting minXYScale="<<xyScaleLimit);
	this->itsMinXYScale=xyScaleLimit;
    }
    if(this->itsMaxZScale > 0) //parset provided a value
	this->itsMaxZScale = std::min(zScaleLimit, this->itsMaxZScale);
    else this->itsMaxZScale = zScaleLimit;
    if(this->itsMinZScale > zScaleLimit){
	ASKAPLOG_WARN_STR(logger, "2D1D Recon: Requested minZScale="<<this->itsMinZScale<< " exceeds maximum possible ("<<zScaleLimit<<"). Setting minZScale="<<zScaleLimit);
	this->itsMinZScale=zScaleLimit;
    }

}


void Recon2D1D::reconstruct()
{

    /// @details This is Lars Floer's <lfloeer@astro.uni-bonn.de>
    /// implementation of the "2D1D reconstruction" algorithm. This uses
    /// the three-dimensional 'a trous' method used in Duchamp to do
    /// wavelet reconstruction, but treats the spatial directions
    /// separately to the spectral direction. Thresholding of the
    /// wavelet coefficients is done, using the same snrrecon parameter
    /// (in the Duchamp Param set) as for the regular Duchamp
    /// reconstruction.

    // Use pointers for ease of access
    float *input = this->itsCube->getArray();
    float *output = this->itsCube->getRecon();

    // Wavelet mother function stuff
    float waveletMotherFunction[5] = {1./16.,4./16.,6./16.,4./16.,1./16.};
    size_t motherFunctionSize = 5;
    size_t motherFunctionHalfSize = motherFunctionSize / 2;

    uint XYScaleFactor, ZScaleFactor;
    ulong xPos, yPos, zPos, offset;

    // Work array access indices
    uint readFromXY = 0;
    uint writeToXY = 1;
    uint readFromZ = 0;
    uint writeToZ = 0;

    uint iteration = 0;

    // Calculate data sizes
    size_t xydim = this->itsXdim * this->itsYdim;
    size_t size = xydim * this->itsZdim;

    // Allocate the three work arrays
    float *work[3];
    float *origwork[3];  // this is used to track the original pointers for easy deletion at the end.
    for(int i=0;i<3;i++){
	work[i] = new float[size];
	origwork[i] = work[i];
    }
    bool *isGood = new bool[size];


    // Check for bad values and initialize the output to 0.
    // Use the isBlank function of the duchamp::Param 
    for(ulong i = 0; i < size; i++)
    {
	isGood[i] = !this->itsCube->pars().isBlank(input[i]);
	output[i] = 0.;
    }

    // Start the iteration loop
    do{
        // (Re)set the spatial scale factor that determines the step sizes
        // in between the wavelet mother function coefficients
        XYScaleFactor = 1;

        // Initialize the first work array to the input data or residual from the previous iteration
        for(ulong i = 0; i < size; i++)
            work[0][i] = isGood[i] ? input[i] - output[i] : 0.;

        for(uint XYScale = this->itsMinXYScale; XYScale <= this->itsMaxXYScale; XYScale++)
        {

	    if (XYScale < this->itsMaxXYScale) {

		// Convolve the x dimension with the wavelet mother function
		// and approriate step size
		for(size_t i = 0; i < size; i++)
		{
		    xPos = i % this->itsXdim;
		    yPos = (i % xydim) / this->itsXdim;
		    zPos = i / xydim;
		    offset = yPos * this->itsXdim + zPos * xydim;
		    long filterPos = xPos - XYScaleFactor * motherFunctionHalfSize;
		    work[2][i] = 0.;

		    if(isGood[i]){
			for(size_t j = 0; j < motherFunctionSize; j++)
			{
			    size_t loc=offset + reflectIndex(filterPos,this->itsXdim) * 1;
			    if(isGood[loc])
				work[2][i] += work[readFromXY][loc] * waveletMotherFunction[j];
			    filterPos += XYScaleFactor;
			}
		    }
		}

		// Convolve the y dimension with the wavelet mother function
		// and appropriate step size
		for(size_t i = 0; i < size; i++)
		{
		    xPos = i % this->itsXdim;
		    yPos = (i % xydim) / this->itsXdim;
		    zPos = i / xydim;
		    offset = xPos + zPos * xydim;
		    long filterPos = yPos - XYScaleFactor * motherFunctionHalfSize;
		    work[writeToXY][i] = 0.;

		    if(isGood[i]){
			for(size_t j = 0; j < motherFunctionSize; j++)
			{
			    size_t loc=offset + reflectIndex(filterPos,this->itsYdim) * this->itsXdim;
			    if(isGood[loc])
				work[writeToXY][i] += work[2][loc] * waveletMotherFunction[j];
			    filterPos += XYScaleFactor;
			}
		    }
		}

		// Exchange the work array access indices 
		readFromXY = (readFromXY + 1) % 2;
		writeToXY = (writeToXY + 1) % 2;

		// Calculate the spatial wavelet coefficients
		for(size_t i = 0; i < size; i++)
		    work[writeToXY][i] -= work[readFromXY][i];

	    }
	    else 
		work[writeToXY] = work[readFromXY];


            // set the access indices for the spectral loop
            readFromZ = writeToXY;
            writeToZ = 2;

            // Set the spectral scale factor and work array access indices
            ZScaleFactor = 1;

            for(uint ZScale = this->itsMinZScale; ZScale <= this->itsMaxZScale; ZScale++)
            {

                // Convolve the z dimension of the spatial wavelet coefficients
                // with the wavelet mother function and appropriate step size
                for(size_t i = 0; i < size; i++)
                {
                    xPos = i % this->itsXdim;
                    yPos = (i % xydim) / this->itsXdim;
                    zPos = i / xydim;
                    offset = xPos + yPos * this->itsXdim;
                    long filterPos = zPos - ZScaleFactor * motherFunctionHalfSize;
                    work[writeToZ][i] = 0.;

		    if(isGood[i]){
			for(size_t j = 0; j < motherFunctionSize; j++)
			{
			    size_t loc=offset + reflectIndex(filterPos,this->itsZdim) * xydim;
			    if(isGood[loc])
				work[writeToZ][i] += work[readFromZ][loc] * waveletMotherFunction[j];
			    filterPos += ZScaleFactor;
			}
		    }
                }

                // Exchange to work array access indices
                if(writeToZ == 2)
                {
                    readFromZ = 2;
                    writeToZ = writeToXY;
                }
                else
                {
                    readFromZ = writeToXY;
                    writeToZ = 2;
                }

                // Only treat coefficients if the desired minimal scale is reached
                if((XYScale >= this->itsMinXYScale) && (ZScale >= this->itsMinZScale))
                {
                    // Calculate the spectral wavelet coefficients
                    for(size_t i = 0; i < size; i++)
                        work[writeToZ][i] -= work[readFromZ][i];

                    // // Calculate statistics for the given wavelet coefficients
                    // // Could be replaced with robust or position dependent statistics
                    double std = 0;
		    size_t goodSize=0;
                    for(size_t i = 0; i < size; i++){
			if(isGood[i]){
			    std += work[writeToZ][i] * work[writeToZ][i];
			    goodSize++;
			}
		    }
                    std = sqrt(std / (goodSize + 1));

		    float middle,spread,threshold;
		    if(this->itsCube->pars().getFlagRobustStats()){
			findMedianStats<float>(work[writeToZ],size,isGood,middle,spread);
			spread=Statistics::madfmToSigma(spread);
		    }
		    else findNormalStats<float>(work[writeToZ],size,isGood,middle,spread);
		    threshold = middle + this->itsReconThreshold * spread;

                    // Threshold coefficients
                    for(size_t i = 0; i < size; i++){
			bool checkFlux;
			if(this->itsFlagDuchampStats)
			    checkFlux = (fabs(work[writeToZ][i]-middle)>this->itsReconThreshold*spread);
			else
			    checkFlux = (fabs(work[writeToZ][i])>this->itsReconThreshold * std);
			if(checkFlux && isGood[i]) output[i] += work[writeToZ][i];
		    }
                }

                // Increase spectral scale factor
                ZScaleFactor *= 2;
            }

            // Increase spatial scale factor
            XYScaleFactor *= 2;
        }

        // Enforce positivity on the (intermediate) solution
        // Greatly improves the reconstruction quality
	if(this->itsFlagPositivity){
	    for(size_t i = 0; i < size; i++)
		if(output[i] < 0 || !isGood[i])
		    output[i] = 0;
	}

        // Increase the iteration counter and check for iteration break
        iteration++;
	
    } while(iteration < this->itsNumIterations);
    
    delete [] isGood;
    for(int i=2;i>=0;i--) delete [] origwork[i];

    this->itsCube->setReconFlag(true);

}
