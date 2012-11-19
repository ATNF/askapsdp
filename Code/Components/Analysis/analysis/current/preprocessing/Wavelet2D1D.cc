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

void atrous2D1DReconstruct(size_t xdim, size_t ydim, size_t zdim, float* input, float* output, duchamp::Param &par)
{

  /// @details This is Lars Floer's <lfloeer@astro.uni-bonn.de>
  /// implementation of the "2D1D reconstruction" algorithm. This uses
  /// the three-dimensional 'a trous' method used in Duchamp to do
  /// wavelet reconstruction, but treats the spatial directions
  /// separately to the spectral direction. Thresholding of the
  /// wavelet coefficients is done, using the same snrrecon parameter
  /// (in the Duchamp Param set) as for the regular Duchamp
  /// reconstruction.

    //Variables to be overwritten by the appropriate parameter
  //    float reconstructionThreshold = 3.0;
  float reconstructionThreshold = par.getAtrousCut();
    uint minXYScale = 0; // minimal spatial scale
    uint maxXYScale = log(min(xdim,ydim))/log(2); // maximal spatial scale
    uint minZScale = 0; // minimal spectral scale
    uint maxZScale = log(zdim)/log(2); // maximal spectral scale

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
    size_t xydim = xdim * ydim;
    size_t size = xydim * zdim;

    // Allocate the three work arrays
    vector < vector < float > > work(3, vector < float > (size));
    vector < bool > isGood(size);

    // Check for bad values and initialize the output to 0.
    for(ulong i = 0; i < size; i++)
    {
        isGood[i] = (input[i] == input[i]);
        output[i] = 0.;
    }

    // Start the iteration loop
    for(;;)
    {
        // (Re)set the spatial scale factor that determines the step sizes
        // in between the wavelet mother function coefficients
        XYScaleFactor = 1;

        // Initialize the first work array to the input data or residual from the previous iteration
        for(ulong i = 0; i < size; i++)
            work[0][i] = isGood[i] ? input[i] - output[i] : 0.;

        for(uint XYScale = 0; XYScale < maxXYScale; XYScale++)
        {

            // Convolve the x dimension with the wavelet mother function
            // and approriate step size
            for(size_t i = 0; i < size; i++)
            {
                xPos = i % xdim;
                yPos = (i % xydim) / xdim;
                zPos = i / xydim;
                offset = yPos * xdim + zPos * xydim;
                long filterPos = xPos - XYScaleFactor * motherFunctionHalfSize;
                work[2][i] = 0.;

                for(size_t j = 0; j < motherFunctionSize; j++)
                {
                    work[2][i] += work[readFromXY][offset + reflectIndex(filterPos,xdim) * 1] * waveletMotherFunction[j];
                    filterPos += XYScaleFactor;
                }
            }

            // Convolve the y dimension with the wavelet mother function
            // and appropriate step size
            for(size_t i = 0; i < size; i++)
            {
                xPos = i % xdim;
                yPos = (i % xydim) / xdim;
                zPos = i / xydim;
                offset = xPos + zPos * xydim;
                long filterPos = yPos - XYScaleFactor * motherFunctionHalfSize;
                work[writeToXY][i] = 0.;

                for(size_t j = 0; j < motherFunctionSize; j++)
                {
                    work[writeToXY][i] += work[2][offset + reflectIndex(filterPos,ydim) * xdim] * waveletMotherFunction[j];
                    filterPos += XYScaleFactor;
                }
            }

            // Calculate the spatial wavelet coefficients
            for(size_t i = 0; i < size; i++)
                work[writeToXY][i] -= work[readFromXY][i];

            // Exchange the work array access indices and
            // set the access indices for the spectral loop
            readFromXY = (readFromXY + 1) % 2;
            writeToXY = (writeToXY + 1) % 2;
            readFromZ = writeToXY;
            writeToZ = 2;

            // Set the spectral scale factor and work array access indices
            ZScaleFactor = 1;

            for(uint ZScale = 0; ZScale < maxZScale; ZScale++)
            {

                // Convolve the z dimension of the spatial wavelet coefficients
                // with the wavelet mother function and appropriate step size
                for(size_t i = 0; i < size; i++)
                {
                    xPos = i % xdim;
                    yPos = (i % xydim) / xdim;
                    zPos = i / xydim;
                    offset = xPos + yPos * xdim;
                    long filterPos = zPos - ZScaleFactor * motherFunctionHalfSize;
                    work[writeToZ][i] = 0.;

                    for(size_t j = 0; j < motherFunctionSize; j++)
                    {
                        work[writeToZ][i] += work[readFromZ][offset + reflectIndex(filterPos,zdim) * xydim] * waveletMotherFunction[j];
                        filterPos += ZScaleFactor;
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
                if((XYScale >= minXYScale) && (ZScale >= minZScale))
                {
                    // Calculate the spectral wavelet coefficients
                    for(size_t i = 0; i < size; i++)
                        work[writeToZ][i] -= work[readFromZ][i];

                    // // Calculate statistics for the given wavelet coefficients
                    // // Could be replaced with robust or position dependent statistics
                    // double std = 0;
                    // for(size_t i = 0; i < size; i++)
                    //     std += work[writeToZ][i] * work[writeToZ][i];
                    // std = sqrt(std / (size + 1));

		    float middle,spread,threshold;
		    if(par.getFlagRobustStats()){
		      findMedianStats<float>(&(work[writeToZ][0]),size,middle,spread);
		      spread=Statistics::madfmToSigma(spread);
		    }
		    else findNormalStats<float>(&(work[writeToZ][0]),size,middle,spread);
		    threshold = middle + reconstructionThreshold * spread;

                    // Threshold coefficients
                    for(size_t i = 0; i < size; i++)
		      //                        if(fabs(work[writeToZ][i]) > reconstructionThreshold * std)
                        if(fabs(work[writeToZ][i]) > threshold)
                            output[i] += work[writeToZ][i];
                }

                // Increase spectral scale factor
                ZScaleFactor *= 2;
            }

            // Increase spatial scale factor
            XYScaleFactor *= 2;
        }

        // Enforce positivity on the (intermediate) solution
        // Greatly improves the reconstruction quality
        for(size_t i = 0; i < size; i++)
            if(output[i] < 0)
                output[i] = 0;

        // Increase the iteration counter and check for iteration break
        iteration++;
        if(iteration > 0)
            break;
    }
}
