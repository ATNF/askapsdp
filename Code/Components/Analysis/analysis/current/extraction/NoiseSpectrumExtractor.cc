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
#include <extraction/NoiseSpectrumExtractor.h>
#include <askap_analysis.h>
#include <extraction/SourceDataExtractor.h>
#include <extraction/SpectralBoxExtractor.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/RadioSource.h>
#include <analysisutilities/NewArrayPartMath.h>

#include <imageaccess/CasaImageAccess.h>

#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Slicer.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/SubImage.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>

#include <Common/ParameterSet.h>

#include <duchamp/Utils/Statistics.hh>

using namespace askap::analysis::sourcefitting;

ASKAP_LOGGER(logger, ".spectralboxextractor");

namespace askap {

  namespace analysis {

    NoiseSpectrumExtractor::NoiseSpectrumExtractor(const LOFAR::ParameterSet& parset)
    {
      /// @details Initialise the extractor from a LOFAR parset. This
      /// sets the input cube, the box width, the scaling flag, and
      /// the base name for the output spectra files (these will have
      /// _X appended, where X is the ID of the object in question).
      this->itsInputCube = parset.getString("spectralCube","");
      this->itsAreaInBeams = parset.getFloat("noiseArea",50);
      this->itsBoxWidth = parset.getInt16("spectralBoxWidth",defaultSpectralExtractionBoxWidth);
      this->itsOutputFilename = parset.getString("spectralOutputBase","");
      this->itsInputCubePtr = 0;
      this->setBoxWidth();
    }

    NoiseSpectrumExtractor::NoiseSpectrumExtractor(const NoiseSpectrumExtractor& other)
    {
      this->operator=(other);
    }

    NoiseSpectrumExtractor& NoiseSpectrumExtractor::operator=(const NoiseSpectrumExtractor& other)
    {
      if(this == &other) return *this;
      ((SpectralBoxExtractor &) *this) = other;
      this->itsAreaInBeams = other.itsAreaInBeams;
      return *this;
    }

    void NoiseSpectrumExtractor::setBoxWidth()
    {
      
      this->openInput();
      Vector<Quantum<Double> > inputBeam = this->itsInputCubePtr->imageInfo().restoringBeam();
      ASKAPLOG_DEBUG_STR(logger, "Beam for input cube = " << inputBeam);
      if(inputBeam.size()==0) {
	ASKAPLOG_WARN_STR(logger, "Input image \""<<this->itsInputCube<<"\" has no beam information. Using box width value from parset of " << this->itsBoxWidth << "pix");
      }
      else{
	casa::CoordinateSystem coo = this->itsInputCubePtr->coordinates();
	casa::DirectionCoordinate dirCoo = coo.directionCoordinate(coo.findCoordinate(casa::Coordinate::DIRECTION));
	double fwhmMajPix = inputBeam[0].getValue(dirCoo.worldAxisUnits()[0]) / dirCoo.increment()[0];
	double fwhmMinPix = inputBeam[1].getValue(dirCoo.worldAxisUnits()[1]) / dirCoo.increment()[1];
	double beamAreaInPix = M_PI * fwhmMajPix * fwhmMinPix;
	
	this->itsBoxWidth = int(ceil(sqrt(this->itsAreaInBeams*beamAreaInPix)));

      }
      
    }


    void NoiseSpectrumExtractor::extract()
    {
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

      this->openInput();

      ASKAPLOG_INFO_STR(logger, "Extracting noise spectrum from " << this->itsInputCube << " surrounding source ID " << this->itsSource.getID());

      const SubImage<Float> *sub = new SubImage<Float>(*this->itsInputCubePtr, this->itsSlicer);
      casa::Array<Float> subarray=sub->get();

      this->itsArray = partialMadfms(subarray, IPosition(2,0,1)) / Statistics::correctionFactor;
      

      // ASKAPLOG_DEBUG_STR(logger,"Finished calculating array, here it is: " << this->itsArray);

      delete sub;

    }


  }

}
