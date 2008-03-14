/// @file
///
///
/// (c) 2008 ASKAP, All Rights Reserved
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/RadioSource.h>
#include <analysisutilities/AnalysisUtilities.h>

#include <duchamp/PixelMap/Voxel.hh>
#include <duchamp/PixelMap/Object2D.hh>
#include <duchamp/Cubes/cubes.hh>
#include <duchamp/Detection/detection.hh>

#include <scimath/Fitting/FitGaussian.h>
#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <scimath/Functionals/Gaussian3D.h>
#include <casa/namespace.h>

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <math.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".sourcefitting");

namespace askap
{

  namespace analysis
  {

    namespace sourcefitting
    {

      RadioSource::RadioSource(){itsNoiseLevel = -1.;};

      //    RadioSource::~RadioSource(){};


      bool RadioSource::setFluxArray(std::vector<PixelInfo::Voxel> *voxelList)
      {
	/// @details
	///
	/// This function takes a list of voxels, and defines an array
	/// of floats representing the fluxes of pixels surrounding the
	/// detected object. 
	///
	/// @return True if all necessary voxels were present. False otherwise. 

	long xmin = this->itsDetection->getXmin()-detectionBorder;
	long xmax = this->itsDetection->getXmax()+detectionBorder;
	long ymin = this->itsDetection->getYmin()-detectionBorder;
	long ymax = this->itsDetection->getYmax()+detectionBorder;
	long z = this->itsDetection->getZcentre();
	long xsize = xmax-xmin+1;
	long ysize = ymax-ymin+1;
	long size = xsize*ysize;

	float failure = false;

	if(z!=this->itsDetection->getZmin() || z != this->itsDetection->getZmax()){
	  ASKAPLOG_ERROR(logger,"Can only do fitting for two-dimensional objects!");
	  return failure;
	}
	else{

	  this->itsFluxArray = new float[size];

	  for(long x=xmin;x<=xmax && !failure;x++){
	    for(long y=ymin;y<=ymax && !failure;y++){
	      int i = (x-xmin) + (y-ymin)*xsize;
	      PixelInfo::Voxel tempvox(x,y,z,0.);
	      std::vector<PixelInfo::Voxel>::iterator vox = voxelList->begin();
	      while( !tempvox.match(*vox) && vox!=voxelList->end() ) vox++;
	      if(vox == voxelList->end()) failure = true;
	      else this->itsFluxArray[i] = vox->getF();
	  
	    }
	  }

	  if(failure){
	    delete this->itsFluxArray;
	    ASKAPLOG_ERROR(logger, "RadioSource: Failed to allocate flux array");
	  }

	}
      
	return !failure;

      }

      std::multimap<int,PixelInfo::Voxel> RadioSource::findDistinctPeaks()
      {

	/// @details
	///
	/// Find a list of local maxima in the detection. This divides
	/// the flux interval between the object's peak flux and the
	/// detection threshold into 10, and searches for objects at
	/// each of these sub-thresholds. Maxima other than the overall
	/// peak will appear at some thresholds but not others.
	///
	/// The list of peak locations is returned as a STL multimap,
	/// with the Key element being the number of times a peak
	/// location was found (the overall peak will be found 10
	/// times), and the Value element being the location of the
	/// peak, stored as a PixelInfo::Voxel.

	long xmin = this->itsDetection->getXmin()-detectionBorder;
	long xmax = this->itsDetection->getXmax()+detectionBorder;
	long ymin = this->itsDetection->getYmin()-detectionBorder;
	long ymax = this->itsDetection->getYmax()+detectionBorder;
	long xsize = xmax-xmin+1;
	long ysize = ymax-ymin+1;

	const int numThresh = 10;

	std::multimap<int,PixelInfo::Voxel> peakMap;
	std::multimap<int,PixelInfo::Voxel>::iterator pk;

	long dim[2]; dim[0]=xsize; dim[1]=ysize;
	duchamp::Image smlIm(dim);
	smlIm.saveArray(this->itsFluxArray,xsize*ysize);
	smlIm.setMinSize(1);

	for(int i=0;i<numThresh;i++){
	  float thresh = (this->itsDetection->getPeakFlux()-this->itsDetectionThreshold)*(i+1)/float(numThresh+1);
	  smlIm.stats().setThreshold(thresh);
	  std::vector<PixelInfo::Object2D> objlist = smlIm.lutz_detect();
	  std::vector<PixelInfo::Object2D>::iterator o;
	  for(o=objlist.begin();o<objlist.end();o++){
	    duchamp::Detection tempobj;
	    tempobj.pixels().addChannel(0,*o);
	    tempobj.calcFluxes(this->itsFluxArray,dim);
	    PixelInfo::Voxel peakLoc(tempobj.getXPeak()+xmin,tempobj.getYPeak()+ymin,
				     tempobj.getZPeak(),tempobj.getPeakFlux());
	    int freq = 1;
	    for(pk=peakMap.begin();pk!=peakMap.end();pk++){
	      if(pk->second==peakLoc){
		freq = pk->first + 1;
		peakMap.erase(pk);
	      }
	    }
	    peakMap.insert( std::pair<int,PixelInfo::Voxel>(freq,peakLoc) );
	  }

	}

	return peakMap;

      }
    
      /// @brief A simple way of printing fitted parameters
      void printparameters(Matrix<Double> &m)
      {
	cout.precision(3);
	cout.setf(ios::scientific);
	uInt g,p;
	for (g = 0; g < m.nrow(); g++)
	  {
	    for (p = 0; p < m.ncolumn() - 1; p++) cout << m(g,p) << ", ";
	    cout << m(g,p) << endl;
	    if (g < m.nrow() - 1) cout << "                    ";
	  }

      }


      bool RadioSource::fitGauss()
      {
	/// @details
	///
	/// This function fits a number of Gaussians to the
	/// Detection. All pixels within a box encompassing the
	/// detection plus the border given by detectionBorder are
	/// included in the fit.
 
	long xmin = this->itsDetection->getXmin()-detectionBorder;
	long xmax = this->itsDetection->getXmax()+detectionBorder;
	long ymin = this->itsDetection->getYmin()-detectionBorder;
	long ymax = this->itsDetection->getYmax()+detectionBorder;
	long xsize = xmax-xmin+1;
	long ysize = ymax-ymin+1;
	long size = xsize*ysize;

	float noise;
	if(this->itsNoiseLevel < 0){
	  noise = 1.;
	  ASKAPLOG_INFO_STR(logger, "Fitting: Noise level not defined. Not doing scaling.");
	}
	else{
	  noise = this->itsNoiseLevel;
	}

	casa::Matrix<casa::Double> pos;
	casa::Vector<casa::Double> f;
 	casa::Vector<casa::Double> sigma;
	pos.resize(xsize*ysize,2);
	f.resize(xsize*ysize);
	sigma.resize(xsize*ysize);
	casa::Vector<casa::Double> curpos(2);
	curpos=0;
	for(int x=xmin;x<=xmax;x++){
	  for(int y=ymin;y<=ymax;y++){
	    int i = (x-xmin) + (y-ymin)*xsize;
	    f(i) = this->itsFluxArray[i] // / noise
	      ;
 	    sigma(i) = noise;
	    curpos(0)=x;
	    curpos(1)=y;
	    pos.row(i)=curpos;
	  }
	}

	float boxFlux = 0.;
	for(int i=0;i<xsize*ysize;i++) boxFlux += f(i);
	float peakFlux = *std::max_element(this->itsFluxArray, this->itsFluxArray+xsize*ysize);
// 	peakFlux /= noise;

	std::multimap<int,PixelInfo::Voxel> peakList = this->findDistinctPeaks();
	std::cerr << peakList.size();
	std::multimap<int,PixelInfo::Voxel>::reverse_iterator pk;

	Double maxRMS = 5.;
	casa::Matrix<casa::Double> components;
	casa::Matrix<casa::Double> estimate;
	casa::Matrix<casa::Double> baseEstimate;
	casa::Matrix<casa::Double> retryfactors;
	casa::Matrix<casa::Double> baseRetryfactors;
	casa::Matrix<casa::Double> solution[4];

	baseEstimate.resize(1,6);
	baseEstimate(0,0)=this->itsDetection->getPeakFlux()//  / noise
	  ;   // height of Gaussian
	baseEstimate(0,1)=this->itsDetection->getXcentre();    // x centre
	baseEstimate(0,2)=this->itsDetection->getYcentre();    // y centre
	// get beam information from the FITSheader, if present.
	if(this->itsHeader->getBmajKeyword()>0){
	  baseEstimate(0,3)=this->itsHeader->getBmajKeyword()/this->itsHeader->getAvPixScale();
	  baseEstimate(0,4)=this->itsHeader->getBminKeyword()/this->itsHeader->getBmajKeyword();
	  baseEstimate(0,5)=this->itsHeader->getBpaKeyword();
	}
	else {
	  float xwidth=(this->itsDetection->getXmax()-this->itsDetection->getXmin() + 1)/2.;
	  float ywidth=(this->itsDetection->getYmax()-this->itsDetection->getYmin() + 1)/2.;
	  baseEstimate(0,3)=std::max(xwidth,ywidth);// x width (doesn't have to be x...)
	  baseEstimate(0,4)=std::min(xwidth,ywidth)/std::max(xwidth,ywidth); // axial ratio
	  baseEstimate(0,5)=0.;                  // position angle
	}

	baseRetryfactors.resize(1,6);
	baseRetryfactors(0,0) = 1.1; 
	baseRetryfactors(0,1) = 0.1; 
	baseRetryfactors(0,2) = 0.1;
	baseRetryfactors(0,3) = 1.1; 
	baseRetryfactors(0,4) = 1.01;
	baseRetryfactors(0,5) = M_PI/180.;

	float chisq[4];
	FitGaussian<casa::Double> fitgauss[4];
	bool fitIsGood = false;
	int bestFit = 0;
	float bestRChisq = 9999.;

	for(int ctr=0;ctr<4;ctr++){

	  uint numGauss = ctr + 1;
	  fitgauss[ctr].setDimensions(2);
	  fitgauss[ctr].setNumGaussians(numGauss);
    

	  estimate.resize(numGauss,6);
	  pk = peakList.rbegin();
	  for(uint g=0;g<numGauss;g++){
	    estimate(g,0) = baseEstimate(0,0);
	    if(g<peakList.size()){
	      estimate(g,1) = pk->second.getX();
	      estimate(g,2) = pk->second.getY();
	      pk++;
	    }
	    else{
	      estimate(g,1) = baseEstimate(0,1);
	      estimate(g,2) = baseEstimate(0,2);
	    }
	    for(int i=3;i<6;i++) estimate(g,i) = baseEstimate(0,i);
	  }
	  fitgauss[ctr].setFirstEstimate(estimate);
	  //	std::cerr << "First estimate of Parameters: "; printparameters(estimate);

	  retryfactors.resize(numGauss,6);
	  for(uint g=0;g<numGauss;g++)
	    for(int i=0;i<6;i++)
	      retryfactors(g,i) = baseRetryfactors(0,i);
	  fitgauss[ctr].setRetryFactors(retryfactors);
    
	  solution[ctr].resize();
	  bool thisFitGood = true;
	  try {
 	    solution[ctr] = fitgauss[ctr].fit(pos, f, maxRMS);
// 	    solution[ctr] = fitgauss[ctr].fit(pos, f, sigma, maxRMS);
	  } catch (AipsError err) {
	    std::string message = err.getMesg().chars();
	    message = "FIT ERROR: " + message;
	    ASKAPLOG_ERROR(logger, message);
	    thisFitGood = false;
	  }

	  chisq[ctr] = fitgauss[ctr].chisquared();
	  int ndof = size - numGauss*6 - 1;
	  float rchisq = chisq[ctr] / float(ndof);
	
	  cout.precision(6);
	  cout << "Solution Parameters: "; printparameters(solution[ctr]);
	  cout << "Num Gaussians = " << numGauss;
	  if( fitgauss[ctr].converged()) cout << ", Converged";
	  else cout << ", Failed";
	  cout << ", chisq = " << chisq[ctr]
	       << ", chisq/nu =  "  << rchisq
	       << ", dof = " << ndof
	       << ", RMS = " << fitgauss[ctr].RMS() << endl;

	  /// Acceptance criteria for a fit are as follows (after the
	  /// FIRST survey criteria, White et al 1997, ApJ 475, 479):
	  /// @li Fit must have converged
	  /// @li Fit must be acceptable according to its chisq value
	  /// @li The centre of each component must be inside the box
	  /// @li The separation between any pair of components must be more than 2 pixels.
	  /// @li The flux of each component must be positive and more than half the detection threshold
	  /// @li No component's peak flux can exceed twice the highest pixel in the box
	  /// @li The sum of the integrated fluxes of all components must not be more than twice the total flux in the box.

	  bool passConv, passChisq, passFlux, passXLoc, passYLoc, passSep, passIntFlux, passPeak;

	  passConv  = fitgauss[ctr].converged();
	  passConv  = passConv && (chisq[ctr]>0.);
	  for(uint i=0;i<numGauss;i++){
	    passConv = passConv && ( fabs(solution[ctr](i,5))<2.*M_PI );
	  }

	  //	  passChisq = rchisq < 50.;  // Replace with actual evaluation of chisq function?
	  passChisq = false;
	  passXLoc = passYLoc = passFlux = passSep = passPeak = passIntFlux = true;

	  if(passConv){

	    if(ndof<343)
	       passChisq = chisqProb(ndof,chisq[ctr]) > 0.01; // Test acceptance at 99% level
	    else 
	      passChisq = (rchisq < 1.2);
	    
	    float intFlux = 0.;
	    for(uint i=0;i<numGauss;i++){
	      passXLoc = passXLoc && (solution[ctr](i,1)>xmin) && (solution[ctr](i,1)<xmax);
	      passYLoc = passYLoc && (solution[ctr](i,2)>ymin) && (solution[ctr](i,2)<ymax);
	      passFlux = passFlux && (solution[ctr](i,0) > 0.);
	      passFlux = passFlux && (solution[ctr](i,0)// *noise
				      > 0.5*this->itsDetectionThreshold);
	      passPeak = passPeak && (solution[ctr](i,0) < 2.*peakFlux);	    
	      
	      Gaussian2D<Double> component(solution[ctr](i,0),solution[ctr](i,1),solution[ctr](i,2),
					   solution[ctr](i,3),solution[ctr](i,4),solution[ctr](i,5));
	      intFlux += component.flux();
	      
	      for(uint j=i+1;j<numGauss;j++){
		float sep = hypot( solution[ctr](i,1)-solution[ctr](j,1) , 
				   solution[ctr](i,2)-solution[ctr](j,2) );
		passSep = passSep && (sep > 2.);
	      }
	    }
	    
	    passIntFlux = (intFlux < 2.*boxFlux);

	  }

	  std::cout<<"Passes: "<<passConv<<passChisq<<passXLoc<<passYLoc<<passSep
		   <<passFlux<<passPeak<<passIntFlux<<"\n";;

	  thisFitGood = passConv && passChisq && passXLoc && passYLoc && passSep && 
	    passFlux && passPeak && passIntFlux;

	  if(thisFitGood){
	    if((ctr==0) || (rchisq < bestRChisq)){
	      fitIsGood = true;
	      bestFit = ctr;
	      bestRChisq = rchisq;
	    }
	  }

	} // end of 'ctr' for-loop

	if(fitIsGood){
	  for(int i=0;i<=bestFit;i++){
	    casa::Gaussian2D<casa::Double> 
	      gauss(solution[bestFit](i,0)// *noise
		    ,solution[bestFit](i,1),solution[bestFit](i,2),
		    solution[bestFit](i,3),solution[bestFit](i,4),solution[bestFit](i,5));
	    this->itsGaussFitSet.push_back(gauss);
	  }
	  cout << "BEST FIT: " << bestFit+1 << " Gaussians"
	       << ", chisq = " << bestRChisq * (size - 6*(bestFit+1) - 1)
	       << ", chisq/nu =  "  << bestRChisq << endl;
	}
	else{
	  cout << "No good fit found.\n";
	}

	return fitIsGood;

      }


      void RadioSource::printFit()
      {
	std::cout << "Fitted " << itsGaussFitSet.size() << " Gaussians\n";
	for(uint g=0;g<itsGaussFitSet.size();g++){
	  // 	itsGaussFitSet[g].parameters().print(std::cout);
	  // 	std::cout << "\n";
	  std::cout << itsGaussFitSet[g] << "\n";
	}

      }


      void RadioSource::writeFitToAnnotationFile(std::ostream &stream)
      {
	/// @details
	///
	/// This function writes the information about the fitted
	/// Gaussian components to a Karma annotation file. There are
	/// two different elements drawn for each RadioSource object.
	///
	/// For each fitted component, an ellipse is drawn indicating
	/// the size and orientation of the Gaussian. The central
	/// position is converted to world coordinates, and the major
	/// and minor axes are converted to elliptical
	/// semimajor/semiminor axes by halving and dividing by 2*ln(2).
	///
	/// Finally, a box is drawn around the detection, indicating the
	/// area used in the fitting. It includes the border around the
	/// detection fiven by sourcefitting::detectionBorder.

	std::vector<casa::Gaussian2D<Double> >::iterator fit;
      
	double *pix = new double[3];
	double *world = new double[3];
	pix[2] = 0.;

	for(fit=this->itsGaussFitSet.begin(); fit<this->itsGaussFitSet.end(); fit++){
	
	  pix[0] = fit->xCenter();
	  pix[1] = fit->yCenter();
	  this->itsHeader->pixToWCS(pix,world);
	
	  stream.precision(6);
	  stream << "ELLIPSE " 
		 << world[0] << " " 
		 << world[1] << " "
		 << fit->majorAxis() * this->itsHeader->getAvPixScale() / (2.*sqrt(2.*M_LN2)) << " "
		 << fit->minorAxis() * this->itsHeader->getAvPixScale() / (2.*sqrt(2.*M_LN2)) << " "
		 << fit->PA() * 180. / M_PI << "\n";
	  
	}

	pix[0] = this->itsDetection->getXmin()-sourcefitting::detectionBorder;
	pix[1] = this->itsDetection->getYmin()-sourcefitting::detectionBorder;
	this->itsHeader->pixToWCS(pix,world);
	stream << "BOX " << world[0] << " " << world[1] << " ";
	
	pix[0] = this->itsDetection->getXmax()+sourcefitting::detectionBorder;
	pix[1] = this->itsDetection->getYmax()+sourcefitting::detectionBorder;
	this->itsHeader->pixToWCS(pix,world);
	stream << world[0] << " " << world[1] << "\n";
	
      
      }


    }

  }

}
