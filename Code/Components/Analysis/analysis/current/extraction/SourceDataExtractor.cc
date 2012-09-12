/// @file
///
/// Base class for handling extraction of image data corresponding to a source
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
#include <extraction/SourceDataExtractor.h>
#include <askap_analysis.h>
#include <string>
#include <sourcefitting/RadioSource.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Slicer.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <lattices/Lattices/LatticeBase.h>
#include <Common/ParameterSet.h>

namespace askap {

  namespace analysis {

    SourceDataExtractor::SourceDataExtractor(const LOFAR::ParameterSet& parset)
    {
      this->itsInputCubePtr=0;
      this->itsSource=0;
      this->itsInputCube = parset.getString("spectralCube","");
    }

    SourceDataExtractor::~SourceDataExtractor()
    {
      this->itsInputCubePtr=0;
      //      if(this->itsInputCubePtr) delete this->itsInputCubePtr;
    }

    SourceDataExtractor::SourceDataExtractor(const SourceDataExtractor& other)
    {
      this->operator=(other);
    }

    SourceDataExtractor& SourceDataExtractor::operator=(const SourceDataExtractor& other)
    {
      if(this == &other) return *this;
      this->itsSource = other.itsSource;
      this->itsSlicer = other.itsSlicer;
      this->itsInputCube = other.itsInputCube;
      this->itsInputCubePtr = other.itsInputCubePtr;
      this->itsOutputFilenameBase = other.itsOutputFilenameBase;
      this->itsOutputFilename = other.itsOutputFilename;
      this->itsArray = other.itsArray;
      return *this;
    }
    
    void SourceDataExtractor::openInput()
    {
      if(this->itsInputCubePtr==0){ // if non-zero, we have already opened the cube
	ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
	ImageOpener::registerOpenImageFunction(ImageOpener::MIRIAD, MIRIADImage::openMIRIADImage);
	const LatticeBase* lattPtr = ImageOpener::openImage(this->itsInputCube);
	if (lattPtr == 0)
	  ASKAPTHROW(AskapError, "Requested input cube \"" << this->itsInputCube << "\" does not exist or could not be opened.");
	this->itsInputCubePtr = dynamic_cast< const ImageInterface<Float>* >(lattPtr);
      }
    }

  }

}
