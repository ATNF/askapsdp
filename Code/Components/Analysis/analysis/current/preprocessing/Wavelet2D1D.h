/// Wavelet2D1D.h
///
/// Header file for Lars Floer's 2D1D wavelet reconstruction algorithm
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

#ifndef ASKAP_ANALYSIS_ATROUS_2D1D_H_
#define ASKAP_ANALYSIS_ATROUS_2D1D_H_

#include <askap_analysis.h>

#include <duchamp/param.hh>
#include <duchamp/Cubes/cubes.hh>
#include <Common/ParameterSet.h>

namespace askap {

namespace analysis {

/// @brief Reconstruct the array with the a trous algorithm, with
/// different treatment of the spatial & spectral directions.

class Recon2D1D {
    public:
        Recon2D1D();
        Recon2D1D(const LOFAR::ParameterSet &parset);
        virtual ~Recon2D1D() {};

        void setCube(duchamp::Cube *cube);
        void setFlagPositivity(bool f) {itsFlagPositivity = f;};
        void setFlagDuchampStats(bool f) {itsFlagDuchampStats = f;};

    /// @details This is Lars Floer's <lfloeer@astro.uni-bonn.de>
    /// implementation of the "2D1D reconstruction" algorithm. This uses
    /// the three-dimensional 'a trous' method used in Duchamp to do
    /// wavelet reconstruction, but treats the spatial directions
    /// separately to the spectral direction. Thresholding of the
    /// wavelet coefficients is done, using the same snrrecon parameter
    /// (in the Duchamp Param set) as for the regular Duchamp
    /// reconstruction.
        void reconstruct();

    protected:

        duchamp::Cube *itsCube;
        bool itsFlagPositivity;
        bool itsFlagDuchampStats;
        float itsReconThreshold;
        unsigned int itsMinXYScale;
        unsigned int itsMaxXYScale;
        unsigned int itsMinZScale;
        unsigned int itsMaxZScale;
        unsigned int itsNumIterations;
        size_t itsXdim;
        size_t itsYdim;
        size_t itsZdim;
};

}

}


#endif
