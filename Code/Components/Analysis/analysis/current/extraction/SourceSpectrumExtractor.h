/// @file
///
/// XXX Notes on program XXX
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
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#ifndef ASKAP_ANALYSIS_EXTRACTION_SOURCESPECTRUM_H_
#define ASKAP_ANALYSIS_EXTRACTION_SOURCESPECTRUM_H_
#include <askap_analysis.h>
#include <extraction/SourceDataExtractor.h>
#include <extraction/SpectralBoxExtractor.h>
#include <Common/ParameterSet.h>
#include <vector>

namespace askap {

namespace analysis {

/// @brief Class to handle the extraction of a spectrum for a given
/// RadioSource.

/// @details This class is aimed primarily at solving the problem
/// of extracting the integrated spectrum from a cube for a
/// previously-detected object, usually a continuum source. One
/// example would be extracting the spectra in all Stokes
/// parameters of a continuum source detected in Stokes I (for
/// instance, in an MFS image).
///
/// The spectrum is extracted by summing over an NxN box, centred
/// on the peak pixel of the RadioSource. The summed flux can be
/// optionally scaled by the beam size to give the flux of an
/// unresolved object.

class SourceSpectrumExtractor : public SpectralBoxExtractor {
    public:
        SourceSpectrumExtractor() {};

        /// @details Initialise the extractor from a LOFAR parset. This
        /// sets the input cube, the box width, the scaling flag, the
        /// base name for the output spectra files (these will have _X
        /// appended, where X is the ID of the object in question), and
        /// the set of polarisation products to extract.
        SourceSpectrumExtractor(const LOFAR::ParameterSet& parset);
        virtual ~SourceSpectrumExtractor() {};
        SourceSpectrumExtractor(const SourceSpectrumExtractor& other);
        SourceSpectrumExtractor& operator=(const SourceSpectrumExtractor& other);

        void setBoxWidth(int w) {itsBoxWidth = w;};
        bool doScale() {return itsFlagDoScale;};
        void setFlagDoScale(bool b) {itsFlagDoScale = b;};

        /// @details This sets the scale factor used to correct the peak
        /// flux of an unresolved source to a total flux. The beam
        /// information is read from the input image, and the beam
        /// weighting is integrated over the same size box as will be
        /// used to extract the spectrum.
        ///
        /// If the input image has no beam information, or if the flag
        /// itsFlagDoScale=false, then the scale factor is set to 1.
        void setBeamScale();

        /// @details The main function that extracts the spectrum from
        /// the desired input. The input cube is opened for reading by
        /// the SourceDataExtractor::openInput() function. A box of
        /// required width is centred on the peak pixel of the
        /// RadioSource, extending over the full spectral range of the
        /// input cube. The box will be truncated at the spatial edges
        /// if necessary. The output spectrum is determined one channel
        /// at a time, summing all pixels within the box and scaling by
        /// the beam if so required. The output spectrum is stored in
        /// itsArray, ready for later access or export.
        void extract();

    protected:
        bool itsFlagDoScale;
        std::vector<float> itsBeamScaleFactor;
        bool itsFlagUseDetection;
        std::string itsBeamLog;

};


}

}


#endif
