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
#include <parallelanalysis/Weighter.h>
#include <analysisutilities/CasaImageUtil.h>
#include <askap_analysis.h>

#include <askapparallel/AskapParallel.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <casa/aipstype.h>
#include <casa/Arrays/Vector.h>

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

    Weighter::Weighter(askap::mwbase::AskapParallel& comms):
      itsComms(comms)
    {
    }

    void Weighter::initialise(std::string &weightsImage, duchamp::Section &section)
    {
      this->itsImage = weightsImage;
      this->itsSection = section;
      this->readWeights();
      this->findNorm();
    }

    void Weighter::readWeights()
    {
      this->itsWeights = getPixelsInBox(itsImage,subsectionToSlicer(itsSection));
    }

    void Weighter::findNorm()
    {
      if(itsComms.isParallel()){
	LOFAR::BlobString bs;
	if(itsComms.isWorker()){
	  // find maximum of weights and send to master
	  float maxW = *std::max_element(this->itsWeights.begin(),this->itsWeights.end());
	  bs.resize(0);
	  LOFAR::BlobOBufString bob(bs);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("localmax", 1);
	  out << maxW;
	  out.putEnd();
	  itsComms.connectionSet()->write(0, bs);

	  // now read actual maximum from master
	  itsComms.connectionSet()->read(0, bs);
	  LOFAR::BlobIBufString bib(bs);
	  LOFAR::BlobIStream in(bib);
	  int version = in.getStart("maxweight");
	  ASKAPASSERT(version == 1);
	  in >> this->itsNorm;
	  in.getEnd();
	}
	else if(itsComms.isMaster()) {
	  // read local maxima from workers and find the maximum of them
	  for (int n=0;n<itsComms.nNodes()-1;n++){
	    float localmax;
	    itsComms.connectionSet()->read(n, bs);
	    LOFAR::BlobIBufString bib(bs);
	    LOFAR::BlobIStream in(bib);
	    int version = in.getStart("localmax");
	    ASKAPASSERT(version == 1);
	    in >> localmax;
	    itsNorm = (n==0) ? localmax : std::max(localmax,itsNorm);
	    in.getEnd();
	  }
	  // send the actual maximum to all workers
	  bs.resize(0);
	  LOFAR::BlobOBufString bob(bs);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("maxweight", 1);
	  out << this->itsNorm;
	  out.putEnd();
	  itsComms.connectionSet()->writeAll(bs);
	  }
      }
      else { 
	// serial mode - read entire weights image, so can just measure maximum directly
	this->itsNorm = *std::max_element(this->itsWeights.begin(),this->itsWeights.end());
      }

      ASKAPLOG_INFO_STR(logger, "Normalising weights image to maximum " << this->itsNorm);
      

    }

    float Weighter::weight(int i)
    {
      return sqrt(itsWeights(i)/itsNorm);
    }


  }

}
