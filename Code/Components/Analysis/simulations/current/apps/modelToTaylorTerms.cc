///
/// @file : Create Taylor term images from a cube
///
/// Control parameters are passed in from a LOFAR ParameterSet file.
///
/// @copyright (c) 2007 CSIRO
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
#include <askap_simulations.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <casa/Logging/LogIO.h>
#include <askap/Log4cxxLogSink.h>
#include <askapparallel/AskapParallel.h>

#include <analysisutilities/CasaImageUtil.h>

#include <Common/LofarTypedefs.h>
using namespace LOFAR::TYPES;
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Common/Exceptions.h>

#include <Common/ParameterSet.h>
#include <casa/OS/Timer.h>
#include <casa/namespace.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>
#include <coordinates/Coordinates/Coordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageInfo.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <gsl/gsl_multifit.h>
#include <wcslib/wcs.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <time.h>

using namespace askap;

ASKAP_LOGGER(logger, "modelToTaylorTerms.log");

// Move to Askap Util?
std::string getInputs(const std::string& key, const std::string& def, int argc,
                      const char** argv)
{
  if (argc > 2) {
    for (int arg = 0; arg < (argc - 1); arg++) {
      std::string argument = std::string(argv[arg]);

      if (argument == key) {
	return std::string(argv[arg+1]);
      }
    }
  }

  return def;
}

int main(int argc, const char **argv)
{

  askap::askapparallel::AskapParallel comms(argc, argv);

  try 
    {
      // Ensure that CASA log messages are captured
      casa::LogSinkInterface* globalSink = new Log4cxxLogSink();
      casa::LogSink::globalSink(globalSink);

      std::string parsetFile(getInputs("-inputs", "modelToTaylorTerms.in", argc, argv));
      ASKAPLOG_INFO_STR(logger,  "parset file " << parsetFile);
      LOFAR::ParameterSet parset(parsetFile);
      ASKAPLOG_INFO_STR(logger, "Full file follows:\n"<<parset);
      LOFAR::ParameterSet subset(parset.makeSubset("model2TT."));
      ASKAPLOG_INFO_STR(logger, "Subset follows:\n"<<subset);

      std::string modelimage=subset.getString("inputmodel","");
      std::string modelimagebase = modelimage.substr(modelimage.rfind('/')+1,modelimage.size());
      int nsubx=subset.getInt16("nsubx",1);
      int nsuby=subset.getInt16("nsuby",1);
      if(comms.isParallel()){
	ASKAPCHECK(nsubx*nsuby+1 == comms.nProcs(),"nsubx and nsuby need to match the number of workers");
      }
      else {
	nsubx=nsuby=1;
      }
      const int nterms=3;
      float logevery = subset.getFloat("logevery",10.);
      ASKAPLOG_INFO_STR(logger, "Will log every "<<logevery << "% of the time");

      casa::PagedImage<Float> img(modelimage);
      IPosition shape = img.shape();
      casa::CoordinateSystem csys = img.coordinates();
      int specCoord = csys.findCoordinate(Coordinate::SPECTRAL);
      int specAxis = csys.worldAxes(specCoord)[0];
      ASKAPLOG_DEBUG_STR(logger, "Model image " << modelimage << " with basename " << modelimagebase << " has shape " << shape << " and the spectral axis is #"<<specAxis );
	
      Unit bunit = img.units();
      casa::Vector<casa::Quantum<Double> > beam = img.imageInfo().restoringBeam();
      casa::ImageInfo ii = img.imageInfo();
      ii.setRestoringBeam(beam);


      if(comms.isMaster()){

	for(int t=0;t<nterms;t++){
	  std::stringstream outname;
	  outname << modelimagebase <<".taylor." << t;

	  casa::IPosition tileshape(shape.size(),1);
	  tileshape(0) = std::min(128L,shape(0));
	  tileshape(1) = std::min(128L,shape(1));
	  casa::IPosition fulloutshape(shape);
	  fulloutshape(specAxis)=1;
	
	  ASKAPLOG_INFO_STR(logger, "Creating a new CASA image " << outname.str() << " with the shape " << fulloutshape << " and tileshape " << tileshape);
	  casa::PagedImage<float> outimg(casa::TiledShape(fulloutshape,tileshape), csys, outname.str());
	
	  outimg.setUnits(bunit);
	  outimg.setImageInfo(ii);

	}
	if(comms.isParallel()){
	  bool OK;
	  LOFAR::BlobString bs;
	  for (int i = 1; i < comms.nProcs(); i++) {
	    // First send the node number
	    ASKAPLOG_DEBUG_STR(logger, "MASTER: Sending 'go' to worker#" << i);
	    bs.resize(0);
	    LOFAR::BlobOBufString bob(bs);
	    LOFAR::BlobOStream out(bob);
	    out.putStart("goInput", 1);
	    out << i ;
	    out.putEnd();
	    comms.sendBlob(bs, i);
	    ASKAPLOG_DEBUG_STR(logger, "MASTER: Sent. Now waiting for reply from worker#"<<i);
	    // Then wait for the OK from that node
	    bs.resize(0);
	    ASKAPLOG_DEBUG_STR(logger, "MASTER: Reading from connection "<< i-1);
	    comms.receiveBlob(bs, i);
	    LOFAR::BlobIBufString bib(bs);
	    LOFAR::BlobIStream in(bib);
	    int version = in.getStart("inputDone");
	    ASKAPASSERT(version == 1);
	    in >> OK;
	    in.getEnd();

	    ASKAPLOG_DEBUG_STR(logger, "MASTER: Received. Worker#"<<i<<" done.");
	    if (!OK) ASKAPTHROW(AskapError, "Staged writing of image failed.");
	    ASKAPLOG_DEBUG_STR(logger, "MASTER: Received. Worker#"<<i<<" done.");
	  }
	}
      }


      if(comms.isWorker()){

	int nx=0,ny=0;
	size_t xmin,xmax,ymin,ymax,xlen,ylen;
	// 	std::stringstream outputnamebase;
	if(comms.isParallel()){
	  nx = (comms.rank()-1) % nsubx;
	  ny = (comms.rank()-1) / nsubx;
	  xmin = size_t( nx * float(shape[0])/float(nsubx) );
	  xmax = size_t( (nx+1) * float(shape[0])/float(nsubx) )-1;
	  ymin = size_t( ny * float(shape[1])/float(nsuby) );
	  ymax = size_t( (ny+1) * float(shape[1])/float(nsuby) )-1;
	  ASKAPLOG_DEBUG_STR(logger, "rank="<<comms.rank()-1<<" nx="<<nx<<" ny="<<ny<<" xmin="<<xmin<<" xmax="<<xmax<<" ymin="<<ymin<<" ymax="<<ymax);
	}
	else{ // if serial mode, use the full range of x & y
	  xmin=ymin=0;
	  xmax=shape[0]-1;
	  ymax=shape[1]-1;
	}
	xlen=xmax-xmin+1;
	ylen=ymax-ymin+1;

	ASKAPLOG_DEBUG_STR(logger, "isParallel="<<comms.isParallel()<< " rank="<<comms.rank()<<"   x in ["<<xmin<<","<<xmax<<"]   y in ["<<ymin << "," << ymax << "]");

	casa::IPosition outshape(2,xlen,ylen);
	outshape[specAxis] = 1;
	ASKAPLOG_DEBUG_STR(logger, "Shape of output images is " << outshape);
	casa::Array<Float> outputs[nterms];
	for(int i=0;i<nterms;i++){
	  outputs[i] = casa::Array<Float>(outshape,0.);
	}


	const int ndata=shape[specAxis];
	const int degree=nterms+2;
	double chisq;
	gsl_matrix *xdat, *cov;
	gsl_vector *ydat, *w, *c;
	xdat = gsl_matrix_alloc(ndata,degree);
	ydat = gsl_vector_alloc(ndata);
	w = gsl_vector_alloc(ndata);
	c = gsl_vector_alloc(degree);
	cov = gsl_matrix_alloc(degree,degree);

      
	double reffreq = csys.spectralCoordinate(specCoord).referenceValue()[0];
	for(int i=0;i<ndata;i++){
	  double freq;
	  if(!csys.spectralCoordinate(specCoord).toWorld(freq,double(i)))
	    ASKAPLOG_ERROR_STR(logger, "Error converting spectral coordinate at channel " << i);
	  float logfreq = log10(freq/reffreq);
	  gsl_matrix_set(xdat,i,0,1.);
	  gsl_matrix_set(xdat,i,1,logfreq);
	  gsl_matrix_set(xdat,i,2,logfreq*logfreq);
	  gsl_matrix_set(xdat,i,3,logfreq*logfreq*logfreq);
	  gsl_matrix_set(xdat,i,4,logfreq*logfreq*logfreq*logfreq);
	  gsl_vector_set(w,i,1.);
	}

	
	casa::IPosition start(shape.size(),0);
	start[0]=xmin;
	start[1]=ymin;
	casa::IPosition end(shape-1);
	end[0] = xmax;
	end[1] = ymax;
	casa::IPosition outpos(2,0,0);
	
	float *subcube = new float[xlen*ylen*shape[specAxis]];
	for(int z=0;z<shape[specAxis];z++){
	  start[specAxis] = end[specAxis] = z;
// 	  ASKAPLOG_DEBUG_STR(logger, "z="<<z<<", start="<<start<<", end="<<end);
	  casa::Slicer specslice(start,end,casa::Slicer::endIsLast);
// 	  ASKAPLOG_DEBUG_STR(logger, "specslice="<<specslice);
	  casa::Array<Float> channel = img.getSlice(specslice,True);
	  for(size_t y=0;y<ylen;y++){
	    for(size_t x=0;x<xlen;x++){
	      subcube[x+y*xlen+z*xlen*ylen] = channel(IPosition(2,x,y));
	    }
	  }
	}


	for(size_t y=0; y<ylen; y++){
	  outpos[1]=y;
	  for(size_t x=0; x<xlen; x++){

	    outpos[0]=x;

	    size_t pos=x+y*xlen;

	    if( pos % int(xlen*ylen*logevery/100.) == 0 )
	      ASKAPLOG_INFO_STR(logger, "Done " << pos << " spectra out of " << xlen*ylen <<" with x="<<x<<" and y="<<y);

	    if(subcube[pos]>1.e-20){
	      for (int i=0;i<ndata;i++){
		gsl_vector_set(ydat,i,log10(subcube[pos+i*xlen*ylen]));
	      }
	      gsl_multifit_linear_workspace * work = gsl_multifit_linear_alloc (ndata,degree);
	      gsl_multifit_wlinear (xdat, w, ydat, c, cov, &chisq, work);
	      gsl_multifit_linear_free (work);
	      
	      outputs[0](outpos) = pow(10.,gsl_vector_get(c,0));
	      outputs[1](outpos) = gsl_vector_get(c,1);
	      outputs[2](outpos) = gsl_vector_get(c,2);
	    }
	  }
	}

	bool OK = true;
	int rank;
	int version;
	LOFAR::BlobString bs;
	
	if (comms.isParallel()) {
	  do {
	    bs.resize(0);
	    comms.receiveBlob(bs, 0);
	    LOFAR::BlobIBufString bib(bs);
	    LOFAR::BlobIStream in(bib);
	    version = in.getStart("goInput");
	    ASKAPASSERT(version == 1);
	    in >> rank;
	    in.getEnd();
	    OK = (rank == comms.rank());
	  } while (!OK);
	}
	if(OK){
	  for(int t=0;t<nterms;t++){
	    
	    std::stringstream outname;
	    outname << modelimagebase <<".taylor." << t;
	    casa::PagedImage<float> outimg(outname.str());
	    casa::IPosition location(shape.size(),0);
	    location[0] = xmin;
	    location[1] = ymin;
	    ASKAPLOG_INFO_STR(logger, "Writing to CASA image " << outname.str() << " at location " << location);
	    outimg.putSlice(outputs[t], location);
	    
	  }
	  // Return the OK to the master to say that we've read the image
	  if (comms.isParallel()) {
	    bs.resize(0);
	    LOFAR::BlobOBufString bob(bs);
	    LOFAR::BlobOStream out(bob);
	    ASKAPLOG_DEBUG_STR(logger, "Worker #" << comms.rank() << ": Sending done message to Master.");
	    out.putStart("inputDone", 1);
	    out << OK;
	    out.putEnd();
	    comms.sendBlob(bs, 0);
	    ASKAPLOG_DEBUG_STR(logger, "Worker #" << comms.rank() << ": All done.");
	    
	  }
	}
      }
      
    } catch (const askap::AskapError& x) {
      ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
      std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
      exit(1);
    } catch (const duchamp::DuchampError& x) {
      ASKAPLOG_FATAL_STR(logger, "Duchamp error in " << argv[0] << ": " << x.what());
      std::cerr << "Duchamp error in " << argv[0] << ": " << x.what() << std::endl;
      exit(1);
    } catch (const std::exception& x) {
      ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
      std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
      exit(1);
    }
  
  return 0;
  
  
}
