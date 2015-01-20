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
#ifndef ASKAP_SIMS_SIMUTIL_H_
#define ASKAP_SIMS_SIMUTIL_H_

#include <simulationutilities/FluxGenerator.h>
#include <modelcomponents/Disc.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <duchamp/Utils/Section.hh>

#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <scimath/Functionals/Gaussian3D.h>
#include <casa/namespace.h>

#include <Common/ParameterSet.h>

#include <vector>
#include <utility>
#include <string>
#include <math.h>

namespace askap {

namespace simulations {

/// @brief Convert between a Gaussian's sigma and its FWHM
inline double FWHMtoSIGMA(double f) {return f / (2. * M_SQRT2 * sqrt(M_LN2));};
inline double SIGMAtoFWHM(double s) {return s * (2. * M_SQRT2 * sqrt(M_LN2));};

/// @brief The string indicating the location of the subimage, used
/// for model chunks.
/// @details Provides a string starting & finishing with
/// '__' and having the starting coordinate of each axis
/// of the given subsection listed and separated by a
/// '_'. So, for example, the subsection
/// [101:200,11:250,1:1,2001:3000] will result in the
/// string '__100_10_0_2000__' (note the difference
/// between the 1-based subsection and the 0-based
/// coordinates).
/// @param A Duchamp subsection object, that has been
/// parsed correctly
const std::string locationString(duchamp::Section &subsection);

/// @brief Create a wcsprm struct from a parset
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
/// @param theRestFreq The value of the restfreq keyword (rest
/// frequency).
/// @param theSection A duchamp::Section object describing the
/// subsection the current image inhabits - necessary for the crpix
/// values.
struct wcsprm *parsetToWCS(const LOFAR::ParameterSet& theParset,
                           const std::vector<unsigned int> &theAxes,
                           const float &theEquinox,
                           const float &theRestFreq,
                           duchamp::Section &theSection);

/// @brief Add a 2D Gaussian component to an array of fluxes.
/// @details Adds the flux of a given 2D Gaussian to the pixel
/// array.  Only look at pixels within a box defined by the
/// distance along the major axis where the flux of the Gaussian
/// falls below the minimum float value. This uses the MAXFLOAT
/// constant from math.h.  Checks are made to make sure that only
/// pixels within the boundary of the array (defined by the axes
/// vector) are added.
///
/// For each pixel, the Gaussian is integrated over the pixel
/// extent to yield the total flux that falls within that pixel.
///
/// @param array The array of pixel flux values to be added to.
/// @param axes The dimensions of the array: axes[0] is the
/// x-dimension and axes[1] is the y-dimension
/// @param gauss The 2-dimensional Gaussian component.
/// @param fluxGen The FluxGenerator object that defines the flux at
/// each channel
/// @param integrate A bool flag that, if true, does the integral over
/// the pixel to find the flux within the pixel. A false value means
/// we just evaluate the Gaussian at the pixel location.
bool addGaussian(std::vector<float>array,
                 std::vector<unsigned int> axes,
                 casa::Gaussian2D<casa::Double> gauss,
                 FluxGenerator &fluxG,
                 bool integrate,
                 bool verbose);

/// @brief Add a 1D Gaussian (in the case of a thin 2D component) to
/// an array of fluxes
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
void add1DGaussian(std::vector<float>array,
                   std::vector<unsigned int> axes,
                   casa::Gaussian2D<casa::Double> gauss,
                   FluxGenerator &fluxGen,
                   bool verbose);

bool addDisc(std::vector<float>array,
             std::vector<unsigned int> axes,
             Disc &disc,
             FluxGenerator &fluxGen,
             bool verbose);

/// @brief Add a single point source to an array of fluxes.
/// @details Adds the flux of a given point source to the
/// appropriate pixel in the given pixel array Checks are
/// made to make sure that only pixels within the boundary
/// of the array (defined by the axes vector) are added.
/// @param array The array of pixel flux values to be added to.
/// @param axes The dimensions of the array: axes[0] is the
/// x-dimension and axes[1] is the y-dimension
/// @param pix The coordinates of the point source
/// @param fluxGen The FluxGenerator object that defines the flux at
/// each channel
bool addPointSource(std::vector<float>array,
                    std::vector<unsigned int> axes,
                    std::vector<double> pix,
                    FluxGenerator &fluxGen,
                    bool verbose);

/// @details Tests whether a given Gaussian component would be added
/// to an array of dimensions given by the axes parameter.
/// @param axes The shape of the flux array
/// @param gauss The 2D Gaussian component to be added
/// @return True if the component would be added to any pixels in the array. False if not.
bool doAddGaussian(std::vector<unsigned int> axes, casa::Gaussian2D<casa::Double> gauss);

/// @details Tests whether a given point-source would be added to an
/// array of dimensions given by the axes parameter.
/// @param axes The shape of the flux array
/// @param pix The location of the point source: an array of at least
/// two values, with pix[0] being the x-coordinate and pix[1] the
/// y-coordinate.
/// @return True if the component would be added to a pixel in the
/// array. False if not.
bool doAddPointSource(std::vector<unsigned int> axes, std::vector<double> pix);

bool doAddDisc(std::vector<unsigned int> axes, Disc &disc);

}

}


#endif
