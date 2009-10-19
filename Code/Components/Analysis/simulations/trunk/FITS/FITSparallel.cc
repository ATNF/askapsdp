/// @file
///
/// Implementation of the parallel handling of FITS creation
///
/// @copyright (c) 2008 CSIRO
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

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <FITS/FITSparallel.h>
#include <FITS/FITSfile.h>

#include <askapparallel/AskapParallel.h>
#include <duchamp/Utils/Section.hh>
#include <analysisutilities/SubimageDef.h>

#include <Common/LofarTypedefs.h>
using namespace LOFAR::TYPES;
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Common/Exceptions.h>

#include <Common/ParameterSet.h>
#include <Common/KVpair.h>

#include <vector>
#include <iostream>
#include <sstream>

ASKAP_LOGGER(logger, ".fitsparallel");

namespace askap {

  namespace simulations {

    namespace FITS {


      FITSparallel::FITSparallel(int argc, const char** argv, const LOFAR::ParameterSet& parset)
	: AskapParallel(argc,argv)
      {
	ASKAPLOG_DEBUG_STR(logger, "Starting the definition of FITSparallel");

	LOFAR::ParameterSet newparset = parset;

	this->itsSubimageDef = analysis::SubimageDef(parset);
 	int numSub = this->itsSubimageDef.nsubx() * this->itsSubimageDef.nsuby();
	if(this->isParallel() && (numSub != this->itsNNode-1))
	  ASKAPTHROW(AskapError, "Number of requested subimages ("<<numSub<<", = " 
		     << this->itsSubimageDef.nsubx()<<"x"<<this->itsSubimageDef.nsuby()
		     <<") does not match the number of worker nodes ("<<this->itsNNode-1<<")");
	
	size_t dim = parset.getInt32("dim", 2);
	std::vector<int> axes = parset.getInt32Vector("axes");

	this->itsSubimageDef.define(dim);
	this->itsSubimageDef.setImageDim(axes);

	if (axes.size() != dim)
	  ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << dim << ", but axes has " << axes.size() << " dimensions.");
	
	if(this->isParallel() && this->isWorker()) {

	  this->itsSubsection = this->itsSubimageDef.section(this->itsRank-1,duchamp::nullSection(dim));

	  ASKAPLOG_DEBUG_STR(logger, "Worker #"<<this->itsRank<<" has offsets (" << this->itsSubsection.getStart(0) <<","<< this->itsSubsection.getStart(1)
			     <<") and dimensions "<< this->itsSubsection.getDim(0) << "x"<<this->itsSubsection.getDim(1));

	}
	else{
	  this->itsSubsection.setSection(duchamp::nullSection(dim));
	  this->itsSubsection.parse(axes);
	}

	ASKAPLOG_DEBUG_STR(logger, "Defining FITSfile");
	this->itsFITSfile = FITSfile(newparset);
	ASKAPLOG_DEBUG_STR(logger, "Defined");
	this->itsFITSfile.setSection(this->itsSubsection);

	ASKAPLOG_DEBUG_STR(logger, "Finished defining FITSparallel");

      }

      //--------------------------------------------------

      void FITSparallel::toMaster()
      {

	if(this->isParallel()){

	  if(this->isWorker()){
	    ASKAPLOG_DEBUG_STR(logger, "Worker #"<<this->itsRank<<": about to send data to Master");
	    LOFAR::BlobString bs;
	    bs.resize(0);
	    LOFAR::BlobOBufString bob(bs);
	    LOFAR::BlobOStream out(bob);
	    out.putStart("pixW2M", 1);
	    out << this->itsSubsection.getStart(0) << this->itsSubsection.getStart(1) << this->itsSubsection.getEnd(0) << this->itsSubsection.getEnd(1);
	    ASKAPLOG_DEBUG_STR(logger, "Worker #"<<this->itsRank<<": sent minima of " << this->itsSubsection.getStart(0) << " and " << this->itsSubsection.getStart(1));
 	    for(int y=this->itsSubsection.getStart(1);y<=this->itsSubsection.getEnd(1);y++){
 	      for(int x=this->itsSubsection.getStart(0);x<=this->itsSubsection.getEnd(0);x++){
// 	    for(int y=0;y<this->itsFITSfile.getYdim();y++){
// 	      for(int x=0;x<this->itsFITSfile.getXdim();x++){
		out << x << y << this->itsFITSfile.array(x,y);
	      }
	    }

	    out.putEnd();
	    this->itsConnectionSet->write(0, bs);

	  }
	  else if(this->isMaster()) {

	    LOFAR::BlobString bs;
	    for(int n=1;n<this->itsNNode;n++){
	      ASKAPLOG_DEBUG_STR(logger, "MASTER: about to read data from Worker #"<<n);
	      this->itsConnectionSet->read(n - 1, bs);
	      LOFAR::BlobIBufString bib(bs);
	      LOFAR::BlobIStream in(bib);
	      int version = in.getStart("pixW2M");
	      ASKAPASSERT(version == 1);
	      int xmin,ymin,xmax,ymax;
	      in >> xmin >> ymin >> xmax >> ymax;
	      ASKAPLOG_DEBUG_STR(logger, "MASTER: Read minima of " << xmin << " and " << ymin);
// 	      for(int y=ymin;y<=ymax;y++){
// 		for(int x=xmin;x<=xmax;x++){
// 	      for(int y=0;y<this->itsFITSfile.getYdim();y++){
// 		for(int x=0;x<this->itsFITSfile.getXdim();x++){
	      for(int pix=0;pix<(xmax-xmin+1)*(ymax-ymin+1);pix++){
		  int xpt,ypt;
		  float flux;
		  in >> xpt >> ypt >> flux;
		  ASKAPASSERT(xpt==(xmin+pix%(xmax-xmin+1)));
		  ASKAPASSERT(ypt==(ymin+pix/(xmax-xmin+1)));
		  flux += this->itsFITSfile.array(xpt,ypt);
		  this->itsFITSfile.setArray(xpt,ypt,flux);
// 		}
	      }
	      in.getEnd();

	    }

	  }

	}


      }

      //--------------------------------------------------

      void FITSparallel::addNoise()
      {
	if(this->isWorker())
	  itsFITSfile.addNoise();
      }
	
      void FITSparallel::addSources()
      {
	if(this->isWorker()){
	  ASKAPLOG_DEBUG_STR(logger, "Worker #"<<this->itsRank<<": About to add sources");
	  itsFITSfile.addSources();
	}
      }
	
      void FITSparallel::convolveWithBeam()
      {
	  itsFITSfile.convolveWithBeam();
      }
	
      void FITSparallel::saveFile()
      {
	if(this->isMaster())
	  itsFITSfile.saveFile();
      }



    }

  }

}
