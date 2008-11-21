/// @file
///
/// @brief General utility functions to support the analysis software
/// @details
/// These functions are unattached to any classes, but provide simple
/// support for the rest of the analysis package.
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
/// 

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <askapparallel/AskapParallel.h>

#include <analysisutilities/AnalysisUtilities.h>

#include <gsl/gsl_sf_gamma.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <duchamp/fitsHeader.hh>
#include <duchamp/Utils/Statistics.hh>
#include <duchamp/Utils/Section.hh>
#include <duchamp/param.hh>

#define WCSLIB_GETWCSTAB // define this so that we don't try and redefine 
                         //  wtbarr (this is a problem when using gcc v.4+)
#include <fitsio.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".analysisutilities");

namespace askap
{
  namespace analysis
  {

    std::vector<long> getFITSdimensions(std::string filename)
    {
      /// @details A simple function to open a FITS file and read the
      /// axis dimensions, returning the array of values.

      int numAxes, status = 0;  /* MUST initialize status */
      fitsfile *fptr;  
      std::vector<long> dim;

      // Open the FITS file
      status = 0;
      if( fits_open_file(&fptr,filename.c_str(),READONLY,&status) ){
	fits_report_error(stderr, status);
	ASKAPTHROW(AskapError, "FITS Error opening file")
      }
      else{

	// Read the size of the FITS file -- number and sizes of the axes
	status = 0;
	if(fits_get_img_dim(fptr, &numAxes, &status)){
	  fits_report_error(stderr, status);
	}
	
	long *dimAxes = new long[numAxes];
	for(int i=0;i<numAxes;i++) dimAxes[i]=1;
	status = 0;
	if(fits_get_img_size(fptr, numAxes, dimAxes, &status)){
	  fits_report_error(stderr, status);
	}
	
	// Close the FITS file -- not needed any more in this function.
	status = 0;
	fits_close_file(fptr, &status);
	if (status){
	  fits_report_error(stderr, status);
	}

	dim = std::vector<long>(numAxes);
	for(int i=0;i<numAxes;i++) dim[i] = dimAxes[i];
	
	delete [] dimAxes;

       std::stringstream ss;
      for(int i=0;i<numAxes-1;i++) ss << dimAxes[i] << "x";
      ss<<dimAxes[numAxes-1];
      ASKAPLOG_DEBUG_STR(logger,"Fits dimensions = "<<ss.str());

      }

 
      return dim;
      
    }

    float chisqProb(float ndof, float chisq)
    {
      /// @details Returns the probability of exceeding the given
      /// value of chisq by chance. If it comes from a fit, this
      /// probability is assuming the fit is valid.
      ///
      /// Typical use: say you have a fit with ndof=5 degrees of
      /// freedom that gives a chisq value of 12. You call this
      /// function via chisqProb(5,12.), which will return
      /// 0.0347878. If your confidence limit is 95% (ie. you can
      /// tolerate a 1-in-20 chance that a valid fit will produce a
      /// chisq value that high), you would reject that fit (since
      /// 0.0347878 < 0.05), but if it is 99%, you would accept it
      /// (since 0.0347878 > 0.01).

      return gsl_sf_gamma_inc(ndof/2.,chisq/2.)/gsl_sf_gamma(ndof/2.);

    }

    duchamp::Param parseParset(const LOFAR::ACC::APS::ParameterSet& parset)
    {
      /// @details 
      /// Takes a ParameterSet and reads in the necessary Duchamp
      /// parameters. It checks many of the duchamp::param parameters,
      /// and if they are not present, a default value, defined
      /// herein, is set (note that this is not necessarily the
      /// standard Duchamp default value).
      /// 
      /// The excpetions are the image names, as these will in general
      /// depend on the node and on whether the current node is a
      /// master or worker. These should be set by the calling
      /// function.

      duchamp::Param par;

      std::string outputfile;
      outputfile = parset.getString("outfile", "" );
      if(outputfile == "") outputfile =  parset.getString("resultsFile", "");
      if(outputfile != "") par.setOutFile(outputfile);

      par.setFlagSubsection( parset.getBool("flagSubsection", false) );
      par.setSubsection( parset.getString("subsection", "") );
      par.setFlagStatSec( parset.getBool("flagStatSec", false) );
      par.setStatSec( parset.getString("statsec", par.getStatSec()) );
      
      par.setVerbosity( parset.getBool("verbose", false) );
      par.setFlagLog(true);

      par.setPixelCentre( parset.getString("pixelCentre", "centroid") );

      par.setCut( parset.getFloat("snrCut", 4.) );
      if(parset.isDefined("threshold")){
	par.setFlagUserThreshold(true);
	par.setThreshold(parset.getFloat("threshold"));
	// 	ASKAPLOG_INFO_STR(logger, "Setting threshold to " << threshold << ".");
      }
      else {
	par.setFlagUserThreshold(false);
      }

      par.setFlagAdjacent( parset.getBool("flagAdjacent", par.getFlagAdjacent()) );
      par.setThreshS( parset.getFloat("threshSpatial", par.getThreshS()) );
      par.setThreshV( parset.getFloat("threshVelocity", par.getThreshV()) );
      par.setMinPix( parset.getInt16("minPix", par.getMinPix()) );
      par.setMinChannels( parset.getInt16("minChannels", par.getMinChannels()) );

      par.setFlagKarma( parset.getBool("flagKarma", true) );

      par.setNewFluxUnits( parset.getString("newFluxUnits", "") );

      par.setFlagGrowth( parset.getBool("flagGrowth", false) );
      par.setGrowthCut( parset.getFloat("growthCut", par.getGrowthCut()) );
      if(parset.isDefined("growthThreshold")){
	par.setGrowthThreshold( parset.getFloat("growthThreshold") );
	par.setFlagUserGrowthThreshold(true);
      }

      par.setFlagATrous( parset.getBool("flagATrous",false) );
      par.setReconDim( parset.getInt16("reconDim", par.getReconDim()) );
      par.setMinScale( parset.getInt16("scaleMin", par.getMinScale()) );
      par.setMaxScale( parset.getInt16("scaleMax", par.getMaxScale()) );
      par.setAtrousCut( parset.getFloat("snrRecon", par.getAtrousCut()) );
      par.setFilterCode( parset.getInt16("filterCode", par.getFilterCode()) );
      par.filter().define(par.getFilterCode());

      if(par.getFlagATrous()) par.setFlagSmooth(false);
      else par.setFlagSmooth( parset.getBool("flagSmooth",false) );
      par.setSmoothType( parset.getString("smoothType", par.getSmoothType()) );
      par.setHanningWidth( parset.getInt16("hanningWidth", par.getHanningWidth()) );
      par.setKernMaj( parset.getFloat("kernMaj", par.getKernMaj()) );
      par.setKernMin( parset.getFloat("kernMin", par.getKernMin()) );
      par.setKernPA( parset.getFloat("kernPA", par.getKernPA()) );
      
      par.checkPars();

      return par;
    }
    
    double findSpread(bool robust, double middle, int size, float *array)
    {
      /// @details
      /// Finds the "spread" (ie. the rms or standard deviation) of an
      /// array of values using a given mean value. The option exists
      /// to use the standard deviation, or, by setting robust=true,
      /// the median absolute deviation from the median. In the latter
      /// case, the middle value given is assumed to be the median,
      /// and the returned value is the median absolute difference of
      /// the data values from the median.

      double spread=0.;
      if(robust){
	float *arrayCopy = new float[size];
	for(int i=0;i<size;i++) arrayCopy[i] = fabs(array[i]-middle);
	std::sort(arrayCopy,arrayCopy+size);
	if((size%2)==0) spread = (arrayCopy[size/2-1]+arrayCopy[size/2])/2;
	else spread = arrayCopy[size/2];
	delete [] arrayCopy;
	spread = Statistics::madfmToSigma(spread);
      }
      else{
	for(int i=0;i<size;i++) spread += (array[i]-middle)*(array[i]-middle);
	spread = sqrt(spread / double(size-1));
      }
      return spread;
    }


    double findSpread(bool robust, double middle, int size, float *array, bool *mask)
    {
      /// @details
      /// Finds the "spread" (ie. the rms or standard deviation) of an
      /// array of values using a given mean value. The option exists
      /// to use the standard deviation, or, by setting robust=true,
      /// the median absolute deviation from the median. In the latter
      /// case, the middle value given is assumed to be the median,
      /// and the returned value is the median absolute difference of
      /// the data values from the median.

      int goodSize=0;
      for(int i=0;i<size;i++) if(mask[i]) goodSize++;
      double spread=0.;
      if(robust){
	float *arrayCopy = new float[goodSize];
	int j=0;
	for(int i=0;i<size;i++) if(mask[i]) arrayCopy[j++] = fabs(array[i]-middle);
	std::sort(arrayCopy,arrayCopy+goodSize);
	if((goodSize%2)==0) spread = (arrayCopy[goodSize/2-1]+arrayCopy[goodSize/2])/2;
	else spread = arrayCopy[goodSize/2];
	delete [] arrayCopy;
	spread = Statistics::madfmToSigma(spread);
      }
      else{
	for(int i=0;i<size;i++) if(mask[i]) spread += (array[i]-middle)*(array[i]-middle);
	spread = sqrt(spread / double(goodSize-1));
      }
      return spread;
    }



 
  }
}
