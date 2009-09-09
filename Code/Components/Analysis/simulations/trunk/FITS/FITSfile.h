/// @file
///
/// Provides base class for handling the creation of FITS files
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
#ifndef ASKAP_SIMS_FITS_H_
#define ASKAP_SIMS_FITS_H_

#include <wcslib/wcs.h>

#include <Common/ParameterSet.h>
#include <casa/Quanta/Unit.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <vector>
#include <utility>
#include <string>
#include <math.h>

namespace askap {

    namespace simulations {

        namespace FITS {

            /// @brief A class to create new FITS files
            /// @details This class handles the creation of FITS files, as
            /// well as WCS handling, adding point or Gaussian components, adding
            /// noise, and convolving with a beam. It is driven by
            /// parameterset input.
            class FITSfile {
                public:
                    /// @brief Default constructor
                    FITSfile();

                    /// @brief Default destructor
                    virtual ~FITSfile();

                    /// @brief Constructor, using an input parameter set
                    FITSfile(const LOFAR::ParameterSet& parset);

		    /// @brief Copy constructor
		    FITSfile(const FITSfile& f);

		    /// @brief Copy operator
		    FITSfile& operator=(const FITSfile& f);

                    /// @brief Define the world coordinate system
                    void setWCS(bool isImage, const LOFAR::ParameterSet& parset);

		    struct wcsprm *getWCS(){return itsWCS;};

		    float array(int pos){return itsArray[pos];};
		    float array(int x, int y){return itsArray[x+itsAxes[0]*y];};
		    void setArray(int pos, float val){itsArray[pos]=val;};
		    void setArray(int x, int y, float val){itsArray[x+itsAxes[0]*y]=val;};

                    /// @brief Make a flux array with just noise in it.
                    void makeNoiseArray();

                    /// @brief Add noise to the flux array
                    void addNoise();

                    /// @brief Add sources to the flux array
                    void addSources();

                    /// @brief Convolve the flux array with a beam
                    void convolveWithBeam();

                    /// @brief Save the array to a FITS file
                    void saveFile();

                protected:

                    /// @brief The name of the file to be written to
                    std::string itsFileName;
                    /// @brief The file containing the list of sources
                    std::string itsSourceList;
                    /// @brief The format of the source positions: "deg"=decimal degrees; "dms"= dd:mm:ss
                    std::string itsPosType;
		    /// @brief The minimum value for the minor axis for the sources in the catalogue. Only used when major axis > 0, to prevent infinite axial ratios
		    float itsMinMinorAxis;
		    /// @brief The units of the position angle for the sources in the catalogue: either "rad" or "deg"
		    casa::Unit itsPAunits;
		    /// @brief The flux units for the sources in the catalogue
		    casa::Unit itsSourceFluxUnits;
		    /// @brief The units of the major & minor axes for the sources in the catalogue
		    casa::Unit itsAxisUnits;

                    /// @brief The array of pixel fluxes
                    float *itsArray;
                    /// @brief Has the memory for itsArray been allocated?
                    bool itsArrayAllocated;
                    /// @brief The RMS of the noise distribution
                    float itsNoiseRMS;

                    /// @brief The dimensionality of the image
                    unsigned int itsDim;
                    /// @brief The axis dimensions
                    std::vector<int> itsAxes;
                    /// @brief The number of pixels in the image
                    int itsNumPix;

                    /// @brief Do we have information on the beam size?
                    bool itsHaveBeam;
                    /// @brief The beam specifications: major axis, minor axis, position angle
                    std::vector<float> itsBeamInfo;

		    /// @brief Do the sources have spectral information for a third axis?
		    bool itsHaveSpectralInfo;
		    float itsBaseFreq;

                    /// @brief The EQUINOX keyword
                    float itsEquinox;
                    /// @brief The BUNIT keyword: units of flux
		    casa::Unit itsBunit;

                    /// @brief How to convert source fluxes to the correct units for the image
                    /// @{
                    double itsUnitScl;
                    double itsUnitOff;
                    double itsUnitPwr;
                    /// @}

                    /// @brief The world coordinate information
                    struct wcsprm *itsWCS;

                    /// @brief The world coordinate information that the sources use, if different from itsWCS
                    struct wcsprm *itsWCSsources;
                    /// @brief If the sources have a different WCS defined, and we need to transform to the image WCS.
                    bool itsFlagPrecess;
                    /// @brief Whether to save the source list with new positions
                    bool itsFlagOutputList;
                    /// @brief The file to save the new source list to.
                    std::string itsOutputSourceList;

            };

        }

    }
}

#endif
