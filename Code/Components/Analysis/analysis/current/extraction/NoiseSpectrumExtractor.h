/// @file NoiseSpectrumExtractor.h
///
/// @copyright (c) 2014 CSIRO
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

#ifndef ASKAP_ANALYSIS_EXTRACTION_NOISESPECTRUM_H_
#define ASKAP_ANALYSIS_EXTRACTION_NOISESPECTRUM_H_

#include <askap_analysis.h>
#include <extraction/SourceDataExtractor.h>
#include <extraction/SpectralBoxExtractor.h>
#include <Common/ParameterSet.h>

namespace askap {

namespace analysis {

class NoiseSpectrumExtractor : public SpectralBoxExtractor {
    public:
        NoiseSpectrumExtractor() {};

        /// @details Initialise the extractor from a LOFAR parset. This
        /// sets the input cube, the box width, the scaling flag, and
        /// the base name for the output spectra files (these will have
        /// _X appended, where X is the ID of the object in question).
        NoiseSpectrumExtractor(const LOFAR::ParameterSet& parset);
        virtual ~NoiseSpectrumExtractor() {};
        NoiseSpectrumExtractor(const NoiseSpectrumExtractor& other);
        NoiseSpectrumExtractor& operator=(const NoiseSpectrumExtractor& other);

        void setBoxWidth(int w) {itsBoxWidth = w;};
        void setBoxWidth();
        void setBoxAreaInBeams(float a) {itsAreaInBeams = a; setBoxWidth();};
        float boxArea() {return itsAreaInBeams;};
        bool robustFlag() {return itsRobustFlag;};

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
        float itsAreaInBeams;
        bool itsRobustFlag;

};


}

}


#endif
