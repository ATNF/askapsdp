/// @file
///
/// Simple class to manage metadata from image that generated a Selavy catalogue
///
/// @copyright (c) 2010 CSIRO
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
#include <askap_simulations.h>

#include <simulationutilities/SelavyImage.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <simulationutilities/ContinuumSelavy.h>

#include <duchamp/FitsIO/Beam.hh>
#include <Common/ParameterSet.h>

#include <casa/aipstype.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/ImageOpener.h>
#include <coordinates/Coordinates/Coordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>

#include <string>
#include <vector>

ASKAP_LOGGER(logger, ".selavyimage");

namespace askap {

    namespace simulations {

      SelavyImage::SelavyImage()
      {
	this->itsBeam = duchamp::Beam(1.,1.,0.);
	this->itsFilename = "";
      }

      SelavyImage::SelavyImage(const SelavyImage& other)
      {
	this->operator=(other);
      }

      SelavyImage& SelavyImage::operator= (const SelavyImage& other)
      {
	if(this==&other) return *this;
	this->itsFilename = other.itsFilename;
	this->itsBeam = other.itsBeam;
	this->itsPixelScale = other.itsPixelScale;
	this->itsDirUnits = other.itsDirUnits;
	return *this;
      }

      SelavyImage::SelavyImage(const LOFAR::ParameterSet& parset)
      {
	/// @details Read the image filename from the parset. Also
	/// calls findBeam().
	this->itsFilename="";
	if(parset.isDefined("Selavyimage")){
	  this->itsFilename = parset.getString("Selavyimage");
	}
	// else{
	//   // Do not have SelavyImage defined, so read beam and pixel info separately
	//   if(parset.isDefined("beam"))

	if(this->itsFilename != "") this->findBeam();
      }

      void SelavyImage::findBeam()
      {
	/// @details Find the beam information from the image
	/// provided. Extracts the beam information from the
	/// ImageInfo, and stores it in a duchamp::Beam object (this
	/// allows easy access to the beam area, used by
	/// convertSource()). Also finds the pixel scale, which is the
	/// geometric mean of the increment of the two spatial
	/// directions, and the units of the direction axes. If these
	/// are not the same a error is raised.  If no beam is found,
	/// the beam area is set to 1 (so convertSource() will not do
	/// anything).

	casa::ImageOpener::registerOpenImageFunction(casa::ImageOpener::FITS, casa::FITSImage::openFITSImage);
	casa::ImageOpener::registerOpenImageFunction(casa::ImageOpener::MIRIAD, casa::MIRIADImage::openMIRIADImage);
	const casa::LatticeBase* lattPtr = casa::ImageOpener::openImage(this->itsFilename);
	
	if (lattPtr == 0){
	  ASKAPTHROW(AskapError, "Requested Selavy image \"" << this->itsFilename << "\" does not exist or could not be opened.");
	}
	else {
	  ASKAPLOG_DEBUG_STR(logger, "Opened Selavy image " << this->itsFilename);
	}
	
	const casa::ImageInterface<casa::Float>* imagePtr = dynamic_cast<const casa::ImageInterface<casa::Float>*>(lattPtr);

	casa::Vector<casa::Quantum<casa::Double> > beam = imagePtr->imageInfo().restoringBeam();
	ASKAPLOG_DEBUG_STR(logger, "Read beam from " << this->itsFilename << " of " << beam);
	casa::CoordinateSystem csys = imagePtr->coordinates();
	int dirCoord = csys.findCoordinate(casa::Coordinate::DIRECTION);
	casa::Vector<casa::Double> increment = csys.directionCoordinate(dirCoord).increment();
	casa::Vector<casa::String> dirUnits = csys.directionCoordinate(dirCoord).worldAxisUnits();
	ASKAPASSERT(increment.size()==2);
	this->itsPixelScale = sqrt(fabs(increment[0]*increment[1]));
	ASKAPLOG_DEBUG_STR(logger, "Read direction axis increment of " << increment << " with units " << dirUnits << " and got pixel scale of " << this->itsPixelScale);
	ASKAPASSERT(dirUnits[0] == dirUnits[1]);
	this->itsDirUnits = dirUnits[0];

	if(beam.size()==0)
	  this->itsBeam.setArea(1.);
	else{
	  double bmaj = beam[0].getValue(this->itsDirUnits)/this->itsPixelScale;
	  double bmin = beam[1].getValue(this->itsDirUnits)/this->itsPixelScale;
	  double bpa = beam[2].getValue("deg");
	  this->itsBeam.define(bmaj,bmin,bpa);
	  ASKAPLOG_DEBUG_STR(logger, "Defined Selavy Image beam with maj="<<this->itsBeam.maj()<<", min="<<this->itsBeam.min() << ", pa="<<this->itsBeam.pa() << " and area="<<this->itsBeam.area());
	}

	delete lattPtr;

      }

      void SelavyImage::convertSource(ContinuumSelavy &src)
      {
	/// @details This function scales the flux of the source
	/// provided by src by the area of the beam. This should do
	/// the correct conversion from Jy (as provided by the
	/// catalogue) to Jy/beam. 
	/// @param src The ContinuuSelavy source under
	/// consideration. Provided as a reference so we can change
	/// its flux.

	ASKAPLOG_DEBUG_STR(logger, "Converting selavy source with flux " << src.fluxZero() << " using beam area " << this->itsBeam.area());
	src.setFluxZero( src.fluxZero() * this->itsBeam.area() );
	ASKAPLOG_DEBUG_STR(logger, "Source's flux now " << src.fluxZero());
      }

      std::vector<float> SelavyImage::beam()
      {
	/// @details Writes out the beam information in a format that
	/// can be used by the rest of the FITSfile functions
	/// (ie. everything in units of degrees).
	/// @return An STL vector containing major axis, minor axis and position angle, in degrees.
	std::vector<float> outputbeam(3);
	outputbeam[0]=casa::Quantity(this->itsBeam.maj()*this->itsPixelScale,this->itsDirUnits).getValue("deg");
	outputbeam[1]=casa::Quantity(this->itsBeam.min()*this->itsPixelScale,this->itsDirUnits).getValue("deg");
	outputbeam[2]=this->itsBeam.pa();
	return outputbeam;
      }

    }

}
