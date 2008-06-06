/// @file
///
///
/// (c) 2008 ASKAP, All Rights Reserved
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/RadioSource.h>
#include <analysisutilities/AnalysisUtilities.h>

#include <duchamp/fitsHeader.hh>
#include <duchamp/PixelMap/Voxel.hh>
#include <duchamp/PixelMap/Object2D.hh>
#include <duchamp/Cubes/cubes.hh>
#include <duchamp/Detection/detection.hh>
#include <duchamp/Detection/columns.hh>

#include <scimath/Fitting/FitGaussian.h>
#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <scimath/Functionals/Gaussian3D.h>
#include <casa/namespace.h>

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

	float threshIncrement = (this->peakFlux-this->itsDetectionThreshold)/float(numThresh);
	for(int i=1;i<=numThresh;i++){
	  float thresh = this->itsDetectionThreshold + i * threshIncrement;
	  smlIm.stats().setThreshold(thresh);
	  std::vector<PixelInfo::Object2D> objlist = smlIm.lutz_detect();
	  std::vector<PixelInfo::Object2D>::iterator o;
	  for(o=objlist.begin();o<objlist.end();o++){	    
	    duchamp::Detection tempobj;
	    tempobj.pixels().addChannel(0,*o);
	    tempobj.calcFluxes(fluxarray,dim);
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

	float failure = false;

	if(this->getZcentre()!=this->getZmin() || this->getZcentre() != this->getZmax()){
	  ASKAPLOG_ERROR(logger,"Can only do fitting for two-dimensional objects!");
	  return failure;
	}

	long z = this->getZcentre();
	for(long x=this->boxXmin();x<=this->boxXmax() && !failure;x++){
	  for(long y=this->boxYmin();y<=this->boxYmax() && !failure;y++){
	    int i = (x-this->boxXmin()) + (y-this->boxYmin())*this->boxXsize();
	    PixelInfo::Voxel tempvox(x,y,z,0.);
	    std::vector<PixelInfo::Voxel>::iterator vox = voxelList->begin();
	    while( !tempvox.match(*vox) && vox!=voxelList->end() ) vox++;
	    if(vox == voxelList->end()) failure = true;
	    else f(i) = vox->getF() / this->itsNoiseLevel;
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

      bool RadioSource::fitGauss(float *fluxArray, long *dimArray)
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
	    if((j>=0)&&(j<dimArray[0]*dimArray[1])) f(i) = fluxArray[j] / this->itsNoiseLevel;
	    else f(i)=0.;
	    sigma(i) = this->itsNoiseLevel;
	    curpos(0)=x;
	    curpos(1)=y;
	    pos.row(i)=curpos;
	  }
	}

	return fitGauss(pos,f,sigma);

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
 
	if(this->getSpatialSize() < minFitSize) return false;

	float boxFlux = 0.;
	for(int i=0;i<this->boxSize();i++) boxFlux += f(i);
	std::vector<Double> fluxes;
	f.tovector(fluxes);
	float peakFlux = *std::max_element(fluxes.begin(), fluxes.end()) / this->itsNoiseLevel;

	std::multimap<int,PixelInfo::Voxel> peakList = this->findDistinctPeaks(f);
	std::multimap<int,PixelInfo::Voxel>::reverse_iterator pk;

	Double maxRMS = 5.;
	casa::Matrix<casa::Double> components;
	casa::Matrix<casa::Double> estimate;
	casa::Matrix<casa::Double> baseEstimate;
	casa::Matrix<casa::Double> retryfactors;
	casa::Matrix<casa::Double> baseRetryfactors;
	casa::Matrix<casa::Double> solution[4];

	baseEstimate.resize(1,6);
	baseEstimate(0,0)=this->peakFlux  / this->itsNoiseLevel;   // height of Gaussian
	baseEstimate(0,1)=this->getXcentre();    // x centre
	baseEstimate(0,2)=this->getYcentre();    // y centre
	// get beam information from the FITSheader, if present.
	if(this->itsHeader.getBmajKeyword()>0){
	  baseEstimate(0,3)=this->itsHeader.getBmajKeyword()/this->itsHeader.getAvPixScale();
	  baseEstimate(0,4)=this->itsHeader.getBminKeyword()/this->itsHeader.getBmajKeyword();
	  baseEstimate(0,5)=this->itsHeader.getBpaKeyword() * M_PI / 180.;
	}
	else {
	  float xwidth=(this->getXmax()-this->getXmin() + 1)/2.;
	  float ywidth=(this->getYmax()-this->getYmin() + 1)/2.;
	  baseEstimate(0,3)=std::max(xwidth,ywidth);// x width (doesn't have to be x...)
	  baseEstimate(0,4)=std::min(xwidth,ywidth)/std::max(xwidth,ywidth); // axial ratio
	  baseEstimate(0,5)=0.;                  // position angle
	}
	cout << "Estimated Parameters: "; printparameters(baseEstimate);

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

	  unsigned int numGauss = ctr + 1;
	  fitgauss[ctr].setDimensions(2);
	  fitgauss[ctr].setNumGaussians(numGauss);

	  estimate.resize(numGauss,6);
	  pk = peakList.rbegin();
	  for(unsigned int g=0;g<numGauss;g++){
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

	  retryfactors.resize(numGauss,6);
	  for(unsigned int g=0;g<numGauss;g++)
	    for(unsigned int i=0;i<6;i++)
	      retryfactors(g,i) = baseRetryfactors(0,i);
	  fitgauss[ctr].setRetryFactors(retryfactors);

	  // mask the beam parameters
	  //	  std::cout << "Mask values:\n";
	  for(unsigned int g=0;g<numGauss;g++){
	    fitgauss[ctr].mask(g,3) = false;
	    fitgauss[ctr].mask(g,4) = false;
	    fitgauss[ctr].mask(g,5) = false;
	    // 	    for(int i=0;i<6;i++) fitgauss[ctr].mask(g,i)=false;
	    //	    for(int i=0;i<6;i++) fitgauss[ctr].mask(g,i) = !fitgauss[ctr].mask(g,i);
	    //	    for(int i=0;i<6;i++) std::cout << fitgauss[ctr].mask(g,i);
	    //	    std::cout << "\n";
	  }	      
    
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
	  int ndof = this->boxSize() - numGauss*6 - 1;
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
	  /// @li The sum of the integrated fluxes of all components
	  /// must not be more than twice the total flux in the box.

	  bool passConv, passChisq, passFlux, passXLoc, passYLoc, passSep, passIntFlux, passPeak;

	  passConv  = fitgauss[ctr].converged();
	  passConv  = passConv && (chisq[ctr]>0.);
	  for(unsigned int i=0;i<numGauss;i++){
	    passConv = passConv && ( fabs(solution[ctr](i,5))<2.*M_PI );
	  }

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
	      passFlux = passFlux && (solution[ctr](i,0) * this->itsNoiseLevel
				      > 0.5*this->itsDetectionThreshold);
	      passPeak = passPeak && (solution[ctr](i,0) < 2.*peakFlux);	    
	      
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
	  this->hasFit = true;
	  for(int i=0;i<=bestFit;i++){
	    casa::Gaussian2D<casa::Double> 
	      gauss(solution[bestFit](i,0) * this->itsNoiseLevel,
		    solution[bestFit](i,1),solution[bestFit](i,2),
		    solution[bestFit](i,3),solution[bestFit](i,4),solution[bestFit](i,5));
	    this->itsGaussFitSet.push_back(gauss);
	  }
	  cout << "BEST FIT: " << bestFit+1 << " Gaussians"
	       << ", chisq = " << bestRChisq * (this->boxSize() - 6*(bestFit+1) - 1)
	       << ", chisq/nu =  "  << bestRChisq << endl;
	}
	else{
	  cout << "No good fit found.\n";
	}

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
	  columns[duchamp::Column::VEL].printTitle(stream);
	  columns[duchamp::Column::FINT].printTitle(stream);
	  columns[duchamp::Column::FPEAK].printTitle(stream);
	  stream << " #Fit  F_int (fit)   F_pk (fit)\n";
	  int width = columns[duchamp::Column::NUM].getWidth() + 
	    columns[duchamp::Column::RA].getWidth() + 
	    columns[duchamp::Column::DEC].getWidth() +
	    columns[duchamp::Column::VEL].getWidth() +
	    columns[duchamp::Column::FINT].getWidth() +
	    columns[duchamp::Column::FPEAK].getWidth();
	  stream << std::setfill('-') << std::setw(width) << '-'
		      << "-------------------------------\n";
	}

	columns[duchamp::Column::NUM].printEntry(stream,this->getID());
	columns[duchamp::Column::RA].printEntry(stream,this->getRAs());
	columns[duchamp::Column::DEC].printEntry(stream,this->getDecs());
	columns[duchamp::Column::VEL].printEntry(stream,this->getVel());
	columns[duchamp::Column::FINT].printEntry(stream,this->getIntegFlux());
	columns[duchamp::Column::FPEAK].printEntry(stream,this->getPeakFlux());

	stream << " " << std::setw(4) << this->itsGaussFitSet.size() << " ";

	float peakflux=0.,intflux=0.;
	std::vector<casa::Gaussian2D<Double> >::iterator fit;
	for(fit=this->itsGaussFitSet.begin(); fit<this->itsGaussFitSet.end(); fit++){
	  if((fit==this->itsGaussFitSet.begin()) || (fit->height()>peakflux)) 
	    peakflux = fit->height();
	  intflux += fit->flux();
	}
	if(this->itsHeader.needBeamSize()) 
	  intflux /= this->itsHeader.getBeamSize(); // Convert from Jy/beam to Jy
	stream << std::setw(12) << std::setprecision(6) << intflux << " ";
	stream << std::setw(12) << std::setprecision(6) << peakflux << "\n";

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
