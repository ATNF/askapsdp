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
#include <extraction/SpectralBoxExtractor.h>
#include <askap_analysis.h>
#include <extraction/SourceDataExtractor.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/RadioSource.h>

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

using namespace askap::analysis::sourcefitting;

ASKAP_LOGGER(logger, ".spectralboxextractor");

namespace askap {

  namespace analysis {

    SpectralBoxExtractor::SpectralBoxExtractor(const LOFAR::ParameterSet& parset)
    {
      /// @details Initialise the extractor from a LOFAR parset. This
      /// sets the input cube, the box width, the scaling flag, and
      /// the base name for the output spectra files (these will have
      /// _X appended, where X is the ID of the object in question).
      this->itsInputCube = parset.getString("spectralCube","");
      this->itsBoxWidth = parset.getInt16("spectralBoxWidth",defaultSpectralExtractionBoxWidth);
      this->itsFlagDoScale = parset.getBool("scaleSpectraByBeam",true);
      this->itsOutputFilename = parset.getString("spectralOutputBase","");
      this->itsInputCubePtr = 0;
    }

    SpectralBoxExtractor::SpectralBoxExtractor(const SpectralBoxExtractor& other)
    {
      this->operator=(other);
    }

    SpectralBoxExtractor& SpectralBoxExtractor::operator=(const SpectralBoxExtractor& other)
    {
      if(this == &other) return *this;
      ((SourceDataExtractor &) *this) = other;
      this->itsBoxWidth = other.itsBoxWidth;
      this->itsFlagDoScale = other.itsFlagDoScale;
      this->itsBeamScaleFactor = other.itsBeamScaleFactor;
      return *this;
    }

    void SpectralBoxExtractor::setSource(RadioSource &src)
    {
      /// @details Sets the source to be used. Also sets the output
      /// filename correctly with the suffix indicating the object's
      /// ID.  
      /// @param src The RadioSource detection used to centre the
      /// spectrum. The central pixel will be chosen to be the peak
      /// pixel, so this needs to be defined.

      this->itsSource = src;
      // Append the source's ID string to the output filename
      int ID=this->itsSource.getID();
      std::stringstream ss;
      ss << this->itsOutputFilename << "_" << ID;
      this->itsOutputFilename = ss.str();
    }

    void SpectralBoxExtractor::setBeamScale()
    {
      if(!this->itsFlagDoScale) this->itsBeamScaleFactor = 1.;
      else{

	accessors::CasaImageAccess ia;
 	this->openInput();
 	Vector<Quantum<Double> > inputBeam = this->itsInputCubePtr->imageInfo().restoringBeam();
	// 	Vector<Quantum<Double> > inputBeam = ia.beamInfo(this->itsInputCube);;
	ASKAPLOG_DEBUG_STR(logger, "Beam for input cube = " << inputBeam);
	if(inputBeam.size()==0) {
	  ASKAPLOG_WARN_STR(logger, "Input image \""<<this->itsInputCube<<"\" has no beam information. Not scaling spectra by beam");
	  this->itsBeamScaleFactor = 1.;
	}
	else{
	  double costheta = cos(inputBeam[2].getValue("rad"));
	  double sintheta = sin(inputBeam[2].getValue("rad"));
	  
	  //	  casa::CoordinateSystem coo=ia.coordSys(this->itsInputCube);
	  //	  casa::DirectionCoordinate dirCoo = this->itsInputCubePtr->coordinates().directionCoordinate(this->itsInputCubePtr->coordinates().findCoordinate(casa::Coordinate::DIRECTION));
	  casa::CoordinateSystem coo = this->itsInputCubePtr->coordinates();
	  casa::DirectionCoordinate dirCoo = coo.directionCoordinate(coo.findCoordinate(casa::Coordinate::DIRECTION));
	  double fwhmMajPix = inputBeam[0].getValue(dirCoo.worldAxisUnits()[0]) / dirCoo.increment()[0];
	  double majSDsq = fwhmMajPix * fwhmMajPix / 8. / M_LN2;
	  double fwhmMinPix = inputBeam[1].getValue(dirCoo.worldAxisUnits()[1]) / dirCoo.increment()[1];
	  double minSDsq = fwhmMinPix * fwhmMinPix / 8. / M_LN2;
	  
	  int hw = (this->itsBoxWidth - 1)/2;
	  this->itsBeamScaleFactor = 0.;
	  for(int y=-hw; y<=hw; y++){
	    for(int x=-hw; x<=hw; x++){
	      double u=x*costheta + y*sintheta;
	      double v=x*sintheta - y*costheta;
	      this->itsBeamScaleFactor += exp(-0.5 * (u*u/majSDsq + v*v/minSDsq));
	    }
	  }
	  ASKAPLOG_DEBUG_STR(logger, "Beam scale factor = " << this->itsBeamScaleFactor);
// 	  this->itsBeamScaleFactor = 1./this->itsBeamScaleFactor;
	}
      }

    }

    void SpectralBoxExtractor::extract()
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
      this->setBeamScale();
      ASKAPLOG_INFO_STR(logger, "Extracting spectrum from " << this->itsInputCube << " for source ID " << this->itsSource.getID());
      //	ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
      //	ImageOpener::registerOpenImageFunction(ImageOpener::MIRIAD, MIRIADImage::openMIRIADImage);

      // get cube shape and identify spectral/spatial axes
       IPosition shape = this->itsInputCubePtr->shape();
       CoordinateSystem coords = this->itsInputCubePtr->coordinates();
       //      accessors::CasaImageAccess ia;
       //      IPosition shape = ia.shape(this->itsInputCube);
       //      casa::CoordinateSystem coords = ia.coordSys(this->itsInputCube);
      ASKAPCHECK(coords.hasSpectralAxis(),"Input cube \""<<this->itsInputCube<<"\" has no spectral axis");
      ASKAPCHECK(coords.hasDirectionCoordinate(),"Input cube \""<<this->itsInputCube<<"\" has no spatial axes");
      int specAxis=coords.spectralAxisNumber();
      int lngAxis=coords.directionAxesNumbers()[0];
      int latAxis=coords.directionAxesNumbers()[1];
	

      // define the slicer based on the source's peak pixel location and the box width.
      // Make sure we don't go over the edges of the image.
      int hw = (this->itsBoxWidth - 1)/2;
      int xpeak = this->itsSource.getXPeak();
      int ypeak = this->itsSource.getYPeak();
      int zero=0;
      int xmin = std::max(zero, xpeak-hw), xmax=std::min(int(shape(lngAxis)-1),xpeak+hw);
      int ymin = std::max(zero, ypeak-hw), ymax=std::min(int(shape(latAxis)-1),ypeak+hw);
      casa::IPosition blc(shape.size(),0),trc(shape.size(),0);
      blc(lngAxis)=xmin; blc(latAxis)=ymin; blc(specAxis)=0;
      trc(lngAxis)=xmax; trc(latAxis)=ymax; trc(specAxis)=shape(specAxis)-1;
      this->itsSlicer = casa::Slicer(blc,trc,casa::Slicer::endIsLast);

       const SubImage<Float> *sub = new SubImage<Float>(*this->itsInputCubePtr, this->itsSlicer);
       casa::Array<Float> subarray=sub->get();
       //      casa::Array<Float> subarray = ia.read(this->itsInputCube, blc, trc);
      // initialise array
      casa::IPosition arrayshape(shape.size(),1); arrayshape(specAxis)=shape(specAxis);
      this->itsArray = casa::Array<Float>(arrayshape,0.);
      
      casa::IPosition chan(shape.size(),0);
      trc=trc-blc;
      blc=0;
      for(int z=0; z<shape(specAxis);z++){
	chan(specAxis)=z;
	blc(specAxis) = trc(specAxis) = z;
	this->itsArray(chan) =  sum(subarray(blc,trc))/ this->itsBeamScaleFactor;
	ASKAPLOG_DEBUG_STR(logger, "z="<<z<<", chan="<<chan <<", blc="<<blc<<", trc="<<trc<<", subarray(blc,trc)="<<subarray(blc,trc)<<", sum(thereof)="<<sum(subarray(blc,trc))<<", beam="<<this->itsBeamScaleFactor<<", this->itsArray(chan)="<<this->itsArray(chan));
     }

      ASKAPLOG_DEBUG_STR(logger,"Finished calculating array, here it is: " << this->itsArray);

      delete sub;

    }

    void SpectralBoxExtractor::writeImage()
    {
      ASKAPLOG_INFO_STR(logger, "Writing spectrum to " << this->itsOutputFilename);
       accessors::CasaImageAccess ia;

      // get the coordinate system from the input cube
//       IPosition shape = ia.shape(this->itsInputCube);
//       casa::CoordinateSystem coords = ia.coordSys(this->itsInputCube);
       IPosition shape = this->itsInputCubePtr->shape();
       CoordinateSystem coords = this->itsInputCubePtr->coordinates();
       casa::Unit units=this->itsInputCubePtr->units();
      ASKAPCHECK(coords.hasSpectralAxis(),"Input cube \""<<this->itsInputCube<<"\" has no spectral axis");
      ASKAPCHECK(coords.hasDirectionCoordinate(),"Input cube \""<<this->itsInputCube<<"\" has no spatial axes");
      int specAxis=coords.spectralAxisNumber();
      int lngAxis=coords.directionAxesNumbers()[0];
      int latAxis=coords.directionAxesNumbers()[1];

      // shift the reference pixel for the spatial coords, so that the RA/DEC (or whatever) are correct. Leave the spectral axis untouched.
      casa::Vector<Float> shift(shape.size(),0), incrFrac(shape.size(),1);
      casa::Vector<Int> newshape=shape.asVector();
      shift(lngAxis)=this->itsSource.getXPeak();
      shift(latAxis)=this->itsSource.getYPeak();
      coords.subImage(shift,incrFrac,newshape);

      // create the new image
      ia.create(this->itsOutputFilename,this->itsArray.shape(),coords);

      /// @todo save the new units - if units were per beam, remove this factor
      
      // write the array
      ia.write(this->itsOutputFilename,this->itsArray);
      ia.setUnits(this->itsOutputFilename, units.getName());

      

    }


  }

}
