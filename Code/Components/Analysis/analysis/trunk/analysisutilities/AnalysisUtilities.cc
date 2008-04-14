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

#define WCSLIB_GETWCSTAB // define this so that we don't try and redefine 
                         //  wtbarr (this is a problem when using gcc v.4+)
#include <fitsio.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".analysisutilities");

namespace askap
{
  namespace analysis
  {

    long * getFITSdimensions(std::string filename)
    {
      /// @details A simple function to open a FITS file and read the
      /// axis dimensions, returning the array of values.

      int numAxes, status = 0;  /* MUST initialize status */
      fitsfile *fptr;  
      long *dimAxes;

      // Open the FITS file
      status = 0;
      if( fits_open_file(&fptr,filename.c_str(),READONLY,&status) ){
	fits_report_error(stderr, status);
      }
      else{

	// Read the size of the FITS file -- number and sizes of the axes
	status = 0;
	if(fits_get_img_dim(fptr, &numAxes, &status)){
	  fits_report_error(stderr, status);
	}
	
	dimAxes = new long[numAxes];
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

      }
      return dimAxes;
      
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

      par.setNewFluxUnits( parset.getString("newFluxUnits", "") );

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

      for(int i=0;i<sectionlist.size();i++)
	std::cerr << sectionlist[i].getSection() << "\n";

      return sectionlist;

    }

    std::string getSubImageName(std::string image, int rank, int numWorkers)
    {
      
      std::stringstream file;
      bool isFits = (image.substr(image.size()-5,image.size())==".fits");
      if(isFits) file << image.substr(0,image.size()-5);
      else file << image;
      file << ".sub" << rank << "." << numWorkers;
      if(isFits) file << ".fits";
      return file.str();

    }


    std::vector<duchamp::Section> makeSubImages(int numWorkers, const LOFAR::ACC::APS::ParameterSet& parset)
    {

      std::vector<duchamp::Section> sectionlist; 
      std::string image = parset.getString("image");
      fitsfile *fin;
      int status=0;
      fits_open_file(&fin,image.c_str(),READONLY,&status);

      int nsubx = parset.getInt16("nsubx",1);
      int nsuby = parset.getInt16("nsuby",1);
      int nsubz = parset.getInt16("nsubz",1);
  
      int overlapx = parset.getInt16("overlapx",0);
      int overlapy = parset.getInt16("overlapy",0);
      int overlapz = parset.getInt16("overlapz",0);
  
      int numRequestedSubs = nsubx * nsuby * nsubz;
  
      if( numWorkers != numRequestedSubs ){
	ASKAPLOG_INFO_STR(logger, "Requested number of subsections ("<<numRequestedSubs
			  <<") doesn't match number of workers (" << numWorkers<<"). Not doing splitting.");
	return sectionlist;
      }  
      else {
    
	int numAxes;
	long *dimAxes;
	status = 0;
	if(fits_get_img_dim(fin, &numAxes, &status)){
	  fits_report_error(stderr, status);
	}
	
	dimAxes = new long[numAxes];
	for(int i=0;i<numAxes;i++) dimAxes[i]=1;
	status = 0;
	if(fits_get_img_size(fin, numAxes, dimAxes, &status)){
	  fits_report_error(stderr, status);
	}
	/// @todo Note that we are assuming a particular axis setup here. Make this more robust!
	long start = 0;

	for(int i=0;i<numWorkers;i++){

	  std::string subimage = "!"+getSubImageName(image,i,numWorkers);
	  
	  std::stringstream section;
	  //      section << "[";
	  if(nsubx>1){
	    int x1 = std::max( start , i*dimAxes[0]/nsubx - overlapx/2 );
	    int x2 = std::min( dimAxes[0] , (i+1)*dimAxes[0]/nsubx + overlapx/2 );
	    section << x1+1 << ":" << x2 << ",";
	  }
	  else section << "*,";
      
	  if(nsuby>1){
	    int y1 = std::max( start , i*dimAxes[1]/nsuby - overlapy/2 );
	    int y2 = std::min( dimAxes[1] , (i+1)*dimAxes[1]/nsuby + overlapy/2 );
	    section << y1+1 << ":" << y2 << "," ;
	  }
	  else section << "*,";
      
	  if(nsubz>1){
	    int z1 = std::max( start , i*dimAxes[2]/nsubz - overlapz/2 );
	    int z2 = std::min( dimAxes[2] , (i+1)*dimAxes[2]/nsubz + overlapz/2 );
	    section << z1+1 << ":" << z2;// << "]";
	  }
	  else section << "*";//]";
      
	  ASKAPLOG_INFO_STR(logger, "Worker #"<<i+1<<" is using subsection " << section.str());
      
	  fitsfile *fout;
	  status=0;
	  fits_create_file(&fout,subimage.c_str(),&status);
	  status=0;
	  ASKAPLOG_INFO_STR(logger, "Creating SubImage: " << subimage);
	  fits_copy_image_section(fin,fout,(char *)section.str().c_str(),&status);
	  status=0;
	  fits_close_file(fout,&status);

	  std::string secstring = "["+section.str()+"]";
	  duchamp::Section sec(secstring);
	  std::vector<long> dim(numAxes);
	  for(int i=0;i<numAxes;i++) dim[i] = dimAxes[i];
	  sec.parse(dim);
	  sectionlist.push_back(sec);
	}

	status=0;
	fits_close_file(fin,&status);

	return sectionlist;

      }
  


    }



  }
}
