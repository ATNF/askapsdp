/// @file
///
///
/// (c) 2008 ASKAP, All Rights Reserved
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///

#include <conrad/ConradLogging.h>
#include <conrad/ConradError.h>

#include <sourcefitting/RadioSource.h>

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

CONRAD_LOGGER(logger, ".sourcefitting");

namespace conrad
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
	CONRADLOG_ERROR(logger,"Can only do fitting for two-dimensional objects!");
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
	  CONRADLOG_ERROR(logger, "RadioSource: Failed to allocate flux array");
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
    
void printparameters(Matrix<Double> &m)
{
  cout.precision(6);
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
	CONRADLOG_INFO_STR(logger, "Fitting: Noise level not defined. Not doing scaling.");
      }
      else{
	noise = this->itsNoiseLevel;
      }

      casa::Matrix<casa::Double> pos;
      casa::Vector<casa::Double> f;
      pos.resize(xsize*ysize,2);
      f.resize(xsize*ysize);
      casa::Vector<casa::Double> curpos(2);
      curpos=0;
      for(int x=xmin;x<=xmax;x++){
	for(int y=ymin;y<=ymax;y++){
          int i = (x-xmin) + (y-ymin)*xsize;
	  f(i) = this->itsFluxArray[i] / noise;
	  curpos(0)=x;
	  curpos(1)=y;
	  pos.row(i)=curpos;
	}
      }

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
      baseEstimate(0,0)=this->itsDetection->getPeakFlux() / noise;   // height of Gaussian
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
      baseRetryfactors(0,4) = 1.1;
      baseRetryfactors(0,5) = M_PI/180.;

      float chisq[4];
      FitGaussian<casa::Double> fitgauss[4];
      bool fitIsGood = false;
      int bestFit = 0;
      float bestRChisq = 9999.;

      for(int ctr=0;ctr<4;ctr++){

	int numGauss = ctr + 1;
	fitgauss[ctr].setDimensions(2);
	fitgauss[ctr].setNumGaussians(numGauss);
    

	estimate.resize(numGauss,6);
	pk = peakList.rbegin();
	for(int g=0;g<numGauss;g++){
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
	for(int g=0;g<numGauss;g++)
	  for(int i=0;i<6;i++)
	    retryfactors(g,i) = baseRetryfactors(0,i);
	fitgauss[ctr].setRetryFactors(retryfactors);
    
	solution[ctr].resize();
	bool thisFitGood = true;
	try {
	  solution[ctr] = fitgauss[ctr].fit(pos, f, maxRMS);
	} catch (AipsError err) {
	  std::string message = err.getMesg().chars();
	  message = "FIT ERROR: " + message;
	  CONRADLOG_ERROR(logger, message);
	  thisFitGood = false;
	}

	chisq[ctr] = fitgauss[ctr].chisquared();
	float rchisq = chisq[ctr] / float(size - numGauss*6 - 1);
	
//  	cout << "Solution Parameters: "; printparameters(solution[ctr]);
// 	cout << "chisq = " << chisq[ctr]
// 	     << ", chisq/nu =  "  << rchisq
// 	     << ", RMS = " << fitgauss[ctr].RMS() << endl;
	
	thisFitGood = fitgauss[ctr].converged() && (rchisq < 50.);
	for(int i=0;i<numGauss;i++){
	  thisFitGood = thisFitGood && (solution[ctr](i,1)>xmin) && (solution[ctr](i,1)<xmax);
	  thisFitGood = thisFitGood && (solution[ctr](i,2)>ymin) && (solution[ctr](i,2)<ymax);
	  thisFitGood = thisFitGood && (solution[ctr](i,0) > 0.5*this->itsDetectionThreshold/noise);
	}
	
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
	    gauss(solution[bestFit](i,0)*noise,solution[bestFit](i,1),solution[bestFit](i,2),
		  solution[bestFit](i,3),solution[bestFit](i,4),solution[bestFit](i,5));
	  this->itsGaussFitSet.push_back(gauss);
	}
      }

      return fitIsGood;

    }


    void RadioSource::printFit()
    {
      std::cout << "Fitted " << itsGaussFitSet.size() << " Gaussians\n";
      for(int g=0;g<itsGaussFitSet.size();g++){
// 	itsGaussFitSet[g].parameters().print(std::cout);
// 	std::cout << "\n";
	std::cout << itsGaussFitSet[g] << "\n";
      }

    }


  }

}
