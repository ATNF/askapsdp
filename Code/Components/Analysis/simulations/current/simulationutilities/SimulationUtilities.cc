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
#include <mathsutils/MathsUtils.h>
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

#include <duchamp/Utils/Section.hh>

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


const std::string locationString(duchamp::Section &subsection)
{

    std::stringstream locationString;
    locationString << "_";
    for (uint i = 0; i < subsection.getStartList().size(); i++) {
        locationString << "_" << subsection.getStart(i);
    }
    locationString << "__";

    return locationString.str();
}

struct wcsprm *parsetToWCS(const LOFAR::ParameterSet& theParset,
                           const std::vector<unsigned int> &theAxes,
                           const float &theEquinox,
                           const float &theRestFreq,
                           duchamp::Section &theSection)
{

    const unsigned int theDim = theAxes.size();

    struct wcsprm *wcs = (struct wcsprm *)calloc(1, sizeof(struct wcsprm));
    wcs->flag = -1;
    int status = wcsini(true , theDim, wcs);
    ASKAPCHECK(status == 0, "WCSINI returned non-zero result - " << status <<
               " = " << wcs_errmsg[status]);
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
    if (theRestFreq > 0.) wcs->restfrq = theRestFreq;
    else wcs->restfrq = 0.;
    wcs->restwav = 0.;
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
    float majorSigma = FWHMtoSIGMA(gauss.majorAxis());
    float zeroPoint = majorSigma * sqrt(-2.*log(1. / (MAXFLOAT * gauss.height())));
    int xmin = std::max(lround(gauss.xCenter() - zeroPoint), 0L);
    int xmax = std::min(lround(gauss.xCenter() + zeroPoint), long(axes[0] - 1));
    int ymin = std::max(lround(gauss.yCenter() - zeroPoint), 0L);
    int ymax = std::min(lround(gauss.yCenter() + zeroPoint), long(axes[1] - 1));
    // ASKAPLOG_DEBUG_STR(logger, axes[0] << " " << axes[1] << " " << gauss << " " << gauss.height() << " " << majorSigma << " " << zeroPoint << " " << xmin << " " << xmax << " " << ymin << " " << ymax);
    return ((xmax >= xmin) && (ymax >= ymin));

}

bool doAddPointSource(std::vector<unsigned int> axes, std::vector<double> pix)
{

    int xpix = lround(pix[0]);
    int ypix = lround(pix[1]);

    return (xpix >= 0 && xpix < int(axes[0]) && ypix >= 0 && ypix < int(axes[1]));
}

bool doAddDisc(std::vector<unsigned int> axes, Disc &disc)
{
    // ranges of pixels that will have flux added to them
    int xmin = std::max(disc.xmin(), 0);
    int xmax = std::min(disc.xmax(), int(axes[0] - 1));
    int ymin = std::max(disc.ymin(), 0);
    int ymax = std::min(disc.ymax(), int(axes[1] - 1));

    return (xmax >= xmin) && (ymax >= ymin);

}



bool addGaussian(std::vector<float>array, std::vector<unsigned int> axes, casa::Gaussian2D<casa::Double> gauss, FluxGenerator &fluxGen, bool integrate, bool verbose)
{

    float majorSigma = FWHMtoSIGMA(gauss.majorAxis()) ;
    float zeroPointMax = majorSigma * sqrt(-2.*log(1. / (MAXFLOAT * gauss.height())));
    float minorSigma = FWHMtoSIGMA(gauss.minorAxis());
    float zeroPointMin = minorSigma * sqrt(-2.*log(1. / (MAXFLOAT * gauss.height())));
    // Assume a round-number in pixel location is the *centre* of the
    // pixel, so we round the floating-point pixel location to get the
    // pixel it falls in
    int xmin = std::max(lround(gauss.xCenter() - zeroPointMax), 0L);
    int xmax = std::min(lround(gauss.xCenter() + zeroPointMax), long(axes[0] - 1));
    int ymin = std::max(lround(gauss.yCenter() - zeroPointMax), 0L);
    int ymax = std::min(lround(gauss.yCenter() + zeroPointMax), long(axes[1] - 1));
    if (verbose)
        ASKAPLOG_DEBUG_STR(logger, "(x,y)=(" << gauss.xCenter() << "," << gauss.yCenter() <<
                           "), FWHMmaj=" << gauss.majorAxis() <<
                           ", FWHMmin=" << gauss.minorAxis() <<
                           ", gauss.height()=" << gauss.height() <<
                           ", sig_maj=" << majorSigma <<
                           ", sig_min=" << minorSigma <<
                           ", ZPmax=" << zeroPointMax <<
                           ", ZPmin=" << zeroPointMin <<
                           "   xmin=" << xmin << " xmax=" << xmax <<
                           " ymin=" << ymin << " ymax=" << ymax);

    bool addSource = (xmax >= xmin) && (ymax >= ymin);
    if (addSource) {  // if there are object pixels falling within the image boundaries

        std::stringstream ss;

        if (axes.size() > 0) ss << axes[0];

        for (size_t i = 1; i < axes.size(); i++) ss << "x" << axes[i];

        // Test to see whether this should be treated as a point source
        float minSigma = FWHMtoSIGMA(std::min(gauss.majorAxis(), gauss.minorAxis()));
//        float delta = std::min(0.01,pow(10., floor(log10(minSigma/5.))));
//        float delta = pow(10.,floor(log10(minSigma))-1.);
        float delta = std::min(1. / 32.,
                               pow(10., floor(log10(minSigma / 5.) / log10(2.)) * log10(2.)));

        if (verbose)
            ASKAPLOG_DEBUG_STR(logger, "Adding Gaussian " << gauss <<
                               " with flux=" << gauss.flux() <<
                               " and bounds [" << xmin << ":" << xmax <<
                               "," << ymin << ":" << ymax
                               << "] (zeropoints = " << zeroPointMax << "," << zeroPointMin <<
                               ") (dimensions of array=" << ss.str() <<
                               ")  delta=" << delta << ", minSigma = " << minSigma);

        if (xmax == xmin && ymax == ymin) { // single pixel only - add as point source
            std::vector<double> pix(2);
            pix[0] = gauss.xCenter();
            pix[1] = gauss.yCenter();
            if (verbose) {
                ASKAPLOG_DEBUG_STR(logger, "Single pixel only, so adding as point source.");
            }
            return addPointSource(array, axes, pix, fluxGen, verbose);
        }
//                else if (delta < 1.e-3 && integrate) { // if it is really thin and we're integrating, use the 1D approximation
        else if (zeroPointMin < 1. && integrate) {
            if (verbose) {
                // ASKAPLOG_DEBUG_STR(logger, "Since delta = " << delta << "( 1./" << 1. / delta
                //                    << ")  (minSigma=" << minSigma << ")  we use the 1D Gaussian function");
                ASKAPLOG_DEBUG_STR(logger, "Since zeroPointMin=" << zeroPointMin <<
                                   ", we use the 1D Gaussian function. Have delta=" <<
                                   delta << ", minSigma=" << minSigma);
            }
            add1DGaussian(array, axes, gauss, fluxGen, verbose);
        } else {
            // In this case, we need to add it as a Gaussian.

            // Loop over all affected pixels and find the overall normalisation for each pixel

            if (integrate && verbose)
                ASKAPLOG_DEBUG_STR(logger, "Integrating over " <<
                                   (xmax - xmin + 1) * (ymax - ymin + 1) <<
                                   " pixels with delta=" << delta <<
                                   " (1./" << 1. / delta <<
                                   ")  (minSigma=" << minSigma << ")");
            int nstep = int(1. / delta);
            float inputGaussFlux = gauss.flux();
            // make it a unit Gaussian.
            // We then scale by the correct flux for each frequency channel.
            gauss.setFlux(1);

            float dx[2], dy[2], du[4], dv[4];
            float mindu, mindv, separation, xpos, ypos;
            float pixelVal;
            float xScaleFactor, yScaleFactor;

            size_t pix = 0;

            for (int x = xmin; x <= xmax; x++) {

                dx[0] = x - 0.5 - gauss.xCenter();
                dx[1] = x + 0.5 - gauss.xCenter();

                for (int y = ymin; y <= ymax; y++) {

                    pixelVal = 0.;

                    // Need to check whether a given pixel is affected by the Gaussian. To
                    // do this, we look at the "maximal" ellipse - given by where the
                    // Gaussian function drops to below the smallest float. If the closest
                    // corner of the pixel to the centre of the Gaussian lies within this
                    // ellipse, or if the Gaussian passes through the pixel, we do the
                    // integration.

                    dy[0] = y - 0.5 - gauss.yCenter();
                    dy[1] = y + 0.5 - gauss.yCenter();

                    for (int i = 0; i < 4; i++) {
                        du[i] = dx[i % 2] * cos(gauss.PA()) + dy[i / 2] * sin(gauss.PA());
                        dv[i] = dy[i / 2] * cos(gauss.PA()) - dx[i % 2] * sin(gauss.PA());
                        // dv[i] = dx[i%2] * cos(gauss.PA()) + dy[i/2] * sin(gauss.PA());
                        // du[i] = dy[i/2] * cos(gauss.PA()) - dx[i%2] * sin(gauss.PA());
                    }

                    mindu = fabs(du[0]);
                    for (int i = 1; i < 4; i++) {
                        if (fabs(du[i]) < mindu) {
                            mindu = fabs(du[i]);
                        }
                    }

                    mindv = fabs(dv[0]);
                    for (int i = 1; i < 4; i++) {
                        if (fabs(dv[i]) < mindv) {
                            mindv = fabs(dv[i]);
                        }
                    }

                    separation = mindv * mindv / (zeroPointMax * zeroPointMax) +
                                 mindu * mindu / (zeroPointMin * zeroPointMin);

                    float xlim1, xlim2, ylim1, ylim2;
                    findEllipseLimits(zeroPointMax, zeroPointMin, gauss.PA(),
                                      xlim1, xlim2, ylim1, ylim2);
//              ASKAPLOG_DEBUG_STR(logger, "separation = " << separation << " (needs to be less than 1, or dx within xlim etc");
//              ASKAPLOG_DEBUG_STR(logger, "xlims,ylims: " << xlim1 << " " << xlim2 << " " << ylim1 << " " << ylim2);
//              ASKAPLOG_DEBUG_STR(logger, "dxs, dys: " << dx[0] << " " << dx[1] << " " << dy[0] << " " << dy[1]);

                    if (separation <= 1. ||
                            ((dx[0] <= xlim1 && dx[1] >= xlim2) &&
                             (dy[0] <= ylim1 && dy[1] >= ylim2))) {
//                                     ((du[0]*du[1] < 0 || du[0]*du[2] < 0 || du[0]*du[3] < 0) && mindv < zeroPointMax) ||
//                                     ((dv[0]*dv[1] < 0 || dv[0]*dv[2] < 0 || dv[0]*dv[3] < 0) && mindu < zeroPointMin)) {
                        // only consider pixels within the maximal ellipse

                        if (integrate) {

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
//                  if(gauss(xpos,ypos)>0.) ASKAPLOG_DEBUG_STR(logger, xpos << " " << ypos << " " << gauss(xpos,ypos));

                                }
                            }

                            pixelVal *= (delta * delta / 9.);
                        } else {
                            pixelVal = gauss(x, y);
                        }
                    }

//              ASKAPLOG_DEBUG_STR(logger, du[0] << " " << du[1] << " " << du[2] << " " << du[3] << "     " << dv[0] << " " << dv[1] <<" " << dv[2] << " " << dv[3]);
//              ASKAPLOG_DEBUG_STR(logger, separation << " " << mindu << " " << mindv << " " << nstep << " " << delta << " " << delta*delta/9. << " " << pixelVal);

                    // For this pixel, loop over all channels and assign the correctly-scaled pixel value.
                    for (size_t istokes = 0; istokes < fluxGen.nStokes(); istokes++) {
                        for (size_t z = 0; z < fluxGen.nChan(); z++) {
                            pix = x + y * axes[0] + z * axes[0] * axes[1] +
                                  istokes * axes[0] * axes[1] * axes[2];
//              ASKAPLOG_DEBUG_STR(logger, "Adding flux of " << pixelVal*fluxGen.getFlux(z,istokes) << " (from pixelval="<<pixelVal<<" and fluxGen(z)="<<fluxGen.getFlux(z,istokes)<<") to (x,y,z)=("<<x<<","<<y<<","<<z<<")");
                            array[pix] += pixelVal * fluxGen.getFlux(z, istokes);
                        }
                    }

                }
            }

            gauss.setFlux(inputGaussFlux);

        }

    }

    return addSource;
}

void add1DGaussian(std::vector<float>array,
                   std::vector<unsigned int> axes,
                   casa::Gaussian2D<casa::Double> gauss,
                   FluxGenerator &fluxGen,
                   bool verbose)
{

    enum DIR {VERTICAL = 0, HORIZONTAL};
    DIR direction = VERTICAL;
    bool specialCase = true;
    double pa = gauss.PA();
    pa = fmod(pa, M_PI);   // in case we get a value of PA outside [0,pi)
    double sinpa = sin(pa);
    double cospa = cos(pa);
    int sign = pa < M_PI / 2. ? -1 : 1;

    if (cospa == 0.) direction = HORIZONTAL;
    else if (sinpa == 0.) direction = VERTICAL;
    else specialCase = false;

    double majorSigma = FWHMtoSIGMA(gauss.majorAxis());
    double zeroPointMax = majorSigma * sqrt(-2.*log(1. / (MAXFLOAT * gauss.height())));
    double length = 0.;
    double increment = 0.;
    double x = gauss.xCenter() - zeroPointMax * sinpa;
    double y = gauss.yCenter() + zeroPointMax * cospa;
    if (verbose) {
        ASKAPLOG_DEBUG_STR(logger, "Adding a 1D Gaussian: majorSigma = " << majorSigma <<
                           ", zpmax = " << zeroPointMax
                           << ", (xcentre,ycentre)=(" << gauss.xCenter() <<
                           "," << gauss.yCenter() << ")" <<
                           ", pa=" << pa << ", sign=" << sign
                           << ", (xstart,ystart)=(" << x << "," << y <<
                           ") and axes=[" << axes[0] << "," << axes[1] << "]");
    }
    int xref = lround(x);
    int yref = lround(y);
    int spatialPixel = xref + axes[0] * yref;
    ASKAPLOG_DEBUG_STR(logger, "add1DGaussian: x=" << x << ", xref=" << xref << ", y=" << y <<
                       ", yref=" << yref << ", axes[0]=" << axes[0] <<
                       ", spatialPixel=" << spatialPixel);
    size_t pix = 0;
    double pixelVal = 0.;
    bool addPixel = true;

    while (length < 2.*zeroPointMax) {

        // is the current pixel in the bounds of the flux array?
        addPixel = (xref >= 0 && xref < int(axes[0])) &&
                   (yref >= 0 && yref < int(axes[1]));

        if (!specialCase) {
            direction = (fabs((yref + 0.5 * sign - y) / cospa) < fabs((xref + 0.5 - x) / sinpa)) ?
                        VERTICAL : HORIZONTAL;
        }

        if (direction == VERTICAL) { // Moving vertically
            increment = std::min(2.*zeroPointMax - length, fabs((yref + sign * 0.5 - y) / cospa));
            ASKAPCHECK(increment > 0.,
                       "Vertical increment negative: increment=" << increment <<
                       ", sign=" << sign << ", yref=" << yref << ", y=" << y <<
                       ", cospa=" << cospa << ", length=" << length <<
                       ", zpmax=" << zeroPointMax << ", pa=" << pa << "=" << pa * 180. / M_PI);
            yref += sign;
        } else if (direction == HORIZONTAL) { // Moving horizontally
            increment = std::min(2.*zeroPointMax - length, fabs((xref + 0.5 - x) / sinpa));
            ASKAPCHECK(increment > 0.,
                       "Horizontal increment negative: increment=" << increment <<
                       ", xref=" << xref << ", x=" << x << ", sinpa=" << sinpa <<
                       ", length=" << length << ", zpmax=" << zeroPointMax <<
                       ", pa=" << pa << "=" << pa * 180. / M_PI);
            xref++;
        }

        if (addPixel) { // only add points if we're in the array boundaries
            pixelVal = 0.5 * (erf((length + increment - zeroPointMax) / (M_SQRT2 * majorSigma)) -
                              erf((length - zeroPointMax) / (M_SQRT2 * majorSigma)));

            for (size_t istokes = 0; istokes < fluxGen.nStokes(); istokes++) {
                for (size_t z = 0; z < fluxGen.nChan(); z++) {
                    pix = spatialPixel + z * axes[0] * axes[1] +
                          istokes * axes[0] * axes[1] * axes[2];
                    array[pix] += pixelVal * fluxGen.getFlux(z, istokes);
                }
            }
        }

        x += increment * sinpa;
//                y -= sign * increment * cospa;
        y -= increment * cospa;
        spatialPixel = xref + axes[0] * yref;
        length += increment;
        ASKAPLOG_DEBUG_STR(logger, "add1DGaussian: x=" << x << ", xref=" << xref <<
                           ", y=" << y << ", yref=" << yref <<
                           ", axes[0]=" << axes[0] << ", spatialPixel=" << spatialPixel <<
                           ", PIXELVAL=" << pixelVal << ", increment=" << increment <<
                           ", direction=" << direction << ", length=" << length);

    }
}

bool addPointSource(std::vector<float>array,
                    std::vector<unsigned int> axes,
                    std::vector<double> pix,
                    FluxGenerator &fluxGen,
                    bool verbose)
{

    int xpix = lround(pix[0]);
    int ypix = lround(pix[1]);

    bool addSource = (xpix >= 0 && xpix < int(axes[0]) && ypix >= 0 && ypix < int(axes[1]));

    if (addSource)  {

        if (verbose) {
            ASKAPLOG_DEBUG_STR(logger, "Adding Point Source with x=" << pix[0] <<
                               " & y=" << pix[1] << " and flux0=" << fluxGen.getFlux(0) <<
                               " to  axes = [" << axes[0] << "," << axes[1] << "]");
        }

        size_t loc = 0;
        for (size_t istokes = 0; istokes < fluxGen.nStokes(); istokes++) {
            for (size_t z = 0 ; z < fluxGen.nChan(); z++) {

                loc = xpix + axes[0] * ypix + z * axes[0] * axes[1] +
                      istokes * axes[0] * axes[1] * axes[2];

                array[loc] += fluxGen.getFlux(z, istokes);

            }
        }

    }

    return addSource;

}

bool addDisc(std::vector<float>array,
             std::vector<unsigned int> axes,
             Disc &disc,
             FluxGenerator &fluxGen,
             bool verbose)
{

    // ranges of pixels that will have flux added to them
    int xmin = std::max(disc.xmin(), 0);
    int xmax = std::min(disc.xmax(), int(axes[0] - 1));
    int ymin = std::max(disc.ymin(), 0);
    int ymax = std::min(disc.ymax(), int(axes[1] - 1));

    bool addSource  = (xmax >= xmin) && (ymax >= ymin);
    if (addSource) {  // if there are object pixels falling within the image boundaries

        if (verbose)
            ASKAPLOG_DEBUG_STR(logger, "Adding Disc " << disc <<  " with x=[" << xmin <<
                               "," << xmax << "] & y=[" << ymin << "," << ymax <<
                               "] and flux0=" << fluxGen.getFlux(0) <<
                               " to  axes = [" << axes[0] << "," << axes[1] << "]");


        for (int y = ymin; y <= ymax; y++) {
            for (int x = xmin; x <= xmax; x++) {


                double discFlux = disc.flux(x, y);

                for (size_t istokes = 0; istokes < fluxGen.nStokes(); istokes++) {
                    for (size_t z = 0 ; z < fluxGen.nChan(); z++) {
                        size_t loc = x + axes[0] * y + z * axes[0] * axes[1] +
                                     istokes * axes[0] * axes[1] * axes[2];
                        array[loc] += discFlux * fluxGen.getFlux(z, istokes);
                    }
                }

            }
        }

    }

    return addSource;

}


}

}
