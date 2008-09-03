/// @file
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/RadioSource.h>
#include <sourcefitting/Fitter.h>
#include <sourcefitting/Component.h>
#include <analysisutilities/AnalysisUtilities.h>
#include <evaluationutilities/EvaluationUtilities.h>

#include <duchamp/fitsHeader.hh>
#include <duchamp/PixelMap/Voxel.hh>
#include <duchamp/PixelMap/Object2D.hh>
#include <duchamp/PixelMap/Object3D.hh>
#include <duchamp/Cubes/cubes.hh>
#include <duchamp/Detection/detection.hh>
#include <duchamp/Detection/columns.hh>

#include <scimath/Fitting/FitGaussian.h>
#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <scimath/Functionals/Gaussian3D.h>
#include <casa/namespace.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <utility>
#include <math.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".sourcefitting");

using namespace duchamp;

namespace askap
{

  namespace analysis
  {

    namespace sourcefitting
    {

      RadioSource::RadioSource():
	duchamp::Detection()
      {
	this->hasFit = false;
	this->atEdge = false;
	this->itsNoiseLevel = 1.;
      }

      RadioSource::RadioSource(duchamp::Detection obj):
	duchamp::Detection(obj)
      {
	this->hasFit = false;
	this->atEdge = false;
	this->itsNoiseLevel = 1.;
      }

      RadioSource::RadioSource(const RadioSource& src):
	duchamp::Detection(src)
      {
	this->atEdge = src.atEdge;
	this->hasFit = src.hasFit;
	this->itsNoiseLevel = src.itsNoiseLevel;
	this->itsDetectionThreshold = src.itsDetectionThreshold;
	this->itsHeader = src.itsHeader;
	this->itsGaussFitSet = src.itsGaussFitSet;
	this->itsBoxMargins = src.itsBoxMargins;
      }

      //**************************************************************//

      RadioSource& RadioSource::operator= (const RadioSource& src)
      {
	((duchamp::Detection &) *this) = src;
	this->atEdge = src.atEdge;
	this->hasFit = src.hasFit;
	this->itsNoiseLevel = src.itsNoiseLevel;
	this->itsDetectionThreshold = src.itsDetectionThreshold;
	this->itsHeader = src.itsHeader;
	this->itsGaussFitSet = src.itsGaussFitSet;
	this->itsBoxMargins = src.itsBoxMargins;
	return *this;
      }

      //**************************************************************//

      void RadioSource::defineBox(long *axes)
      {
	/// @details Defines the maximum and minimum points of the box
	/// in each axis direction. The size of the image array is
	/// taken into account, using the axes array, so that the box
	/// does not go outside the allowed pixel area.

	this->itsBoxMargins.clear();
	long zero = 0;
	long xmin = std::max(zero, this->getXmin() - detectionBorder);
	long xmax = std::min(axes[0], this->getXmax() + detectionBorder);
 	long ymin = std::max(zero, this->getYmin() - detectionBorder);
 	long ymax = std::min(axes[1], this->getYmax() + detectionBorder);
 	long zmin = std::max(zero, this->getZmin() - detectionBorder);
 	long zmax = std::min(axes[2], this->getZmax() + detectionBorder);
	std::vector<std::pair<long,long> > vec(3);
	vec[0] = std::pair<long,long>(xmin,xmax);
	vec[1] = std::pair<long,long>(ymin,ymax);
	vec[2] = std::pair<long,long>(zmin,zmax);
	this->itsBoxMargins = vec;

      }


      //**************************************************************//

      void RadioSource::setAtEdge(duchamp::Cube &cube)
      {
	/// @details Sets the atEdge flag based on the dimensions of
	/// the cube and the duchamp parameters flagAdjacent, threshS
	/// and threshV. If flagAdjacent is true, then the source is
	/// at the edge if it occupies a pixel on the boundary of the
	/// image (the z-direction is only examined if there is more
	/// than one channel). If flagAdjacent, the source must lie
	/// within the appropriate threshold (threshS for the spatial
	/// directions and threshV for the spectral/velocity) of the
	/// image boundary.

	bool flagBoundary=false;
	bool flagAdj = cube.pars().getFlagAdjacent();
	float threshS = cube.pars().getThreshS();
	float threshV = cube.pars().getThreshV();
	if(flagAdj){
	  flagBoundary = flagBoundary || ( this->getXmin()==0 );
	  flagBoundary = flagBoundary || ( this->getXmax()==cube.getDimX()-1 ); 
	  flagBoundary = flagBoundary || ( this->getYmin()==0 );
	  flagBoundary = flagBoundary || ( this->getYmax()==cube.getDimY()-1 ); 
	  if(cube.getDimZ()>1){
	    flagBoundary = flagBoundary || ( this->getZmin()==0 );
	    flagBoundary = flagBoundary || ( this->getZmax()==cube.getDimZ()-1 ); 
	  }
	}
	else{
	  flagBoundary = flagBoundary || ( this->getXmin()<threshS );
	  flagBoundary = flagBoundary || ( (cube.getDimX()-this->getXmax())<threshS ); 
	  flagBoundary = flagBoundary || ( this->getYmin()<threshS );
	  flagBoundary = flagBoundary || ( (cube.getDimY()-this->getYmax())<threshS ); 
	  flagBoundary = flagBoundary || ( this->getZmin()<threshV );
	  flagBoundary = flagBoundary || ( (cube.getDimZ()-this->getZmax())<threshV ); 
	}
	
	this->atEdge = flagBoundary;

      }
      //**************************************************************//

      void RadioSource::setNoiseLevel(duchamp::Cube &cube, int boxSize)
      {
	/// @details Sets the value of the local noise level by taking
	/// the MADFM of the surrounding pixels from the Cube's array. 
	/// Calls setNoiseLevel(float *, long *, int).
	/// @param cube The duchamp::Cube object containing the pixel array
	/// @param boxSize The side length of the box used.

	this->setNoiseLevel(cube.getArray(), cube.getDimArray(), boxSize);
      }

      void RadioSource::setNoiseLevel(float *array, long *dim, int boxSize)
      {
	/// @details Sets the value of the local noise level by taking
	/// the MADFM of the surrounding pixels from the Cube's array.
	/// A box of side length boxSize is centred on the peak pixel
	/// of the detection, and the MADFM of the pixels therein is
	/// found. This is converted to a Gaussian rms, and stored as
	/// the RadioSource::itsNoiseLevel value.
	/// @param array Array of pixel values
	/// @param dim Set of dimensions for array
	/// @param boxSize The side length of the box used.

	int hw = boxSize/2;
	
	float *localArray = new float[boxSize*boxSize];
	
	long xmin = max(0,this->xpeak-hw);
	long ymin = max(0,this->ypeak-hw);
	long xmax = min(dim[0]-1,this->xpeak+hw);
	long ymax = min(dim[1]-1,this->ypeak+hw);
	int size=0;
	for(int x=xmin;x<=xmax;x++){
	  for(int y=ymin;y<=ymax;y++){
	    int pos = x + y*dim[0];
	    localArray[size++] = array[pos];
	  }
	}
	std::sort(localArray,localArray+size);
	
	float median,madfm;
	if(size%2==0) median = (localArray[size/2]+localArray[size/2-1])/2.;
	else median = localArray[size/2];

	for(int i=0;i<size;i++) localArray[i] = fabs(localArray[i]-median);
	std::sort(localArray,localArray+size);

	if((size%2)==0) madfm = (localArray[size/2]+localArray[size/2-1])/2.;
	else madfm = localArray[size/2];
	this->itsNoiseLevel = Statistics::madfmToSigma(madfm);

	delete [] localArray;

      }


      //**************************************************************//

      void RadioSource::getFWHMestimate(float *fluxarray, double &angle, double &maj, double &min)
      {
	/// @details This returns an estimate of an object's shape,
	/// using the principle axes and position angle calculated in
	/// the duchamp::PixelInfo code. This is done by using the
	/// array of flux values given by f, thresholding at half the
	/// object's peak flux value, and averaging the x- and
	/// y-widths that the Duchamp code gives.
	///
	/// It may be that the thresholding returns more than one
	/// object. In this case, we only look at the one with the
	/// same peak location as the base object.
	
// 	angle = this->pixelArray.getSpatialMap().getPositionAngle();
// 	std::pair<double,double> axes = this->pixelArray.getSpatialMap().getPrincipleAxes();
// 	maj = std::max(axes.first,axes.second);
// 	min = std::min(axes.first,axes.second);
// 	return;

	long dim[2]; dim[0]=this->boxXsize(); dim[1]=this->boxYsize();
	duchamp::Image smlIm(dim);
	smlIm.saveArray(fluxarray,this->boxSize());
	smlIm.setMinSize(1);
	float thresh = (this->itsDetectionThreshold + this->peakFlux) / 2.;
	smlIm.stats().setThreshold(thresh);
	std::vector<PixelInfo::Object2D> objlist = smlIm.lutz_detect();
	std::vector<PixelInfo::Object2D>::iterator o;
	for(o=objlist.begin();o<objlist.end();o++){	    
	  duchamp::Detection tempobj;
	  tempobj.pixels().addChannel(0,*o);
	  tempobj.calcFluxes(fluxarray,dim);  // we need to know where the peak is.
	  if((tempobj.getXPeak()+this->boxXmin())==this->getXPeak()  &&  
	     (tempobj.getYPeak()+this->boxYmin())==this->getYPeak()){
	    angle = o->getPositionAngle();
	    std::pair<double,double> axes = o->getPrincipleAxes();
	    maj = std::max(axes.first,axes.second);
	    min = std::min(axes.first,axes.second);
	  }	  


	}
	
      }

      //**************************************************************//

      std::vector<SubComponent> RadioSource::getSubComponentList(casa::Vector<casa::Double> &f)
      {
	
	std::vector<SubComponent> cmpntlist = this->getThresholdedSubComponentList(f);

	SubComponent antipus;
	if(this->itsHeader.getBmajKeyword()>0){
	  antipus.setPA(this->itsHeader.getBpaKeyword() * M_PI / 180.);
	  antipus.setMajor(this->itsHeader.getBmajKeyword()/this->itsHeader.getAvPixScale());
	  antipus.setMinor(this->itsHeader.getBminKeyword()/this->itsHeader.getAvPixScale());
	}
	else{
	  antipus.setPA(cmpntlist[0].pa());
	  antipus.setMajor(cmpntlist[0].maj());
	  antipus.setMinor(cmpntlist[0].min());
	}
	float dx = this->getXAverage() - this->getXPeak();
	float dy = this->getYAverage() - this->getYPeak();
	antipus.setX(this->getXAverage()+dx);
	antipus.setY(this->getYAverage()+dy);
	int pos = int(this->getXAverage()+dx-this->boxXmin()) + this->boxXsize()*int(this->getYAverage()+dy-this->boxYmin());
	antipus.setPeak( f(pos) );
	cmpntlist.push_back(antipus);
	
	SubComponent centre;
	centre.setPA( antipus.pa() );
	centre.setMajor( antipus.maj() );
	centre.setMinor( antipus.min() );
	centre.setX(this->getXAverage());
	centre.setY(this->getYAverage());
	pos = int(this->getXAverage()-this->boxXmin()) + this->boxXsize()*int(this->getYAverage()-this->boxYmin());
	centre.setPeak( f(pos) );
	cmpntlist.push_back(centre);

	return cmpntlist;
      }

      std::vector<SubComponent> RadioSource::getThresholdedSubComponentList(casa::Vector<casa::Double> &f)
      {
	
	/// @details This function returns a vector list of
	/// subcomponents that make up the Detection. The pixel array
	/// f is searched at a series of thresholds spaced
	/// logarithmically between the Detection's peak flux and the
	/// original detection threshold. If more than one object is
	/// detected at any of these searches, getSubComponentList()
	/// is called on each of these objects. This recursive
	/// exectution will continue until only one object is left, at
	/// which point we return a SubComponent object that holds all
	/// parameters necessary to specify a 2D Gaussian (the shape
	/// parameters are determined using getFWHMestimate()).  The
	/// ultimate returned object is a vector list of
	/// SubComponents, ordered from highest to lowest peak flux.


	std::vector<SubComponent> fullList;

	long dim[2]; dim[0]=this->boxXsize(); dim[1]=this->boxYsize();
	duchamp::Image smlIm(dim);
	float *fluxarray = new float[this->boxSize()];
	PixelInfo::Object2D spatMap = this->pixelArray.getSpatialMap();
	for(int i=0;i<this->boxSize();i++){
	  if(spatMap.isInObject(i%this->boxXsize()+this->boxXmin(),i/this->boxXsize()+this->boxYmin())) 
	    fluxarray[i] = f(i);
	  else fluxarray[i] = 0.;
	}
	smlIm.saveArray(fluxarray,this->boxSize());
	smlIm.setMinSize(1);
	
	SubComponent base;
	base.setPeak(this->peakFlux);
	base.setX(this->xpeak);
	base.setY(this->ypeak);
	double a,b,c;
	this->getFWHMestimate(fluxarray,a,b,c);
	base.setPA(a);
	base.setMajor(b);
	base.setMinor(c);
// 	base.setPA(this->pixelArray.getSpatialMap().getPositionAngle());
// 	std::pair<double,double> axes = this->pixelArray.getSpatialMap().getPrincipleAxes();
// 	base.setMajor(std::max(axes.first,axes.second));
// 	base.setMinor(std::min(axes.first,axes.second));

	const int numThresh = 10;
	float baseThresh = log10(this->itsDetectionThreshold);
	float threshIncrement = (log10(this->peakFlux)-baseThresh)/float(numThresh+1);
	float thresh;
	int threshCtr = 0;
	std::vector<PixelInfo::Object2D> objlist;
	std::vector<PixelInfo::Object2D>::iterator obj;
	bool keepGoing;
	do{
	  threshCtr++;
	  thresh = pow(10.,baseThresh + threshCtr * threshIncrement);
	  smlIm.stats().setThreshold(thresh);
	  objlist = smlIm.lutz_detect();
	  keepGoing = (objlist.size()==1);
	}while(keepGoing && (threshCtr < numThresh));

	if(!keepGoing){
	  for(obj=objlist.begin();obj<objlist.end();obj++){
	    RadioSource newsrc;
	    newsrc.setDetectionThreshold(thresh);
	    newsrc.pixels().addChannel(0,*obj);
	    newsrc.calcFluxes(fluxarray,dim);
	    newsrc.setBox(this->box());
	    newsrc.pixels().addOffsets(this->boxXmin(),this->boxYmin(),0);
	    newsrc.xpeak += this->boxXmin();
	    newsrc.ypeak += this->boxYmin();
	    std::vector<SubComponent> newlist = newsrc.getThresholdedSubComponentList(f);
	    for(uInt i=0;i<newlist.size();i++) fullList.push_back(newlist[i]);
	  }
	}
	else fullList.push_back(base);

	std::sort(fullList.begin(),fullList.end());
	std::reverse(fullList.begin(),fullList.end());

	delete [] fluxarray;

	return fullList;
      }


      //**************************************************************//

      std::multimap<int,PixelInfo::Voxel> RadioSource::findDistinctPeaks(casa::Vector<casa::Double> f)
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

	const int numThresh = 10;

	std::multimap<int,PixelInfo::Voxel> peakMap;
	std::multimap<int,PixelInfo::Voxel>::iterator pk;

	long dim[2]; dim[0]=this->boxXsize(); dim[1]=this->boxYsize();
	duchamp::Image smlIm(dim);
	float *fluxarray = new float[this->boxSize()];
	for(int i=0;i<this->boxSize();i++) fluxarray[i] = f(i);
	smlIm.saveArray(fluxarray,this->boxSize());
	smlIm.setMinSize(1);

	float baseThresh = log10(this->itsDetectionThreshold);
	float threshIncrement = (log10(this->peakFlux)-baseThresh)/float(numThresh);

	PixelInfo::Object2D spatMap = this->pixelArray.getSpatialMap();

	for(int i=1;i<=numThresh;i++){
	  float thresh = pow(10.,baseThresh + i * threshIncrement);
	  smlIm.stats().setThreshold(thresh);
	  std::vector<PixelInfo::Object2D> objlist = smlIm.lutz_detect();
	  std::vector<PixelInfo::Object2D>::iterator o;
	  for(o=objlist.begin();o<objlist.end();o++){	    
	    duchamp::Detection tempobj;
	    tempobj.pixels().addChannel(0,*o);
	    tempobj.calcFluxes(fluxarray,dim);
	    bool pkInObj = spatMap.isInObject(tempobj.getXPeak()+this->boxXmin(),
					      tempobj.getYPeak()+this->boxYmin());
	    if(pkInObj){
	      PixelInfo::Voxel peakLoc(tempobj.getXPeak()+this->boxXmin(),
				       tempobj.getYPeak()+this->boxYmin(),
				       tempobj.getZPeak(),
				       tempobj.getPeakFlux());
	      int freq = 1;
	    
	      bool finished = false;
	      if(peakMap.size()>0){
		pk = peakMap.begin();
		while(!finished && pk!=peakMap.end()){
		  if(!(pk->second==peakLoc)) pk++;
		  else{
		    freq = pk->first+1;
		    peakMap.erase(pk);
		    finished=true;
		  }
		}
	      }
	      peakMap.insert( std::pair<int,PixelInfo::Voxel>(freq,peakLoc) );
	    }
	  }

	}
	
	delete [] fluxarray;

	return peakMap;

      }
    
      //**************************************************************//

      /// @brief A simple way of printing fitted parameters
      void printparameters(Matrix<Double> &m)
      {
	cout.precision(3);
	cout.setf(ios::fixed);
	uInt g,p;
	for (g = 0; g < m.nrow(); g++)
	  {
	    for (p = 0; p < m.ncolumn() - 1; p++) cout << m(g,p) << ", ";
	    cout << m(g,p) << endl;
	    if (g < m.nrow() - 1) cout << "                    ";
	  }

      }

      /// @brief A simple way of printing fitted parameters
      void logparameters(Matrix<Double> &m)
      {
	uInt g,p;
	for (g = 0; g < m.nrow(); g++)
	  {
	    std::stringstream outmsg;
	    outmsg.precision(3);
	    outmsg.setf(ios::fixed);
	    for (p = 0; p < m.ncolumn() - 1; p++) outmsg << m(g,p) << ", ";
	    outmsg << m(g,p);
	    ASKAPLOG_INFO_STR(logger, outmsg.str());
	  }

      }

      //**************************************************************//

      bool RadioSource::fitGauss(std::vector<PixelInfo::Voxel> *voxelList)
      {

	/// @details First defines the pixel array with the flux
	/// values by extracting the voxels from voxelList that are
	/// within the box surrounding the object. Their flux values
	/// are placed in the flux matrix, which is passed to
	/// fitGauss(casa::Matrix<casa::Double> pos,
	/// casa::Vector<casa::Double> f, casa::Vector<casa::Double>
	/// sigma).

	if(this->getSpatialSize() < minFitSize) return false;

	casa::Matrix<casa::Double> pos;
	casa::Vector<casa::Double> f;
 	casa::Vector<casa::Double> sigma;
	pos.resize(this->boxSize(),2);
	f.resize(this->boxSize());
	sigma.resize(this->boxSize());
	casa::Vector<casa::Double> curpos(2);
	curpos=0;

	bool failure = false;

	if(this->getZcentre()!=this->getZmin() || this->getZcentre() != this->getZmax()){
	  ASKAPLOG_ERROR(logger,"Can only do fitting for two-dimensional objects!");
	  return failure;
	}

	long z = this->getZPeak();
	for(long x=this->boxXmin();x<=this->boxXmax() && !failure;x++){
	  for(long y=this->boxYmin();y<=this->boxYmax() && !failure;y++){
	    int i = (x-this->boxXmin()) + (y-this->boxYmin())*this->boxXsize();
	    PixelInfo::Voxel tempvox(x,y,z,0.);
	    std::vector<PixelInfo::Voxel>::iterator vox = voxelList->begin();
	    while( !tempvox.match(*vox) && vox!=voxelList->end() ) vox++;
	    if(vox == voxelList->end()) failure = true;
	    else f(i) = vox->getF();
 	    sigma(i) = this->itsNoiseLevel;
	    curpos(0)=x;
	    curpos(1)=y;
	    pos.row(i)=curpos;
	  }
	}
	
	if(failure){
	  ASKAPLOG_ERROR(logger, "RadioSource: Failed to allocate flux array");
	  return !failure;
	}

	return fitGauss(pos,f,sigma);

      }

      //**************************************************************//

      bool RadioSource::fitGauss(float *fluxArray, long *dimArray, Fitter &baseFitter)
//       bool RadioSource::fitGauss(float *fluxArray, long *dimArray)
      {

	/// @details First defines the pixel array with the flux
	/// values by extracting the voxels from fluxArray that are
	/// within the box surrounding the object. Their flux values
	/// are placed in the flux matrix, which is passed to
	/// fitGauss(casa::Matrix<casa::Double> pos,
	/// casa::Vector<casa::Double> f, casa::Vector<casa::Double>
	/// sigma).

	if(this->getSpatialSize() < minFitSize) return false;

	if(this->getZcentre()!=this->getZmin() || this->getZcentre() != this->getZmax()){
	  ASKAPLOG_ERROR(logger,"Can only do fitting for two-dimensional objects!");
	  return false;
	}

	casa::Matrix<casa::Double> pos;
	casa::Vector<casa::Double> f;
	casa::Vector<casa::Double> sigma;
	pos.resize(this->boxSize(),2);
	f.resize(this->boxSize());
	sigma.resize(this->boxSize());
	casa::Vector<casa::Double> curpos(2);
	curpos=0;
	for(int x=this->boxXmin();x<=this->boxXmax();x++){
	  for(int y=this->boxYmin();y<=this->boxYmax();y++){
	    int i = (x-this->boxXmin()) + (y-this->boxYmin())*this->boxXsize();
	    int j = x + y*dimArray[0];
	    if((j>=0)&&(j<dimArray[0]*dimArray[1])) f(i) = fluxArray[j];
	    else f(i)=0.;
	    sigma(i) = this->itsNoiseLevel;
	    curpos(0)=x;
	    curpos(1)=y;
	    pos.row(i)=curpos;
	  }
	}

// 	return fitGauss(pos,f,sigma);
	return fitGaussNew(pos,f,sigma,baseFitter);

      }

      //**************************************************************//
      
      bool RadioSource::fitGaussNew(casa::Matrix<casa::Double> pos, casa::Vector<casa::Double> f,
				    casa::Vector<casa::Double> sigma, Fitter &baseFitter)
      {

	if(this->getSpatialSize() < minFitSize) return false;

	ASKAPLOG_INFO_STR(logger, "Fitting source at RA=" << this->raS << ", Dec=" << this->decS);

	ASKAPLOG_INFO_STR(logger, "detect thresh = " << this->itsDetectionThreshold
			  << "  peak = " << this->peakFlux 
			  << "  noise level = " << this->itsNoiseLevel);
	
	std::vector<SubComponent> cmpntList = this->getSubComponentList(f);
	ASKAPLOG_INFO_STR(logger, "Found " << cmpntList.size() << " subcomponents");
	for(uInt i=0;i<cmpntList.size();i++)
	  ASKAPLOG_INFO_STR(logger, "SubComponent: " << cmpntList[i]);

	const int maxNumGauss = 4;
	Fitter fit[maxNumGauss];

	bool fitIsGood = false;
	int bestFit = 0;
	float bestRChisq = 9999.;

	for(int ctr=0;ctr<maxNumGauss;ctr++){

	  fit[ctr].setNumGauss(ctr+1);
	  fit[ctr].setEstimates(cmpntList, this->itsHeader);
	  fit[ctr].setRetries();
	  fit[ctr].setMasks();
	  fit[ctr].fit(pos, f, sigma);

	  if(fit[ctr].acceptable(this)){
	    if((ctr==0) || (fit[ctr].redChisq() < bestRChisq)){
	      fitIsGood = true;
	      bestFit = ctr;
	      bestRChisq = fit[ctr].redChisq();
	    }

	  }

	} // end of 'ctr' for-loop

	if(fitIsGood){
	  this->hasFit = true;
	  // Make a map so that we can output the fitted components in order of peak flux
	  std::multimap<double,int> fitMap = fit[bestFit].peakFluxList();
	  // Need to use reverse_iterator so that brightest component's listed first
	  std::multimap<double,int>::reverse_iterator rfit=fitMap.rbegin();
	  for(;rfit!=fitMap.rend();rfit++)
	    this->itsGaussFitSet.push_back(fit[bestFit].gaussian(rfit->second));

	  ASKAPLOG_INFO_STR(logger,"BEST FIT: " << bestFit+1 << " Gaussians"
			    << ", chisq = " << fit[bestFit].chisq()
			    << ", chisq/nu =  "  << bestRChisq);
	}
	else{
	  ASKAPLOG_INFO_STR(logger, "No good fit found.");
	}

	ASKAPLOG_INFO_STR(logger, "-----------------------");

	return fitIsGood;


      }

      //**************************************************************//
	
      bool RadioSource::fitGauss(casa::Matrix<casa::Double> pos, casa::Vector<casa::Double> f,
				 casa::Vector<casa::Double> sigma)
      {
	/// @details
	///
	/// This function fits a number of Gaussians to the
	/// Detection. All pixels within a box encompassing the
	/// detection plus the border given by detectionBorder are
	/// included in the fit.
 
	const int maxNumGauss = 4;

	if(this->getSpatialSize() < minFitSize) return false;

	ASKAPLOG_INFO_STR(logger, "Fitting source at RA=" << this->raS << ", Dec=" << this->decS);

	ASKAPLOG_INFO_STR(logger, "detect thresh = " << this->itsDetectionThreshold
			  << "  peak = " << this->peakFlux 
			  << "  noise level = " << this->itsNoiseLevel);

	float boxFlux = 0.;
	for(int i=0;i<this->boxSize();i++) boxFlux += f(i);

	Double maxRMS = 5.;
	casa::Matrix<casa::Double> estimate;
	casa::Matrix<casa::Double> retryfactors;
	casa::Matrix<casa::Double> baseRetryfactors;
	casa::Matrix<casa::Double> solution[maxNumGauss];

	baseRetryfactors.resize(1,6);
	baseRetryfactors(0,0) = 1.1; 
	baseRetryfactors(0,1) = 0.1; 
	baseRetryfactors(0,2) = 0.1;
	baseRetryfactors(0,3) = 1.1; 
	baseRetryfactors(0,4) = 1.01;
	baseRetryfactors(0,5) = M_PI/180.;

	float chisq[maxNumGauss];
	FitGaussian<casa::Double> fitgauss[maxNumGauss];
	bool fitIsGood = false;
	int bestFit = 0;
	float bestRChisq = 9999.;

	std::vector<SubComponent> cmpntList = this->getSubComponentList(f);
	ASKAPLOG_INFO_STR(logger, "Found " << cmpntList.size() << " subcomponents");
	for(uInt i=0;i<cmpntList.size();i++)
	  ASKAPLOG_INFO_STR(logger, "SubComponent: " << cmpntList[i]);

	for(int ctr=0;ctr<maxNumGauss;ctr++){

	  unsigned int numGauss = ctr + 1;
	  ASKAPLOG_INFO_STR(logger, "Attempting to fit " << numGauss << " Gaussians."); 
	  fitgauss[ctr].setDimensions(2);
	  fitgauss[ctr].setNumGaussians(numGauss);

	  estimate.resize(numGauss,6);

	  uInt nCmpnt = cmpntList.size();
	  for(uInt g=0;g<numGauss;g++){
	    uInt cmpnt = g % nCmpnt;
	    estimate(g,0) = cmpntList[cmpnt].peak();
	    estimate(g,1) = cmpntList[cmpnt].x();
	    estimate(g,2) = cmpntList[cmpnt].y();
	    if(this->itsHeader.getBmajKeyword()>0 && 
	       (this->itsHeader.getBmajKeyword()/this->itsHeader.getAvPixScale() > cmpntList[cmpnt].maj())){
	      estimate(g,3)=this->itsHeader.getBmajKeyword()/this->itsHeader.getAvPixScale();
	      estimate(g,4)=this->itsHeader.getBminKeyword()/this->itsHeader.getBmajKeyword();
	      estimate(g,5)=this->itsHeader.getBpaKeyword() * M_PI / 180.;
	    }
	    else{
	      estimate(g,3) = cmpntList[cmpnt].maj();
	      estimate(g,4) = cmpntList[cmpnt].min()/cmpntList[cmpnt].maj();
	      estimate(g,5) = cmpntList[cmpnt].pa();
	    }
	  }

	  fitgauss[ctr].setFirstEstimate(estimate);
	  ASKAPLOG_INFO_STR(logger, "Initial estimates of parameters follow: ");
	  logparameters(estimate);

	  retryfactors.resize(numGauss,6);
	  for(unsigned int g=0;g<numGauss;g++)
	    for(unsigned int i=0;i<6;i++)
	      retryfactors(g,i) = baseRetryfactors(0,i);
	  fitgauss[ctr].setRetryFactors(retryfactors);

// 	  // mask the beam parameters
// 	  //	  std::cout << "Mask values:\n";
// 	  for(unsigned int g=0;g<numGauss;g++){
// 	    fitgauss[ctr].mask(g,3) = false;
// 	    fitgauss[ctr].mask(g,4) = false;
// 	    fitgauss[ctr].mask(g,5) = false;
// 	    // 	    for(int i=0;i<6;i++) fitgauss[ctr].mask(g,i)=false;
// 	    //	    for(int i=0;i<6;i++) fitgauss[ctr].mask(g,i) = !fitgauss[ctr].mask(g,i);
// 	    //	    for(int i=0;i<6;i++) std::cout << fitgauss[ctr].mask(g,i);
// 	    //	    std::cout << "\n";
// 	  }	      
    
	  solution[ctr].resize();
	  bool thisFitGood = true;
	  for(int fitloop=0;fitloop<3;fitloop++){
	    try {
	      solution[ctr] = fitgauss[ctr].fit(pos, f, sigma, maxRMS);
	    } catch (AipsError err) {
	      std::string message = err.getMesg().chars();
	      message = "FIT ERROR: " + message;
	      ASKAPLOG_ERROR(logger, message);
	      thisFitGood = false;
	    }
	    for(unsigned int i=0;i<numGauss;i++){
	      solution[ctr](i,5) = remainder(solution[ctr](i,5), 2.*M_PI);
	    }
	    ASKAPLOG_INFO_STR(logger,  "Int. Solution #" << fitloop+1
			      <<": chisq=" << fitgauss[ctr].chisquared()
			      <<": Parameters are:"); 
	    logparameters(solution[ctr]);
	    if(!fitgauss[ctr].converged()) fitloop=9999;
	    else fitgauss[ctr].setFirstEstimate(solution[ctr]);
	  }

	  for(unsigned int i=0;i<numGauss;i++){
	    solution[ctr](i,5) = remainder(solution[ctr](i,5), 2.*M_PI);
	  }

	  chisq[ctr] = fitgauss[ctr].chisquared();
	  int ndof = this->boxSize() - numGauss*6 - 1;
	  float rchisq = chisq[ctr] / float(ndof);
	
	  cout.precision(6);
	  if(fitgauss[ctr].converged()){
	    ASKAPLOG_INFO_STR(logger, "Fit converged. Solution Parameters follow: "); 
	    logparameters(solution[ctr]);
	  }
	  else ASKAPLOG_INFO_STR(logger, "Fit did not converge");

	  std::stringstream outmsg;
	  outmsg << "Num Gaussians = " << numGauss;
	  if( fitgauss[ctr].converged()) outmsg << ", Converged";
	  else outmsg << ", Failed";
	  outmsg << ", chisq = " << chisq[ctr]
		 << ", chisq/nu =  "  << rchisq
		 << ", dof = " << ndof
		 << ", RMS = " << fitgauss[ctr].RMS();
	  ASKAPLOG_INFO_STR(logger, outmsg.str());

	  /// Acceptance criteria for a fit are as follows (after the
	  /// FIRST survey criteria, White et al 1997, ApJ 475, 479):
	  /// @li Fit must have converged
	  /// @li Fit must be acceptable according to its chisq value
	  /// @li The centre of each component must be inside the box
	  /// @li The separation between any pair of components must be more than 2 pixels.
	  /// @li The flux of each component must be positive and more than half the detection threshold
	  /// @li No component's peak flux can exceed twice the highest pixel in the box
	  /// @li The sum of the integrated fluxes of all components
	  /// must not be more than twice the total flux in the box.

	  bool passConv, passChisq, passFlux, passXLoc, passYLoc, passSep, passIntFlux, passPeak;

	  passConv  = fitgauss[ctr].converged();
	  passConv  = passConv && (chisq[ctr]>0.);

	  passChisq = false;
	  passXLoc = passYLoc = passFlux = passSep = passPeak = passIntFlux = true;

	  if(passConv){

	    if(ndof<343)
	      passChisq = chisqProb(ndof,chisq[ctr]) > 0.01; // Test acceptance at 99% level
	    else 
	      passChisq = (rchisq < 1.2);
	    
	    float intFlux = 0.;
	    for(unsigned int i=0;i<numGauss;i++){
	      passXLoc = passXLoc && (solution[ctr](i,1)>this->boxXmin()) && 
		(solution[ctr](i,1)<this->boxXmax());
	      passYLoc = passYLoc && (solution[ctr](i,2)>this->boxYmin()) && 
		(solution[ctr](i,2)<this->boxYmax());
	      passFlux = passFlux && (solution[ctr](i,0) > 0.);
	      passFlux = passFlux && (solution[ctr](i,0) > 0.5*this->itsDetectionThreshold);
 	      passPeak = passPeak && (solution[ctr](i,0) < 2.*this->peakFlux);	    
	      
	      Gaussian2D<Double> component(solution[ctr](i,0),solution[ctr](i,1),solution[ctr](i,2),
					   solution[ctr](i,3),solution[ctr](i,4),solution[ctr](i,5));
	      intFlux += component.flux();
	      
	      for(unsigned int j=i+1;j<numGauss;j++){
		float sep = hypot( solution[ctr](i,1)-solution[ctr](j,1) , 
				   solution[ctr](i,2)-solution[ctr](j,2) );
		passSep = passSep && (sep > 2.);
	      }
	    }
	    
	    passIntFlux = (intFlux < 2.*boxFlux);

	  }

	  ASKAPLOG_INFO_STR(logger,"Passes: "<<passConv<<passChisq<<passXLoc<<passYLoc<<passSep
			    <<passFlux<<passPeak<<passIntFlux);

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
	  this->hasFit = true;
	  // Make a map so that we can output the fitted components in order of peak flux
	  std::multimap<double,int> fitMap;
	  for(int i=0;i<=bestFit;i++) fitMap.insert(std::pair<double,int>(solution[bestFit](i,0),i));
	  // Need to use reverse_iterator so that brightest component's listed first
	  std::multimap<double,int>::reverse_iterator fit=fitMap.rbegin();
	  for(;fit!=fitMap.rend();fit++){
	    int ifit = fit->second;
	    casa::Gaussian2D<casa::Double> 
	      gauss(solution[bestFit](ifit,0),
		    solution[bestFit](ifit,1),solution[bestFit](ifit,2),
		    solution[bestFit](ifit,3),solution[bestFit](ifit,4),solution[bestFit](ifit,5));
	    this->itsGaussFitSet.push_back(gauss);
	  }
	  ASKAPLOG_INFO_STR(logger,"BEST FIT: " << bestFit+1 << " Gaussians"
			    << ", chisq = " << bestRChisq * (this->boxSize() - 6*(bestFit+1) - 1)
			    << ", chisq/nu =  "  << bestRChisq);
	}
	else{
	  ASKAPLOG_INFO_STR(logger, "No good fit found.");
	}

	ASKAPLOG_INFO_STR(logger, "-----------------------");

	return fitIsGood;

      }

      //**************************************************************//

      void RadioSource::printFit()
      {
	std::cout << "Fitted " << itsGaussFitSet.size() << " Gaussians\n";
	for(unsigned int g=0;g<itsGaussFitSet.size();g++){
	  // 	itsGaussFitSet[g].parameters().print(std::cout);
	  // 	std::cout << "\n";
	  std::cout << itsGaussFitSet[g] << "\n";
	}

      }

      //**************************************************************//

      void RadioSource::printSummary(std::ostream &stream, std::vector<duchamp::Column::Col> columns,
				     bool doHeader)
      {
	/// @details
	///
	/// This function writes out the position and flux information
	/// for the detected object and its fitted componenets. The
	/// information includes:
	/// @li RA & Dec & Vel
	/// @li Detected peak flux (from duchamp::Detection object)
	/// @li Detected integrated flux (from duchamp::Detection)
	/// @li Number of fitted componente
	/// @li Peak & Integrated flux of fitted components (using all components) 

	stream.setf(std::ios::fixed);
	
	int suffixCtr=0;
	char suffix[4]  = {'a','b','c','d'};

	int prec=columns[duchamp::Column::FINT].getPrecision();
	if(prec < 6)
	  for(int i=prec;i<6;i++) columns[duchamp::Column::FINT].upPrec();
	prec=columns[duchamp::Column::FPEAK].getPrecision();
	if(prec < 6)
	  for(int i=prec;i<6;i++) columns[duchamp::Column::FPEAK].upPrec();

	if(doHeader){

	  columns[duchamp::Column::NUM].printTitle(stream);
	  columns[duchamp::Column::RA].printTitle(stream);
	  columns[duchamp::Column::DEC].printTitle(stream);
// 	  columns[duchamp::Column::VEL].printTitle(stream);
	  columns[duchamp::Column::FINT].printTitle(stream);
	  columns[duchamp::Column::FPEAK].printTitle(stream);
	  stream << "   F_int(fit)    F_pk(fit)   Maj(fit)   Min(fit)  P.A.(fit)\n";
	  int width = columns[duchamp::Column::NUM].getWidth() + 
	    columns[duchamp::Column::RA].getWidth() + 
	    columns[duchamp::Column::DEC].getWidth() +
// 	    columns[duchamp::Column::VEL].getWidth() +
	    columns[duchamp::Column::FINT].getWidth() +
	    columns[duchamp::Column::FPEAK].getWidth();
	  stream << std::setfill('-') << std::setw(width) << '-'
		 << "-----------------------------------------------------------\n";
	}

	if(this->itsGaussFitSet.size()==0) {  //if no fits were made...
	  columns[duchamp::Column::NUM].printEntry(stream,this->getID());
	  columns[duchamp::Column::RA].printEntry(stream,this->getRAs());
	  columns[duchamp::Column::DEC].printEntry(stream,this->getDecs());
// 	  columns[duchamp::Column::VEL].printEntry(stream,this->getVel());
	  columns[duchamp::Column::FINT].printEntry(stream,this->getIntegFlux());
	  columns[duchamp::Column::FPEAK].printEntry(stream,this->getPeakFlux());
	  float peakflux=0.,intflux=0.,maj=0.,min=0.,pa=0.;
	  stream << " " << std::setw(12) << std::setprecision(6) << intflux << " ";
	  stream << std::setw(12) << std::setprecision(6) << peakflux << " ";
	  stream << std::setw(10) << std::setprecision(6) << maj << " ";
	  stream << std::setw(10) << std::setprecision(6) << min << " ";
	  stream << std::setw(7) << std::setprecision(2) << pa << "\n";
	}

	std::vector<casa::Gaussian2D<Double> >::iterator fit;
	for(fit=this->itsGaussFitSet.begin(); fit<this->itsGaussFitSet.end(); fit++){

	  std::stringstream id;
	  id << this->getID() << suffix[suffixCtr++];
	  columns[duchamp::Column::NUM].printEntry(stream, id.str());
	  double *pix = new double[3];
	  pix[0] = fit->xCenter();
	  pix[1] = fit->yCenter();
	  pix[2] = this->getZcentre();
	  double *wld = new double[3];
	  this->itsHeader.pixToWCS(pix,wld);
	  std::string thisRA = evaluation::decToDMS(wld[0],"RA");
	  std::string thisDec = evaluation::decToDMS(wld[1],"DEC");
	  columns[duchamp::Column::RA].printEntry(stream,  thisRA);
	  columns[duchamp::Column::DEC].printEntry(stream, thisDec);
// 	  columns[duchamp::Column::VEL].printEntry(stream,this->getVel());
	  columns[duchamp::Column::FINT].printEntry(stream,this->getIntegFlux());
	  columns[duchamp::Column::FPEAK].printEntry(stream,this->getPeakFlux());

	  stream << " " ;

	  float peakflux=0.,intflux=0.,maj=0.,min=0.,pa=0.;
	  peakflux = fit->height();
	  intflux  = fit->flux();
	  if(this->itsHeader.needBeamSize()) 
	    intflux /= this->itsHeader.getBeamSize(); // Convert from Jy/beam to Jy
	  maj = fit->majorAxis()*this->itsHeader.getAvPixScale()*3600.; // convert from pixels to arcsec
	  min = fit->minorAxis()*this->itsHeader.getAvPixScale()*3600.;
	  pa = fit->PA()*180./M_PI;
	  stream << std::setw(12) << std::setprecision(6) << intflux << " ";
	  stream << std::setw(12) << std::setprecision(6) << peakflux << " ";
	  stream << std::setw(10) << std::setprecision(6) << maj << " ";
	  stream << std::setw(10) << std::setprecision(6) << min << " ";
	  stream << std::setw(7) << std::setprecision(2) << pa << "\n";

	}

      }

      //**************************************************************//

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
	  this->itsHeader.pixToWCS(pix,world);
	
	  stream.precision(6);
	  stream << "ELLIPSE " 
		 << world[0] << " " 
		 << world[1] << " "
		 << fit->majorAxis() * this->itsHeader.getAvPixScale() / (2.*sqrt(2.*M_LN2)) << " "
		 << fit->minorAxis() * this->itsHeader.getAvPixScale() / (2.*sqrt(2.*M_LN2)) << " "
		 << fit->PA() * 180. / M_PI << "\n";
	  
	}

	pix[0] = this->getXmin()-sourcefitting::detectionBorder;
	pix[1] = this->getYmin()-sourcefitting::detectionBorder;
	this->itsHeader.pixToWCS(pix,world);
	stream << "BOX " << world[0] << " " << world[1] << " ";
	
	pix[0] = this->getXmax()+sourcefitting::detectionBorder;
	pix[1] = this->getYmax()+sourcefitting::detectionBorder;
	this->itsHeader.pixToWCS(pix,world);
	stream << world[0] << " " << world[1] << "\n";
	
      
      }


    }

  }

}
