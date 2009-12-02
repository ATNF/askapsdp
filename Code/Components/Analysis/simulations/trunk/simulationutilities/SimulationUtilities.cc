/// @file
///
/// Provides utility functions for simulations package
///
/// @copyright (c) 2007 CSIRO
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#include <askap_simulations.h>

#include <simulationutilities/SimulationUtilities.h>
#include <simulationutilities/FluxGenerator.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <scimath/Functionals/Gaussian3D.h>
#include <casa/namespace.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <utility>
#include <string>
#include <stdlib.h>
#include <math.h>

ASKAP_LOGGER(logger, ".simutils");

namespace askap {

    namespace simulations {


        float normalRandomVariable(float mean, float sigma)
        {
            /// @details Simulate a normal random variable from a
            /// distribution with mean given by mean and standard deviation
            /// given by sigma. The variable is simulated via the polar
            /// method.
            /// @param mean The mean of the normal distribution
            /// @param sigma The standard deviation of the normal distribution
            /// @return A random variable.
            float v1, v2, s;

            // simulate a standard normal RV via polar method
            do {
                v1 = 2.*(1.*random()) / (RAND_MAX + 1.0) - 1.;
                v2 = 2.*(1.*random()) / (RAND_MAX + 1.0) - 1.;
                s = v1 * v1 + v2 * v2;
            } while (s > 1);

            float z = sqrt(-2.*log(s) / s) * v1;
            return z*sigma + mean;
        }

        void addGaussian(float *array, duchamp::Section subsection, std::vector<unsigned int> axes, casa::Gaussian2D<casa::Double> gauss, FluxGenerator &fluxGen)
        {
            /// @details Adds the flux of a given 2D Gaussian to the pixel
            /// array.  Only look at pixels within a box defined by the
            /// distance along the major axis where the flux of the Gaussian
            /// falls below the minimum float value. This uses the MAXFLOAT
            /// constant from math.h.  Checks are made to make sure that
            /// only pixels within the boundary of the array (defined by the
            /// axes vector) are added.
            ///
            /// For each pixel, the Gaussian is integrated over the
            /// pixel extent to yield the total flux that falls within
            /// that pixel.
            ///
            /// @param array The array of pixel flux values to be added to.
            /// @param subsection The subsection of the image being used
            /// @param axes The dimensions of the array: axes[0] is the x-dimension and axes[1] is the y-dimension
            /// @param gauss The 2-dimensional Gaussian component.
            /// @param fluxGen The FluxGenerator object that defines the flux at each channel

            float majorSigma = gauss.majorAxis() / (4.*M_LN2);
            float zeroPoint = majorSigma * sqrt(-2.*log(1. / (MAXFLOAT * gauss.height())));
            int xmin = std::max(int(gauss.xCenter() - 0.5 - zeroPoint), 0);
            int xmax = std::min(int(gauss.xCenter() + 0.5 + zeroPoint), int(axes[0]));
            int ymin = std::max(int(gauss.yCenter() - 0.5 - zeroPoint), 0);
            int ymax = std::min(int(gauss.yCenter() + 0.5 + zeroPoint), int(axes[1]));

            ASKAPLOG_DEBUG_STR(logger, "Adding Gaussian... xmin=" << xmin << ", xmax=" << xmax << ", ymin=" << ymin << ", ymax=" << ymax << " ... using subsection " << subsection.getSection() << " with [start:end]=[" << subsection.getStart(0) << ":" << subsection.getEnd(0) << "," << subsection.getStart(1) << ":" << subsection.getEnd(1) << "], axes=["<<axes[0]<<","<<axes[1]<<"] zeropoint=" << zeroPoint << " and gaussian at [" << gauss.xCenter() << "," << gauss.yCenter() << "]");

            if ((xmax >= xmin) && (ymax >= ymin)) {  // if there are object pixels falling within the image boundaries

                ASKAPLOG_DEBUG_STR(logger, "Adding Gaussian " << gauss << " with bounds [" << xmin << ":" << xmax << "," << ymin << ":" << ymax << "] (zeropoint = " << zeroPoint << ") (subsection=" << subsection.getSection() << " --> " << subsection.getStart(0) << "--" << subsection.getEnd(0) << "|" << subsection.getStart(1) << "--" << subsection.getEnd(1) << ")");

                // Test to see whether this should be treated as a point source
                float minSigma = (std::min(gauss.majorAxis(), gauss.minorAxis()) / (2.*sqrt(2.*M_LN2)));
//        float delta = std::min(0.01,pow(10., floor(log10(minSigma/5.))));
//        float delta = pow(10.,floor(log10(minSigma))-1.);
                float delta = std::min(1. / 32., pow(10., floor(log10(minSigma / 5.) / log10(2.)) * log10(2.)));

                if (delta < 1.e-4) { // if it is really small, just make it a point source
                    double *pix = new double[2];
                    pix[0] = gauss.xCenter();
                    pix[1] = gauss.yCenter();
                    ASKAPLOG_DEBUG_STR(logger, "Making this Gaussian a point source since delta = " << delta << " (1./" << 1. / delta << ")  (minSigma=" << minSigma << ")");
                    addPointSource(array, subsection, axes, pix, fluxGen);
                    delete [] pix;
                } else {
                    // In this case, we need to add it as a Gaussian.

                    // Loop over all affected pixels and find the overall normalisation for each pixel

                    ASKAPLOG_DEBUG_STR(logger, "Integrating over " << (xmax - xmin + 1)*(ymax - ymin + 1) << " pixels with delta=" << delta << " (1./" << 1. / delta << ")  (minSigma=" << minSigma << ")");
                    int nstep = int(1. / delta);
                    float inputGaussFlux = gauss.flux();
                    gauss.setFlux(1); // make it a unit Gaussian. We then scale by the correct flux for each frequency channel.

                    for (int x = xmin; x <= xmax; x++) {
                        for (int y = ymin; y <= ymax; y++) {

                            float pixelVal = 0.;
                            float xpos = x - 0.5 - delta;

                            for (int dx = 0; dx <= nstep; dx++) {
                                xpos += delta;
                                float ypos = y - 0.5 - delta;

                                for (int dy = 0; dy <= nstep; dy++) {
                                    ypos += delta;

                                    // This is integration using Simpson's
                                    // rule. In each direction, the end points get
                                    // a factor of 1, then odd steps get a factor
                                    // of 4, and even steps 2. The whole sum then
                                    // gets scaled by delta/3. for each dimension.
                                    float xScaleFactor, yScaleFactor;

                                    if (dx == 0 || dx == nstep) xScaleFactor = 1;
                                    else xScaleFactor = (dx % 2 == 1) ? 4. : 2.;

                                    if (dy == 0 || dy == nstep) yScaleFactor = 1;
                                    else yScaleFactor = (dy % 2 == 1) ? 4. : 2.;

                                    pixelVal += gauss(xpos, ypos) * (xScaleFactor * yScaleFactor);

                                }
                            }

                            pixelVal *= (delta * delta / 9.);

                            // For this pixel, loop over all channels and assign the correctly-scaled pixel value.
                            for (int z = 0; z < fluxGen.nChan(); z++) {
                                int pix = x + y * axes[0] + z * axes[0] * axes[1];
                                float f = fluxGen.getFlux(z);
                                array[pix] += pixelVal * f;
                                //              if(f>0.) ASKAPLOG_DEBUG_STR(logger, "Adding a flux of " << f << " to channel " << z);
                            }

                        }
                    }

                    gauss.setFlux(inputGaussFlux);

                }

            }
        }

        void addPointSource(float *array, duchamp::Section subsection, std::vector<unsigned int> axes, double *pix, FluxGenerator &fluxGen)
        {
            /// @details Adds the flux of a given point source to the
            /// appropriate pixel in the given pixel array Checks are
            /// made to make sure that only pixels within the boundary
            /// of the array (defined by the axes vector) are added.
            /// @param array The array of pixel flux values to be added to.
            /// @param subsection The subsection of the image being used
            /// @param axes The dimensions of the array: axes[0] is the x-dimension and axes[1] is the y-dimension
            /// @param pix The coordinates of the point source
            /// @param fluxGen The FluxGenerator object that defines the flux at each channel


	    if (pix[0] >= 0 && pix[0] <= axes[0] && pix[1] >= 0 && pix[1] <= axes[1]) {

	      ASKAPLOG_DEBUG_STR(logger, "Adding Point Source with x="<<pix[0]<<" & y="<<pix[1]
				 <<"  ...  xmin=" << subsection.getStart(0) << ", xmax=" << subsection.getEnd(0)
				 << ", ymin=" << subsection.getStart(1) << ", ymax=" << subsection.getEnd(1) 
				 <<  ",   Subsection="<<subsection.getSection() << "   axes = ["<<axes[0] << ","<<axes[1]<<"]");

	      for (int z = 0 ; z < fluxGen.nChan(); z++) {

		int loc = int(pix[0]-subsection.getStart(0)) + axes[0] * int(pix[1]-subsection.getStart(1)) + z * axes[0] * axes[1];
		
		array[loc] += fluxGen.getFlux(z);

	      }

            }

        }


    }

}
