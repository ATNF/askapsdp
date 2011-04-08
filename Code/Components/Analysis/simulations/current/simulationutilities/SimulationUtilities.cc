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

#include <wcslib/wcs.h>
#include <wcslib/wcsunits.h>
#include <wcslib/wcsfix.h>
#define WCSLIB_GETWCSTAB // define this so that we don't try to redefine wtbarr
// (this is a problem when using wcslib-4.2)

#include <Common/ParameterSet.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
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

      struct wcsprm *parsetToWCS(const LOFAR::ParameterSet& theParset, const std::vector<unsigned int> &theAxes, const float &theEquinox, duchamp::Section &theSection)
      {
	/// @details Defines a world coordinate system from an
	/// input parameter set. This looks for parameters that
	/// define the various FITS header keywords for each
	/// axis (ctype, cunit, crval, cdelt, crpix, crota), as
	/// well as the equinox, then defines a WCSLIB wcsprm
	/// structure and assigns it to either FITSfile::itsWCS
	/// or FITSfile::itsWCSsources depending on the isImage
	/// parameter.
	/// @param theParset The input parset to be examined.
	/// @param theDim The number of axes expected
	/// @param theEquinox The value of the Equinox keyword
	/// @param theSection A duchamp::Section object describing the subsection the current image inhabits - necessary for the crpix values.

	const unsigned int theDim = theAxes.size();

	struct wcsprm *wcs = (struct wcsprm *)calloc(1, sizeof(struct wcsprm));
	wcs->flag = -1;
	wcsini(true , theDim, wcs);
	wcs->flag = 0;
	std::vector<std::string> ctype, cunit;
	std::vector<float> crval, crpix, cdelt, crota;
	ctype = theParset.getStringVector("ctype");

	if (ctype.size() != theDim)
	  ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << theDim <<
		     ", but ctype has " << ctype.size() << " dimensions.");

	cunit = theParset.getStringVector("cunit");

	if (cunit.size() != theDim)
	  ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << theDim <<
		     ", but cunit has " << cunit.size() << " dimensions.");

	crval = theParset.getFloatVector("crval");

	if (crval.size() != theDim)
	  ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << theDim <<
		     ", but crval has " << crval.size() << " dimensions.");

	crpix = theParset.getFloatVector("crpix");

	if (crpix.size() != theDim)
	  ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << theDim <<
		     ", but crpix has " << crpix.size() << " dimensions.");

	cdelt = theParset.getFloatVector("cdelt");

	if (cdelt.size() != theDim)
	  ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << theDim <<
		     ", but cdelt has " << cdelt.size() << " dimensions.");

	crota = theParset.getFloatVector("crota");

	if (crota.size() != theDim)
	  ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << theDim <<
		     ", but crota has " << crota.size() << " dimensions.");

	for (uint i = 0; i < theDim; i++) {
	  wcs->crpix[i] = crpix[i] - theSection.getStart(i) + 1;
	  wcs->cdelt[i] = cdelt[i];
	  wcs->crval[i] = crval[i];
	  wcs->crota[i] = crota[i];
	  strcpy(wcs->cunit[i], cunit[i].c_str());
	  strcpy(wcs->ctype[i], ctype[i].c_str());
	}

	wcs->equinox = theEquinox;
	wcsset(wcs);

	int stat[NWCSFIX];
	int *axes = new int[theDim];

	for (uint i = 0; i < theDim; i++) axes[i] = theAxes[i];
	wcsfix(1, (const int*)axes, wcs, stat);
	wcsset(wcs);

	delete [] axes;

	return wcs;
	// int nwcs = 1;

	// if (isImage) {
	//   if (this->itsWCSAllocated) wcsvfree(&nwcs, &this->itsWCS);

	//   this->itsWCS = (struct wcsprm *)calloc(1, sizeof(struct wcsprm));
	//   this->itsWCSAllocated = true;
	//   this->itsWCS->flag = -1;
	//   wcsini(true, wcs->naxis, this->itsWCS);
	//   wcsfix(1, (const int*)axes, wcs, stat);
	//   wcscopy(true, wcs, this->itsWCS);
	//   wcsset(this->itsWCS);
	// } else {
	//   if (this->itsWCSsourcesAllocated)  wcsvfree(&nwcs, &this->itsWCSsources);

	//   this->itsWCSsources = (struct wcsprm *)calloc(1, sizeof(struct wcsprm));
	//   this->itsWCSsourcesAllocated = true;
	//   this->itsWCSsources->flag = -1;
	//   wcsini(true, wcs->naxis, this->itsWCSsources);
	//   wcsfix(1, (const int*)axes, wcs, stat);
	//   wcscopy(true, wcs, this->itsWCSsources);
	//   wcsset(this->itsWCSsources);
	// }

	// wcsvfree(&nwcs, &wcs);

      }


        bool doAddGaussian(std::vector<unsigned int> axes, casa::Gaussian2D<casa::Double> gauss)
        {
            /// @details Tests whether a given Gaussian component would be added to an array of dimensions given by the axes parameter.
            /// @param axes The shape of the flux array
            /// @param gauss The 2D Gaussian component to be added
            /// @return True if the component would be added to any pixels in the array. False if not.
            float majorSigma = gauss.majorAxis() / SIGMAtoFWHM;
            float zeroPoint = majorSigma * sqrt(-2.*log(1. / (MAXFLOAT * gauss.height())));
            int xmin = std::max(int(gauss.xCenter() - 0.5 - zeroPoint), 0);
            int xmax = std::min(int(gauss.xCenter() + 0.5 + zeroPoint), int(axes[0] - 1));
            int ymin = std::max(int(gauss.yCenter() - 0.5 - zeroPoint), 0);
            int ymax = std::min(int(gauss.yCenter() + 0.5 + zeroPoint), int(axes[1] - 1));
            return ((xmax >= xmin) && (ymax >= ymin));

        }

        bool doAddPointSource(std::vector<unsigned int> axes, double *pix)
        {
            /// @details Tests whether a given point-source would be added to an array of dimensions given by the axes parameter.
            /// @param axes The shape of the flux array
            /// @param pix The location of the point source: an array of at least two values, with pix[0] being the x-coordinate and pix[1] the y-coordinate.
            /// @return True if the component would be added to a pixel in the array. False if not.

            unsigned int xpix = int(pix[0] + 0.5);
            unsigned int ypix = int(pix[1] + 0.5);

            return (xpix >= 0 && xpix < axes[0] && ypix >= 0 && ypix < axes[1]);
        }

        bool addGaussian(float *array, std::vector<unsigned int> axes, casa::Gaussian2D<casa::Double> gauss, FluxGenerator &fluxGen)
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
            /// @param axes The dimensions of the array: axes[0] is the x-dimension and axes[1] is the y-dimension
            /// @param gauss The 2-dimensional Gaussian component.
            /// @param fluxGen The FluxGenerator object that defines the flux at each channel

            float majorSigma = gauss.majorAxis() / SIGMAtoFWHM;
            float zeroPointMax = majorSigma * sqrt(-2.*log(1. / (MAXFLOAT * gauss.height())));
            float minorSigma = gauss.minorAxis() / SIGMAtoFWHM;
            float zeroPointMin = minorSigma * sqrt(-2.*log(1. / (MAXFLOAT * gauss.height())));
            int xmin = std::max(int(gauss.xCenter() - 0.5 - zeroPointMax), 0);
            int xmax = std::min(int(gauss.xCenter() + 0.5 + zeroPointMax), int(axes[0] - 1));
            int ymin = std::max(int(gauss.yCenter() - 0.5 - zeroPointMax), 0);
            int ymax = std::min(int(gauss.yCenter() + 0.5 + zeroPointMax), int(axes[1] - 1));

	    bool addSource = (xmax >= xmin) && (ymax >= ymin);
            if (addSource) {  // if there are object pixels falling within the image boundaries

                std::stringstream ss;

                if (axes.size() > 0) ss << axes[0];

                for (size_t i = 1; i < axes.size(); i++) ss << "x" << axes[i];

                ASKAPLOG_DEBUG_STR(logger, "Adding Gaussian " << gauss << " with bounds [" << xmin << ":" << xmax << "," << ymin << ":" << ymax
                                       << "] (zeropoints = " << zeroPointMax << "," << zeroPointMin << ") (dimensions of array=" << ss.str() << ")");

                // Test to see whether this should be treated as a point source
                float minSigma = (std::min(gauss.majorAxis(), gauss.minorAxis()) / SIGMAtoFWHM);
//        float delta = std::min(0.01,pow(10., floor(log10(minSigma/5.))));
//        float delta = pow(10.,floor(log10(minSigma))-1.);
                float delta = std::min(1. / 32., pow(10., floor(log10(minSigma / 5.) / log10(2.)) * log10(2.)));

                if (delta < 1.e-4) { // if it is really thin, use the 1D approximation
                    ASKAPLOG_DEBUG_STR(logger, "Since delta = " << delta << "( 1./" << 1. / delta
                                           << ")  (minSigma=" << minSigma << ")  we use the 1D Gaussian function");
                    add1DGaussian(array, axes, gauss, fluxGen);
                } else {
                    // In this case, we need to add it as a Gaussian.

                    // Loop over all affected pixels and find the overall normalisation for each pixel

                    ASKAPLOG_DEBUG_STR(logger, "Integrating over " << (xmax - xmin + 1)*(ymax - ymin + 1) << " pixels with delta="
                                           << delta << " (1./" << 1. / delta << ")  (minSigma=" << minSigma << ")");
                    int nstep = int(1. / delta);
                    float inputGaussFlux = gauss.flux();
                    gauss.setFlux(1); // make it a unit Gaussian. We then scale by the correct flux for each frequency channel.

                    float dx[2], dy[2], du[4], dv[4];
                    float mindu, mindv, separation, xpos, ypos;
                    float pixelVal;
                    float xScaleFactor, yScaleFactor;

		    int pix = 0;

                    for (int x = xmin; x <= xmax; x++) {
                        for (int y = ymin; y <= ymax; y++) {

                            pixelVal = 0.;

                            // Need to check whether a given pixel is affected by the Gaussian. To
                            // do this, we look at the "maximal" ellipse - given by where the
                            // Gaussian function drops to below the smallest float. If the closest
                            // corner of the pixel to the centre of the Gaussian lies within this
                            // ellipse, or if the Gaussian passes through the pixel, we do the
                            // integration.
                            dx[0] = x - 0.5 - gauss.xCenter();
                            dx[1] = x + 0.5 - gauss.xCenter();
                            dy[0] = y - 0.5 - gauss.yCenter();
                            dy[1] = y + 0.5 - gauss.yCenter();

                            for (int i = 0; i < 4; i++) {
                                du[i] = dx[i%2] * cos(gauss.PA()) + dy[i/2] * sin(gauss.PA());
                                dv[i] = dy[i/2] * cos(gauss.PA()) - dx[i%2] * sin(gauss.PA());
                            }

                            mindu = fabs(du[0]); for (int i = 1; i < 4; i++) if (fabs(du[i]) < mindu) mindu = fabs(du[i]);

                            mindv = fabs(dv[0]); for (int i = 1; i < 4; i++) if (fabs(dv[i]) < mindv) mindv = fabs(dv[i]);

                            separation = mindv * mindv / (zeroPointMax * zeroPointMax) + mindu * mindu / (zeroPointMin * zeroPointMin);

                            if (separation <= 1. ||
                                    ((du[0]*du[1] < 0 || du[0]*du[2] < 0 || du[0]*du[3] < 0) && mindv < zeroPointMax) ||
                                    ((dv[0]*dv[1] < 0 || dv[0]*dv[2] < 0 || dv[0]*dv[3] < 0) && mindu < zeroPointMin)) { //only do the integrations if it lies within the maximal ellipse

                                xpos = x - 0.5 - delta;

                                for (int dx = 0; dx <= nstep; dx++) {
                                    xpos += delta;
                                    ypos = y - 0.5 - delta;

                                    for (int dy = 0; dy <= nstep; dy++) {
                                        ypos += delta;

                                        // This is integration using Simpson's rule. In each direction, the
                                        // end points get a factor of 1, then odd steps get a factor of 4, and
                                        // even steps 2. The whole sum then gets scaled by delta/3. for each
                                        // dimension.

                                        if (dx == 0 || dx == nstep) xScaleFactor = 1;
                                        else xScaleFactor = (dx % 2 == 1) ? 4. : 2.;

                                        if (dy == 0 || dy == nstep) yScaleFactor = 1;
                                        else yScaleFactor = (dy % 2 == 1) ? 4. : 2.;

                                        pixelVal += gauss(xpos, ypos) * (xScaleFactor * yScaleFactor);

                                    }
                                }

                                pixelVal *= (delta * delta / 9.);
                            }

                            // For this pixel, loop over all channels and assign the correctly-scaled pixel value.
			    for(int istokes=0; istokes<fluxGen.nStokes();istokes++){
			      for (int z = 0; z < fluxGen.nChan(); z++) {
                                pix = x + y * axes[0] + z * axes[0] * axes[1] + istokes*axes[0]*axes[1]*axes[2];
                                array[pix] += pixelVal * fluxGen.getFlux(z,istokes);
			      }
			    }

                        }
                    }

                    gauss.setFlux(inputGaussFlux);

                }

            }

	    return addSource;
        }

        void add1DGaussian(float *array, std::vector<unsigned int> axes, casa::Gaussian2D<casa::Double> gauss, FluxGenerator &fluxGen)
        {

            /// @details This adds a Gaussian to the array by
            /// approximating it as a 1-dimensional Gaussian. This starts
            /// at the end of the Gaussian with lowest X pixel value, and
            /// moves along the length of the line. When a pixel boundary
            /// is crossed, the flux of the 1D Gaussian between that point
            /// and the previous boundary (or the start) is added to the
            /// pixel. The addition is only done if the pixel lies within
            /// the boundaries of the array (given by the axes parameter).
            /// @param array The array of fluxes
            /// @param axes The dimensions of each axis of the flux array
            /// @param gauss The specification of the Gaussian to be
            /// added. This is defined as a 2D Gaussian, as the position
            /// angle is required, but the minor axis is not used.
            /// @param fluxGen The FluxGenerator object that defines the
            /// fluxes at each channel.

            enum DIR {VERTICAL, HORIZONTAL};
            DIR direction = VERTICAL;
            bool specialCase = true;
            double pa = gauss.PA();
            pa = fmod( pa, M_PI ); // in case we get a value of PA outside [0,pi)
            double sinpa = sin(pa);
            double cospa = cos(pa);
            int sign = pa < M_PI / 2. ? -1 : 1;

            if (cospa == 0.) direction = HORIZONTAL;
            else if (sinpa == 0.) direction = VERTICAL;
            else specialCase = false;

            double majorSigma = gauss.majorAxis() / SIGMAtoFWHM;
            double zeroPointMax = majorSigma * sqrt(-2.*log(1. / (MAXFLOAT * gauss.height())));
            double length = 0.;
            double increment = 0.;
            double x = gauss.xCenter() - zeroPointMax * sinpa;;
            double y = gauss.yCenter() + zeroPointMax * cospa;;
            ASKAPLOG_DEBUG_STR(logger, "Adding a 1D Gaussian: majorSigma = " << majorSigma << ", zpmax = " << zeroPointMax
                                   << ", (xstart,ystart)=(" << x << "," << y << ") and axes=[" << axes[0] << "," << axes[1] << "]");
            unsigned int xref = int(x + 0.5);
            unsigned int yref = int(y + 0.5);
            unsigned int spatialPixel = xref + axes[0] * yref;

            int pix = 0;
            double pixelVal = 0.;
            bool addPixel = true;

            while (length < 2.*zeroPointMax) {

                addPixel = (xref >= 0 && xref < axes[0]) && (yref >= 0 && yref < axes[1]); // is the current pixel in the bounds of the flux array?

                if (!specialCase) {
                    direction = (fabs((yref + 0.5 * sign - y) / cospa) < fabs((xref + 0.5 - x) / sinpa)) ? VERTICAL : HORIZONTAL;
                }

                if (direction == VERTICAL) { // Moving vertically
		    increment = std::min(2.*zeroPointMax - length, fabs((yref + sign * 0.5 - y) / cospa ));
		    ASKAPCHECK(increment>0., "Vertical increment negative: increment="<<increment<<", sign="<<sign<<", yref="<<yref<<", y="<<y<<", cospa="<<cospa<<", length="<<length<<", zpmax="<<zeroPointMax<<", pa="<<pa<<"="<<pa*180./M_PI);
                    yref += sign;
                } else if (direction == HORIZONTAL) { // Moving horizontally
		    increment = std::min(2.*zeroPointMax - length, fabs((xref + 0.5 - x) / sinpa));
		    ASKAPCHECK(increment>0., "Horizontal increment negative: increment="<<increment<<", xref="<<xref<<", x="<<x<<", sinpa="<<sinpa<<", length="<<length<<", zpmax="<<zeroPointMax<<", pa="<<pa<<"="<<pa*180./M_PI);
                    xref++;
                }

                if (addPixel) { // only add points if we're in the array boundaries
                    pixelVal = 0.5 * (erf((length + increment - zeroPointMax) / (M_SQRT2 * majorSigma)) - erf((length - zeroPointMax) / (M_SQRT2 * majorSigma)));

		    for(int istokes=0; istokes<fluxGen.nStokes();istokes++){
		      for (int z = 0; z < fluxGen.nChan(); z++) {
                        pix = spatialPixel + z * axes[0] * axes[1] + istokes*axes[0]*axes[1]*axes[2];
                        array[pix] += pixelVal * fluxGen.getFlux(z,istokes);
		      }
		    }
                }

                x += increment * sinpa;
                y += sign * increment * cospa;
                spatialPixel = xref + axes[0] * yref;
                length += increment;

            }
        }

        bool addPointSource(float *array, std::vector<unsigned int> axes, double *pix, FluxGenerator &fluxGen)
        {
            /// @details Adds the flux of a given point source to the
            /// appropriate pixel in the given pixel array Checks are
            /// made to make sure that only pixels within the boundary
            /// of the array (defined by the axes vector) are added.
            /// @param array The array of pixel flux values to be added to.
            /// @param axes The dimensions of the array: axes[0] is the x-dimension and axes[1] is the y-dimension
            /// @param pix The coordinates of the point source
            /// @param fluxGen The FluxGenerator object that defines the flux at each channel

            unsigned int xpix = int(pix[0] + 0.5);
            unsigned int ypix = int(pix[1] + 0.5);

	    bool addSource = (xpix >= 0 && xpix < axes[0] && ypix >= 0 && ypix < axes[1]);

            if(addSource)  {

                ASKAPLOG_DEBUG_STR(logger, "Adding Point Source with x=" << pix[0] << " & y=" << pix[1]
                                       << "  to  axes = [" << axes[0] << "," << axes[1] << "]");

		int loc = 0;
		for(int istokes=0; istokes<fluxGen.nStokes();istokes++){
		  for (int z = 0 ; z < fluxGen.nChan(); z++) {

                    loc = xpix + axes[0] * ypix + z * axes[0] * axes[1] + istokes*axes[0]*axes[1]*axes[2];

                    array[loc] += fluxGen.getFlux(z,istokes);

		  }
		}

            }

	    return addSource;

        }


    }

}
