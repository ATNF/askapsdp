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

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <string>
#include <sourcefitting/RadioSource.h>
#include <imageaccess/CasaImageAccess.h>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/Slicer.h>
#include <casa/BasicSL/String.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <lattices/Lattices/LatticeBase.h>
#include <Common/ParameterSet.h>
#include <measures/Measures/Stokes.h>

#include <utils/PolConverter.h>

ASKAP_LOGGER(logger, ".sourcedataextractor");

namespace askap {

  namespace analysis {

    SourceDataExtractor::SourceDataExtractor(const LOFAR::ParameterSet& parset)
    {
      this->itsInputCubePtr=0;
      this->itsSource=0;
      this->itsInputCube = ""; // start off with this blank. Needs to be set before calling openInput()
      this->itsInputCubeList = parset.getStringVector("spectralCube",std::vector<std::string>(0));
      this->itsCentreType = parset.getString("pixelCentre","peak");

      // Take the following from SynthesisParamsHelper.cc in Synthesis
      // there could be many ways to define stokes, e.g. ["XX YY"] or ["XX","YY"] or "XX,YY"
      // to allow some flexibility we have to concatenate all elements first and then 
      // allow the parser from PolConverter to take care of extracting the products.                                            
      const std::vector<std::string> stokesVec = parset.getStringVector("polarisation", std::vector<std::string>(1,"I"));
      std::string stokesStr;
      for (size_t i=0; i<stokesVec.size(); ++i) {
	stokesStr += stokesVec[i];
      }
      this->itsStokesList = scimath::PolConverter::fromString(stokesStr);
      
      this->verifyInputs();
      
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
      this->itsCentreType = other.itsCentreType;
      this->itsSlicer = other.itsSlicer;
      this->itsInputCube = other.itsInputCube;
      this->itsInputCubeList = other.itsInputCubeList;
      this->itsInputCubePtr = other.itsInputCubePtr;
      this->itsStokesList = other.itsStokesList;
      this->itsCurrentStokes = other.itsCurrentStokes;
      this->itsOutputFilenameBase = other.itsOutputFilenameBase;
      this->itsOutputFilename = other.itsOutputFilename;
      this->itsArray = other.itsArray;
      this->itsInputCoords = other.itsInputCoords;
      this->itsLngAxis = other.itsLngAxis;
      this->itsLatAxis = other.itsLatAxis;
      this->itsSpcAxis = other.itsSpcAxis;
      this->itsStkAxis = other.itsStkAxis;
      return *this;
    }

    casa::IPosition SourceDataExtractor::getShape(std::string image)
    {
      this->itsInputCube = image;
      casa::IPosition shape;
      if(this->openInput()){
	  shape=this->itsInputCubePtr->shape();
	  this->closeInput();
      }
      return shape;
    }

    void SourceDataExtractor::setSource(RadioSource* src)
    {
      /// @details Sets the source to be used. Also sets the output
      /// filename correctly with the suffix indicating the object's
      /// ID.  
      /// @param src The RadioSource detection used to centre the
      /// spectrum. The central pixel will be chosen to be the peak
      /// pixel, so this needs to be defined.

      this->itsSource = src;

      if(this->itsSource){
	// Append the source's ID string to the output filename
	int ID=this->itsSource->getID();
	std::stringstream ss;
	ss << this->itsOutputFilenameBase << "_" << ID;
	this->itsOutputFilename = ss.str();
	this->getLocation();
      }
    }


    void SourceDataExtractor::getLocation()
    {

      if(this->itsSource){

	std::string srcCentreType = this->itsSource->getCentreType();
	this->itsSource->setCentreType(this->itsCentreType);
	this->itsXloc = this->itsSource->getXcentre();
	this->itsYloc = this->itsSource->getYcentre();
	this->itsSource->setCentreType(srcCentreType);

      }

    }

    void SourceDataExtractor::checkPol(std::string image, casa::Stokes::StokesTypes stokes, int nStokesRequest)
    {

      this->itsInputCube = image;
      std::vector<casa::Stokes::StokesTypes> stokesvec(1,stokes);
      std::string polstring=scimath::PolConverter::toString(stokesvec)[0];
      if(this->openInput()){
	  int stokeCoo = this->itsInputCubePtr->coordinates().polarizationCoordinateNumber();
	  int stokeAxis = this->itsInputCubePtr->coordinates().polarizationAxisNumber();
	  if(stokeCoo==-1 || stokeAxis==-1) {
	      ASKAPCHECK(polstring=="I","Extraction: Input cube "<<image<<" has no polarisation axis, but you requested " << polstring);
	  }
	  else{
	      int nstoke=this->itsInputCubePtr->shape()[stokeAxis];
	      ASKAPCHECK(nstoke=nStokesRequest, "Extraction: input cube " << image << " has " << nstoke << " polarisations, whereas you requested " << nStokesRequest);
	      bool haveMatch=false;
	      for(int i=0;i<nstoke;i++){
		  haveMatch = haveMatch || (this->itsInputCubePtr->coordinates().stokesCoordinate(stokeCoo).stokes()[i] == stokes);
	      }
	      ASKAPCHECK(haveMatch, "Extraction: input cube "<<image<<" does not have requested polarisation " << polstring);
	  }
	  
	  this->closeInput();
      }
      else ASKAPLOG_ERROR_STR(logger, "Could not open image");
    }
    
    void SourceDataExtractor::verifyInputs()
    {
      std::vector<std::string>::iterator im;
      std::vector<std::string> pollist=scimath::PolConverter::toString(this->itsStokesList);
      ASKAPCHECK(this->itsInputCubeList.size()>0,"Extraction: You have not provided a spectralCube input");
      ASKAPCHECK(this->itsStokesList.size()>0,"Extraction: You have not provided a list of Stokes parameters (input parameter \"polarisation\")");
    
      if(this->itsInputCubeList.size() > 1 ){ // multiple input cubes provided
	ASKAPCHECK(this->itsInputCubeList.size() == this->itsStokesList.size(), "Extraction: Sizes of spectral cube and polarisation lists do not match");
	int ct=0;
	for(im=this->itsInputCubeList.begin();im<this->itsInputCubeList.end();im++,ct++){
	  this->checkPol(*im, this->itsStokesList[ct],1);
	}
	// check they are all the same shape
	casa::IPosition refShape=this->getShape(this->itsInputCubeList[0]);
	for(size_t i=1;i<this->itsInputCubeList.size();i++){
	  ASKAPCHECK(refShape == this->getShape(this->itsInputCubeList[i]), "Extraction: shapes of " << this->itsInputCubeList[0] << " and " << this->itsInputCubeList[i] << " do not match");
	}
      }
      else{ // only have a single input cube
	if(this->itsStokesList.size() == 1 ){ // only single Stokes parameter requested -- check if it matches the image
	  this->checkPol(this->itsInputCubeList[0], this->itsStokesList[0],1);
	}
	else { // multiple Stokes parameters requested
	  if(this->itsInputCubeList[0].find("%p") != std::string::npos) { // the filename as a "%p" string, meaning polarisation substitution is possible
	    std::string input = this->itsInputCubeList[0];
	    this->itsInputCubeList=std::vector<std::string>(this->itsStokesList.size());
	    casa::Stokes stokes;
	    for(size_t i=0;i<this->itsStokesList.size();i++){
	      casa::String stokesname(stokes.name(this->itsStokesList[i]));
	      stokesname.downcase();
	      ASKAPLOG_DEBUG_STR(logger,"Input cube name: replacing \"%p\" with " << stokesname.c_str() << " in " << input);
	      this->itsInputCubeList[i] = input;
	      this->itsInputCubeList[i].replace(input.find("%p"),2,stokesname.c_str());
	      this->checkPol(this->itsInputCubeList[i], this->itsStokesList[i],1);
	    }
	  }
	  else{
	    // get list of polarisations in that one image - are all the requested ones there?
	    ASKAPCHECK(this->itsInputCubeList.size()==1, "Extraction: For multiple polarisations, either use %p substitution or provide a single image cube.");
	    for(size_t i=0;i<this->itsStokesList.size();i++){
	      this->checkPol(this->itsInputCubeList[0], this->itsStokesList[i],this->itsStokesList.size());
	    }
	    // else{
	    //   std::string polset="[";
	    //   std::vector<std::string> pols=this->polarisations();
	    //   for(size_t i=0;i<pols.size();i++) polset+=pols[i]+(i!=pols.size()-1?",":"");
	    //   polset+="]";
	    //   ASKAPTHROW(AskapError, "Extraction: You have provided more than one stokes parameter ("<<polset<<"\") but only one input cube that doesn't contain all of these");
	    // }
	  }
	}
      }
    }

      
      void SourceDataExtractor::writeBeam(std::string &filename)
      {
	  casa::Vector<Quantum<Double> > inputBeam = this->itsInputCubePtr->imageInfo().restoringBeam();
	  accessors::CasaImageAccess ia;
	  ia.setBeamInfo(filename, inputBeam[0].getValue("rad"), inputBeam[1].getValue("rad"), inputBeam[2].getValue("rad"));
      }


    bool SourceDataExtractor::openInput()
    {
	bool isOK = (this->itsInputCubePtr == 0); // if non-zero, we have already opened the cube
	isOK = isOK && (this->itsInputCube != "");

      if(isOK){ 
	ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
	ImageOpener::registerOpenImageFunction(ImageOpener::MIRIAD, MIRIADImage::openMIRIADImage);
	this->itsLatticePtr = ImageOpener::openImage(this->itsInputCube);
	if (this->itsLatticePtr == 0){
	    ASKAPLOG_ERROR_STR(logger, "Requested input cube \"" << this->itsInputCube << "\" does not exist or could not be opened.");
	    isOK=false;
	}
	else{
	    this->itsInputCubePtr = dynamic_cast< const ImageInterface<Float>* >(this->itsLatticePtr);

	    this->itsInputCoords = this->itsInputCubePtr->coordinates();
	    this->itsLngAxis=this->itsInputCoords.directionAxesNumbers()[0];
	    this->itsLatAxis=this->itsInputCoords.directionAxesNumbers()[1];
	    this->itsSpcAxis=this->itsInputCoords.spectralAxisNumber();
	    this->itsStkAxis=this->itsInputCoords.polarizationAxisNumber();
	}
      }
      return isOK;
    }

    void SourceDataExtractor::closeInput()
    {
      delete this->itsLatticePtr;
      this->itsInputCubePtr = 0;
      
    }

  }

}
