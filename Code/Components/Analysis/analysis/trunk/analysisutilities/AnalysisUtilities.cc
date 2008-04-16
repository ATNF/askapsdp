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
	std::sort(arrayCopy,arrayCopy+size);
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
     
      /// @details A standard way for producing the filename for a
      /// subimage, coded according to the number of workers and the
      /// rank of the particular worker in question. If the input
      /// image is image.fits, then for the first worker (rank=0) out
      /// of 5, the filename returned will be image.sub0.5.fits. If
      /// the input image does not end in ".fits", the sub0.5 is
      /// appended: e.g image.fts.sub0.5
      /// @param image The existing disk image
      /// @param rank The rank of the worker desiring the subimage
      /// @param numWorkers The total number of workers (ie. total
      /// number of subimages.)
      /// @return The filename for the subimage.
 
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

      /// @details This function takes an existing FITS image on disk,
      /// and creates a number of subimages, one for each worker. The
      /// division of the image is governed by the parameter set,
      /// specifically the nsubx/y/z and overlapx/y/z parameters. The
      /// image to be split is given by the image parameter. The new
      /// files are always written, and will overwrite any
      /// pre-existing files (by using a "!" at the start of the
      /// filename).
      ///
      /// The section strings are stored in duchamp::Section objects,
      /// and are parsed so that they can provide offsets etc. These
      /// are stored in a std::vector, and returned by the fuction
      /// upon completion.
      ///
      /// @param numWorkers The number of workers and the number of subimages. 
      /// @param parset The parameter set holding info on how to divide the image.
      /// @return A std::vector of duchamp::Section objects.

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

	duchamp::Param tempPar; // This is needed for defineWCS(), but we don't care about the contents.
	duchamp::FitsHeader imageHeader;
	imageHeader.defineWCS(image,tempPar);
	int naxis = imageHeader.WCS().naxis;
	const int lng = imageHeader.WCS().lng;
	const int lat = imageHeader.WCS().lat;
	const int spec = imageHeader.WCS().spec;

	int *nsub = new int[naxis];
	int *overlap = new int[naxis];
	for(int i=0;i<naxis;i++){
	  if(i==lng){ 
	    nsub[i]=nsubx; 
	    overlap[i]=overlapx; 
	  }
	  else if(i==lat){
	    nsub[i]=nsuby; 
	    overlap[i]=overlapy;
	  }
	  else if(i==spec){
	    nsub[i]=nsubz;
	    overlap[i]=overlapz; 
	  }
	  else{
	    nsub[i] = 1; 
	    overlap[i] = 0;
	  }
	}

	long *dimAxes = new long[naxis];
	for(int i=0;i<naxis;i++) dimAxes[i]=1;
	status = 0;
	if(fits_get_img_size(fin, naxis, dimAxes, &status)){
	  fits_report_error(stderr, status);
	}
	/// @todo Note that we are assuming a particular axis setup here. Make this more robust!
	long start = 0;

	for(int w=0;w<numWorkers;w++){

	  std::string subimage = "!"+getSubImageName(image,w,numWorkers);
	  
	  std::stringstream section;

	  for(int i=0;i<naxis;i++){
	    
	    if(nsub[i] > 1){
	      int min = std::max( start, w*dimAxes[i]/nsub[i] - overlap[i]/2 ) + 1;
	      int max = std::min( dimAxes[i], (w+1)*dimAxes[i]/nsub[i] + overlap[i]/2 );
	      section << min << ":" << max;
	    }
	    else section << "*";
	    if(i != naxis-1) section << ",";

	  }
      
	  ASKAPLOG_INFO_STR(logger, "Worker #"<<w+1<<" is using subsection " << section.str());
      
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
	  std::vector<long> dim(naxis);
	  for(int i=0;i<naxis;i++) dim[i] = dimAxes[i];
 	  sec.parse(dim);
	  sectionlist.push_back(sec);
	}

	status=0;
	fits_close_file(fin,&status);

	delete [] nsub;
	delete [] overlap;
	delete [] dimAxes;

	return sectionlist;

      }
  


    }



  }
}
