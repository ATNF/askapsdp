/// @file
///
/// XXX Notes on program XXX
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
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#include <askap_analysis.h>
#include <parallelanalysis/Weighter.h>

#include <casainterface/CasaInterface.h>

#include <askapparallel/AskapParallel.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <casa/aipstype.h>
#include <casa/Arrays/Vector.h>
#define AIPS_ARRAY_INDEX_CHECK

#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

#include <vector>
#include <string>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".weighter");

namespace askap {

  namespace analysis {

    Weighter::Weighter(askap::askapparallel::AskapParallel& comms, const LOFAR::ParameterSet &parset):
      itsComms(&comms)
    {
      this->itsImage = parset.getString("weightsImage","");
      if(this->itsComms->isMaster()) ASKAPLOG_INFO_STR(logger, "Using weights image: " << this->itsImage);
      this->itsFlagDoScaling = parset.getBool("scaleByWeights",false);
      this->itsWeightCutoff = parset.getFloat("weightsCutoff",-1.);
    }

    Weighter::Weighter(const Weighter& other)
    {
      this->operator=(other);
    }

    Weighter& Weighter::operator= (const Weighter& other)
    {
      if(this == &other) return *this;
      this->itsComms = other.itsComms;
      this->itsImage = other.itsImage;
      this->itsCube = other.itsCube;
      this->itsNorm = other.itsNorm;
      this->itsWeights = other.itsWeights;
      return *this;
    }

    void Weighter::initialise(duchamp::Cube &cube, bool doAllocation)
    {
      this->itsCube = &cube;
      if(doAllocation) this->readWeights();

      if (this->itsFlagDoScaling || (this->itsWeightCutoff > 0.) )
	this->findNorm();
    }
      
    void Weighter::readWeights()
    {
      ASKAPCHECK(this->itsImage!="", "Weights image not defined");
      ASKAPLOG_INFO_STR(logger, "Reading weights from " << this->itsImage << ", section " << this->itsCube->pars().section().getSection());
      this->itsWeights = analysisutilities::getPixelsInBox(this->itsImage,analysisutilities::subsectionToSlicer(this->itsCube->pars().section()),false);
    }

    void Weighter::findNorm()
    {
      if(itsComms->isParallel()){
	LOFAR::BlobString bs;
	if(itsComms->isWorker()){
	  if(this->itsWeights.size()==0)
	    ASKAPLOG_ERROR_STR(logger, "Weights array not initialised!");
	  // find maximum of weights and send to master
	  //		    float maxW = *std::max_element(this->itsWeights.begin(),this->itsWeights.end());
	  float maxW = max(this->itsWeights);
	  ASKAPLOG_DEBUG_STR(logger, "Local maximum weight = " << maxW);
	  bs.resize(0);
	  LOFAR::BlobOBufString bob(bs);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("localmax", 1);
	  out << maxW;
	  out.putEnd();
	  itsComms->sendBlob(bs, 0);

	  // now read actual maximum from master
	  itsComms->broadcastBlob(bs, 0);
	  LOFAR::BlobIBufString bib(bs);
	  LOFAR::BlobIStream in(bib);
	  int version = in.getStart("maxweight");
	  ASKAPASSERT(version == 1);
	  in >> this->itsNorm;
	  in.getEnd();
	}
	else if(itsComms->isMaster()) {
	  // read local maxima from workers and find the maximum of them
	  for (int n=0;n<itsComms->nProcs()-1;n++){
	    float localmax;
	    itsComms->receiveBlob(bs, n + 1);
	    LOFAR::BlobIBufString bib(bs);
	    LOFAR::BlobIStream in(bib);
	    int version = in.getStart("localmax");
	    ASKAPASSERT(version == 1);
	    in >> localmax;
	    this->itsNorm = (n==0) ? localmax : std::max(localmax,itsNorm);
	    in.getEnd();
	  }
	  // send the actual maximum to all workers
	  bs.resize(0);
	  LOFAR::BlobOBufString bob(bs);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("maxweight", 1);
	  out << this->itsNorm;
	  out.putEnd();
	  itsComms->broadcastBlob(bs, 0);
	}
      }
      else { 
	// serial mode - read entire weights image, so can just measure maximum directly
	// this->itsNorm = *std::max_element(this->itsWeights.begin(),this->itsWeights.end());
	this->itsNorm = max(this->itsWeights);
      }

      ASKAPLOG_INFO_STR(logger, "Normalising weights image to maximum " << this->itsNorm);
      

    }

    float Weighter::weight(size_t i)
    {
      ASKAPCHECK(i < this->itsWeights.size(), "Index out of bounds for weights array : index="<<i<<", weights array is size " << this->itsWeights.size());
      // return sqrt(this->itsWeights(i)/this->itsNorm);
      return sqrt(this->itsWeights.data()[i]/this->itsNorm);
    }

    void Weighter::applyCutoff()
    {
      if (this->itsWeightCutoff > 0.){

	ASKAPASSERT(this->itsCube->getSize() == this->itsWeights.size());
	ASKAPASSERT(this->itsCube->getRecon() > 0);
	    
	float blankValue = this->itsCube->pars().getBzeroKeyword() + this->itsCube->pars().getBlankKeyword()*this->itsCube->pars().getBscaleKeyword();
	ASKAPASSERT(this->itsCube->pars().isBlank(blankValue));//, "Blank value failed isBlank test in Weighter::applyCutoff");
	    
	for(size_t i=0; i<this->itsCube->getSize();i++){
	  if(this->weight(i) < this->itsWeightCutoff)
	    this->itsCube->getArray()[i] = blankValue;
	}

      }
    }

    void Weighter::search()
    {

      if (this->itsFlagDoScaling){

	ASKAPASSERT(this->itsCube->getSize() == this->itsWeights.size());
	ASKAPASSERT(this->itsCube->getRecon() > 0);
	for(size_t i=0; i<this->itsCube->getSize();i++){
	  this->itsCube->getRecon()[i] = this->itsCube->getPixValue(i)*this->weight(i);
	}
	this->itsCube->setReconFlag(true);

	ASKAPLOG_DEBUG_STR(logger, "Searching weighted image to threshold " << this->itsCube->stats().getThreshold());
	this->itsCube->ObjectList() = searchReconArray(this->itsCube->getDimArray(),this->itsCube->getArray(),
						       this->itsCube->getRecon(),this->itsCube->pars(),this->itsCube->stats());
      }
      else{
	ASKAPLOG_DEBUG_STR(logger, "Searching image to threshold " << this->itsCube->stats().getThreshold());
	this->itsCube->ObjectList() = search3DArray(this->itsCube->getDimArray(),this->itsCube->getArray(),
						    this->itsCube->pars(),this->itsCube->stats());

      }

      this->itsCube->updateDetectMap();
      if(this->itsCube->pars().getFlagLog())
	this->itsCube->logDetectionList();
	  
    }


  }

}
