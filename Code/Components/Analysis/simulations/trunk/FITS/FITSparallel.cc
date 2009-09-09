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
	LOFAR::ParameterSet newparset = parset;

	this->itsSubimageDef = analysis::SubimageDef(parset);
 	int numSub = this->itsSubimageDef.nsubx() * this->itsSubimageDef.nsuby();
	if(this->isParallel() && (numSub != this->itsNNode-1))
	  ASKAPTHROW(AskapError, "Number of requested subimages ("<<numSub<<", = " 
		     << this->itsSubimageDef.nsubx()<<"x"<<this->itsSubimageDef.nsuby()
		     <<") does not match the number of worker nodes ("<<this->itsNNode-1<<")");
	

// 	this->itsNsubx = parset.getInt16("nsubx",1);
// 	this->itsNsuby = parset.getInt16("nsuby", 1);
// 	int numSub = this->itsNsubx * this->itsNsuby;
// 	if(numSub != this->itsNNode-1)
// 	  ASKAPTHROW(AskapError, "Number of requested subimages ("<<numSub<<", = " 
// 		     << this->itsNsubx<<"x"<<this->itsNsuby
// 		     <<") does not match the number of worker nodes ("<<this->itsNNode-1<<")");

	size_t dim = parset.getInt32("dim", 2);
// 	this->itsAxes = parset.getInt32Vector("axes");
	std::vector<int> axes = parset.getInt32Vector("axes");

	this->itsSubimageDef.define(dim);
	this->itsSubimageDef.setImageDim(axes);

// 	if (this->itsAxes.size() != dim)
// 	  ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << dim << ", but axes has " << this->itsAxes.size() << " dimensions.");
	if (axes.size() != dim)
	  ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << dim << ", but axes has " << axes.size() << " dimensions.");
	
// 	this->itsXmin = 0;
// 	this->itsYmin = 0;
	
	if(this->isParallel() && this->isWorker()) {

// 	  // the consecutive number of the subimage, numbered in the usual raster fashion
// 	  int xsubnum = (this->itsRank-1)%this->itsNsubx;
// 	  int ysubnum = ((this->itsRank-1)%(this->itsNsubx*this->itsNsuby))/this->itsNsubx;
// 	  // the length of the subsections in each direction
// 	  float xsublength = float(this->itsAxes[0]) / float(this->itsNsubx);
// 	  float ysublength = float(this->itsAxes[1]) / float(this->itsNsuby);
// 	  // the minimum pixel of the subimage in each direction
// 	  this->itsXmin = long( xsubnum * xsublength);
// 	  this->itsYmin = long( ysubnum * ysublength);

// 	  this->itsAxes[0] = int(xsublength);
// 	  this->itsAxes[1] = int(ysublength);

	  this->itsSubsection = this->itsSubimageDef.section(this->itsRank-1,duchamp::nullSection(dim));
// 	  this->itsXmin = subsection.getStart(0);
// 	  this->itsYmin = subsection.getStart(1);
// 	  this->itsAxes[0] = subsection.getDim(0);
// 	  this->itsAxes[1] = subsection.getDim(1);
	  axes[0] = this->itsSubsection.getDim(0);
	  axes[1] = this->itsSubsection.getDim(1);

	  newparset.replace(LOFAR::KVpair("axes",this->itsAxes));

// 	  ASKAPLOG_DEBUG_STR(logger, "Worker #"<<this->itsRank<<" has offsets (" << this->itsXmin<<","<<this->itsYmin
// 			     <<") and dimensions "<<this->itsAxes[0]<<"x"<<this->itsAxes[1]);
	  ASKAPLOG_DEBUG_STR(logger, "Worker #"<<this->itsRank<<" has offsets (" << this->itsSubsection.getStart(0) <<","<< this->itsSubsection.getStart(1)
			     <<") and dimensions "<< this->itsSubsection.getDim(0) << "x"<<this->itsSubsection.getDim(1));

	  if(newparset.isDefined("WCSimage.crpix")) {
	    std::vector<int> crpix = newparset.getInt32Vector("WCSimage.crpix");
// 	    crpix[0] -= this->itsXmin;
// 	    crpix[1] -= this->itsYmin;
	    crpix[0] -= this->itsSubsection.getStart(0);
	    crpix[1] -= this->itsSubsection.getStart(1);
	    newparset.replace(LOFAR::KVpair("WCSimage.crpix",crpix));
	  }

	  if(newparset.isDefined("WCSsources.crpix")) {
	    std::vector<int> crpix = newparset.getInt32Vector("WCSsources.crpix");
// 	    crpix[0] -= this->itsXmin;
// 	    crpix[1] -= this->itsYmin;
	    crpix[0] -= this->itsSubsection.getStart(0);
	    crpix[1] -= this->itsSubsection.getStart(1);
	    newparset.replace(LOFAR::KVpair("WCSsources.crpix",crpix));
	  }

	}
	else{
	  this->itsSubsection.setSection(duchamp::nullSection(dim));
	  std::vector<long> laxes(axes.size());
	  for(size_t i=0;i<laxes.size();i++) laxes[i]=axes[i];
	  this->itsSubsection.parse(laxes);
	}

//  	std::cerr << newparset;

	this->itsFITSfile = FITSfile(newparset);

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
	    out << this->itsSubsection.getStart(0) << this->itsSubsection.getStart(1) << this->itsSubsection.getDim(0) << this->itsSubsection.getDim(1);
	    ASKAPLOG_DEBUG_STR(logger, "Worker #"<<this->itsRank<<": sent minima of " << this->itsSubsection.getStart(0) << " and " << this->itsSubsection.getStart(1));
// 	    out << this->itsXmin << this->itsYmin << this->itsAxes[0] << this->itsAxes[1];
// 	    ASKAPLOG_DEBUG_STR(logger, "Worker #"<<this->itsRank<<": sent minima of " << this->itsXmin << " and " << this->itsYmin);
// 	    for(int y=0;y<this->itsAxes[1];y++){
// 	      for(int x=0;x<this->itsAxes[0];x++){
	    for(int y=0;y<this->itsSubsection.getDim(1);y++){
	      for(int x=0;x<this->itsSubsection.getDim(0);x++){
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
	      int xmin,ymin,xdim,ydim;
	      in >> xmin >> ymin >> xdim >> ydim;
	      ASKAPLOG_DEBUG_STR(logger, "MASTER: Read minima of " << xmin << " and " << ymin);
	      for(int y=0;y<ydim;y++){
		for(int x=0;x<xdim;x++){
		  int xpt,ypt;
		  float flux;
		  in >> xpt >> ypt >> flux;
		  ASKAPASSERT(x==xpt);
		  ASKAPASSERT(y==ypt);
		  this->itsFITSfile.setArray(x+xmin,y+ymin,flux);
		}
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
	if(this->isWorker())
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
