/// @file
///
/// Class to handle extraction of a summed spectrum corresponding to a source.
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
#ifndef ASKAP_ANALYSIS_EXTRACTION_SPECTRALBOX_H_
#define ASKAP_ANALYSIS_EXTRACTION_SPECTRALBOX_H_
#include <askap_analysis.h>
#include <extraction/SourceDataExtractor.h>
#include <Common/ParameterSet.h>

namespace askap {

  namespace analysis { 

    /// @brief The default box width for spectral extraction
    const int defaultSpectralExtractionBoxWidth = 5;

    /// @brief Class to handle the extraction of some sort of spectrum
    /// corresponding to a given RadioSource.

    /// @details This is a base class that provides the core
    /// functionality to extract a spectrum obtained in some way over
    /// a box of a given size centred on the prescribed object. This
    /// class defines functions that set up the slicer used to extract
    /// the data from the input image, and that write out the
    /// resulting spectrum to an image on disk. The details of the
    /// extraction (what function to use, how the flux is scaled, etc)
    /// is left to derived classes. 

    class SpectralBoxExtractor : public SourceDataExtractor
    {
    public:
      SpectralBoxExtractor(){};
      SpectralBoxExtractor(const LOFAR::ParameterSet& parset);
      virtual ~SpectralBoxExtractor(){};
      SpectralBoxExtractor(const SpectralBoxExtractor& other);
      SpectralBoxExtractor& operator=(const SpectralBoxExtractor& other);
      
      int boxWidth(){return itsBoxWidth;};
      virtual void setBoxWidth(int w){itsBoxWidth=w;};

      /// @brief Set the pointer to the source, and define the output filename based on its ID
      void setSource(RadioSource* src);
      
      void define();

      virtual void extract() = 0;

      void writeImage();

    protected:
      int itsBoxWidth;

    };

  }
}

#endif
