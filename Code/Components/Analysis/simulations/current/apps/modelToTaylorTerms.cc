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

#include <Common/ParameterSet.h>
#include <casa/OS/Timer.h>
#include <casa/namespace.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>
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
      if(comms.isParallel() && comms.isMaster()){
	ASKAPLOG_INFO_STR(logger, "On master, so not doing anything");
      }
      else{

	std::string parsetFile(getInputs("-inputs", "createFITS.in", argc, argv));
	ASKAPLOG_INFO_STR(logger,  "parset file " << parsetFile);
	LOFAR::ParameterSet parset(parsetFile);
	LOFAR::ParameterSet subset(parset.makeSubset("model2TT."));

	std::string modelimage=subset.getString("inputmodel");
	int nsubx=subset.getInt16("nsubx",1);
	int nsuby=subset.getInt16("nsuby",1);
	const int nterms=3;
	int logevery = subset.getInt16("logevery",10);

	struct wcsprm *wcs = analysis::casaImageToWCS(modelimage);      

	ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
	ImageOpener::registerOpenImageFunction(ImageOpener::MIRIAD, MIRIADImage::openMIRIADImage);
	const LatticeBase* lattPtr = ImageOpener::openImage(modelimage);
	if (lattPtr == 0)
	  ASKAPTHROW(AskapError, "Requested image \"" << modelimage << "\" does not exist or could not be opened.");
	const ImageInterface<Float>* imagePtr = dynamic_cast<const ImageInterface<Float>*>(lattPtr);
	IPosition shape = imagePtr->shape();

	int nx,ny,xmin,xmax,ymin,ymax;
	if(comms.isParallel()){
	  nx = (comms.rank()-1) % nsubx;
	  ny = (comms.rank()-1) / nsubx;
	  xmin = int( nx * float(shape[wcs->lng])/float(nsubx) );
	  xmax = int( (nx+1) * float(shape[wcs->lng])/float(nsubx) )-1;
	  ymin = int( ny * float(shape[wcs->lat])/float(nsuby) );
	  ymax = int( (ny+1) * float(shape[wcs->lat])/float(nsuby) )-1;
	}
	else{ // if serial mode, use the full range of x & y
	  xmin=ymin=0;
	  xmax=shape[wcs->lng];
	  ymax=shape[wcs->lat];
	}

	casa::Array<Float> outputs[nterms];
	casa::IPosition outshape(shape);
	outshape[wcs->spec] = 1;
	for(int i=0;i<nterms;i++){
	  outputs[i] = casa::Array<Float>(outshape,0.);
	}

	casa::IPosition start(shape.size(),0);
	casa::IPosition end(shape-1);      

	const int ndata=shape[wcs->spec];
	double chisq;
	gsl_matrix *xdat, *cov;
	gsl_vector *ydat, *w, *c;
	xdat = gsl_matrix_alloc(ndata,nterms);
	ydat = gsl_vector_alloc(ndata);
	w = gsl_vector_alloc(ndata);
	c = gsl_vector_alloc(nterms);
	cov = gsl_matrix_alloc(nterms,nterms);
      
	for(int i=0;i<ndata;i++){
	  float logfreq=log10((wcs->crval[wcs->spec] + (i-wcs->crpix[wcs->spec])*wcs->cdelt[wcs->spec])/wcs->crval[wcs->spec]);
	  gsl_matrix_set(xdat,i,0,1.);
	  gsl_matrix_set(xdat,i,1,logfreq);
	  gsl_matrix_set(xdat,i,2,logfreq*logfreq);
	  gsl_matrix_set(xdat,i,3,logfreq*logfreq*logfreq);
	  gsl_matrix_set(xdat,i,4,logfreq*logfreq*logfreq*logfreq);
	  gsl_vector_set(w,i,1.);
	}

	for(int y=ymin; y<ymax; y++){
	  for(int x=xmin; x<xmax; x++){

	    // LOOP OVER Y AND X
	    // EXTRACT SPECTRUM FROM MODEL IMAGE
	    // FIT TO SPECTRUM
	    // STORE FIT RESULTS IN OUTPUT ARRAYS

	    if( (x+y*(xmax-xmin+1)) % (xmax-xmin+1)*(ymax-ymin+1)/logevery == 0 )
	      ASKAPLOG_INFO_STR(logger, "Done " << x+y*(xmax-xmin+1) << " spectra out of " << (xmax-xmin+1)*(ymax-ymin+1) <<" with x="<<x<<" and y="<<y);

	    start[wcs->lng]=end[wcs->lng]=x;
	    start[wcs->lat]=end[wcs->lat]=y;

	    casa::Array<Float> spectrum=imagePtr->getSlice(start,end);
	    casa::Array<Float>::iterator iterSpec=spectrum.begin();
	    for (int i=0;i<ndata;i++){
	      gsl_vector_set(ydat,i,log10(double(*iterSpec++)));
	    }
	    gsl_multifit_linear_workspace * work = gsl_multifit_linear_alloc (ndata,nterms);
	    gsl_multifit_wlinear (xdat, w, ydat, c, cov, &chisq, work);
	    gsl_multifit_linear_free (work);
	  
	    casa::IPosition outpos(2,x,y);
	    outputs[0](outpos) = pow(10.,gsl_vector_get(c,0));
	    outputs[1](outpos) = gsl_vector_get(c,1);
	    outputs[2](outpos) = gsl_vector_get(c,2);

	  }
	}

	Unit bunit=imagePtr->units();
	casa::Vector<casa::Quantum<Double> > beam = imagePtr->imageInfo().restoringBeam();
      
	for(int t=0;t<nterms;t++){
	  std::stringstream name;
	  name << modelimage << "_w"<<comms.rank()-1<<".taylor." << t;

	  casa::CoordinateSystem csys = analysis::wcsToCASAcoord(wcs,1);
	  casa::IPosition tileshape(shape.size(),1);
	  tileshape(wcs->lng) = std::min(128L,shape(wcs->lng));
	  tileshape(wcs->lat) = std::min(128L,shape(wcs->lat));
	  shape(wcs->spec)=1;
	
	  ASKAPLOG_INFO_STR(logger, "Creating a new CASA image " << name.str() << " with the shape " << shape << " and tileshape " << tileshape);
	  casa::PagedImage<float> img(casa::TiledShape(shape,tileshape), csys, name.str());
	
	  img.setUnits(bunit);
	  casa::ImageInfo ii = img.imageInfo();
	  ii.setRestoringBeam(beam);
	  img.setImageInfo(ii);

	  casa::IPosition location(shape.size(),0);
	  img.putSlice(outputs[t], location);
	
	}
      }

    } catch (const askap::AskapError& x) {
    ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
    std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  } catch (const std::exception& x) {
    ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
    std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }

  return 0;


}
