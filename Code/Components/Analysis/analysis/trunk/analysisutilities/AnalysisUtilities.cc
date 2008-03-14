/// @file
///
/// @brief General utility functions to support the analysis software
/// @details
/// These functions are unattached to any classes, but provide simple
/// support for the rest of the analysis package.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
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
#include <vector>
#include <string>

#include <duchamp/Utils/Statistics.hh>
#include <duchamp/Utils/Section.hh>
#include <duchamp/param.hh>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".analysisutilities");

namespace askap
{
  namespace analysis
  {

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

      par.setPixelCentre( parset.getString("pixelCentre", "centroid") );

      par.setCut( parset.getFloat("snrCut", 4.) );
      par.setMinPix( parset.getInt16("minPix", par.getMinPix()) );
      float threshold = parset.getFloat("threshold", -99999.9);
      if(threshold < -99999.){ // if "threshold" was not in the parset
	par.setFlagUserThreshold(false);
      }
      else{
	par.setFlagUserThreshold(true);
	par.setThreshold(threshold);
	ASKAPLOG_INFO_STR(logger, "Setting threshold to " << threshold << ".");
      }

      par.setFlagKarma( parset.getBool("flagKarma", true) );

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


    std::vector<duchamp::Section> readSectionInfo(std::string filename)
    {
      /// @details
      /// Record the section information that details what pixels are
      /// covered by each of the distributed images/cubes. This is
      /// designed for the case where the data to be searched is
      /// spread over a number of data files (potentially on a number
      /// of nodes).
      ///
      /// The information is read from a "sectionInfo" file that has the following format:
      /// @li Number of axes
      /// @li Dimension of axis 1
      /// @li Dimension of axis 2
      /// @li ... [repeat for all axes]
      /// @li Pixel section for image 1, e.g. [a:b,c:d,e:f] or [*,*,a:b]
      /// @li Pixel section for image 2
      /// @li ... [repeat for all images -- typically one image per node]
      /// 
      /// The pixel sections are parsed by the duchamp::Section class.
      ///
      /// @param filename The name of the sectionInfo file.
      /// @return A std::vector containing a duchamp::Section object for each image.
      std::vector<duchamp::Section> sectionlist; 
      std::ifstream fin(filename.c_str());
      int numAxes=0;
      if(!fin.is_open()) 
	ASKAPLOG_ERROR_STR(logger, "SectionInfo file " << filename.c_str() << " not found!"); 
      else{	
	fin >> numAxes;	
	std::vector<long> dimAxes(numAxes);	
	for(int i=0;i<numAxes;i++) fin>>dimAxes[i];	
	while(!fin.eof()){
	  std::string image,sectionString;
	  fin >> image >> sectionString;
	  if(!fin.eof()){
	    duchamp::Section section(sectionString);
	    section.parse(dimAxes);
	    sectionlist.push_back(section);
	  }
	}
	fin.close();
      }

      return sectionlist;

    }


  }
}
