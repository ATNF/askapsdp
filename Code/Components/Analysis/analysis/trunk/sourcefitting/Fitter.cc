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

#include <sourcefitting/Fitter.h>
#include <sourcefitting/RadioSource.h>
#include <sourcefitting/Component.h>
#include <analysisutilities/AnalysisUtilities.h>
#include <evaluationutilities/EvaluationUtilities.h>

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

      Fitter::Fitter(const LOFAR::ACC::APS::ParameterSet& parset)
      {
	
	this->itsMaxRMS = parset.getDouble("maxRMS", defaultMaxRMS);
	this->itsNumGauss = parset.getInt32("numGauss", defaultNumFittedGauss);
	this->itsBoxPadSize = parset.getInt32("boxPadSize", detectionBorder);
	this->itsChisqCutoff = parset.getFloat("chisqCutoff", defaultChisqCutoff);
	this->itsNoiseBoxSize = parset.getInt32("noiseBoxSize", defaultNoiseBoxSize);
	
	this->itsFitter.setDimensions(2);

      }


      void Fitter::setEstimates(std::vector<SubComponent> &cmpntList, duchamp::FitsHeader &head)
      {
	
	this->itsFitter.setDimensions(2);
	this->itsFitter.setNumGaussians(this->itsNumGauss);

	casa::Matrix<casa::Double> estimate;
	estimate.resize(this->itsNumGauss,6);

	uInt nCmpnt = cmpntList.size();
	for(uInt g=0;g<this->itsNumGauss;g++){
	  uInt cmpnt = g % nCmpnt;

	  estimate(g,0) = cmpntList[cmpnt].peak();
	  estimate(g,1) = cmpntList[cmpnt].x();
	  estimate(g,2) = cmpntList[cmpnt].y();

	  if(head.getBmajKeyword()>0 && 
	     (head.getBmajKeyword()/head.getAvPixScale() > cmpntList[cmpnt].maj())){
	    estimate(g,3)=head.getBmajKeyword()/head.getAvPixScale();
	    estimate(g,4)=head.getBminKeyword()/head.getBmajKeyword();
	    estimate(g,5)=head.getBpaKeyword() * M_PI / 180.;
	  }
	  else{
	    estimate(g,3) = cmpntList[cmpnt].maj();
	    estimate(g,4) = cmpntList[cmpnt].min()/cmpntList[cmpnt].maj();
	    estimate(g,5) = cmpntList[cmpnt].pa();
	  }
	
	}

	this->itsFitter.setFirstEstimate(estimate);



      }


      void Fitter::setRetries()
      {
	casa::Matrix<casa::Double> retryfactors;
	casa::Matrix<casa::Double> baseRetryfactors;

	baseRetryfactors.resize(1,6);
	retryfactors.resize(this->itsNumGauss,6);

	baseRetryfactors(0,0) = 1.1; 
	baseRetryfactors(0,1) = 0.1; 
	baseRetryfactors(0,2) = 0.1;
	baseRetryfactors(0,3) = 1.1; 
	baseRetryfactors(0,4) = 1.01;
	baseRetryfactors(0,5) = M_PI/180.;

	for(unsigned int g=0;g<this->itsNumGauss;g++)
	  for(unsigned int i=0;i<6;i++)
	    retryfactors(g,i) = baseRetryfactors(0,i);

	this->itsFitter.setRetryFactors(retryfactors);

      }

      
      void Fitter::setMasks()
      {

// 	  // mask the beam parameters
// 	  //	  std::cout << "Mask values:\n";
// 	  for(unsigned int g=0;g<this->itsNumGauss;g++){
// 	    this->itsFitter.mask(g,3) = false;
// 	    this->itsFitter.mask(g,4) = false;
// 	    this->itsFitter.mask(g,5) = false;
// 	    // 	    for(int i=0;i<6;i++) this->itsFitter.mask(g,i)=false;
// 	    //	    for(int i=0;i<6;i++) this->itsFitter.mask(g,i) = !this->itsFitter.mask(g,i);
// 	    //	    for(int i=0;i<6;i++) std::cout << this->itsFitter.mask(g,i);
// 	    //	    std::cout << "\n";
// 	  }	      
      }


      void Fitter::fit(casa::Matrix<casa::Double> pos, casa::Vector<casa::Double> f,
		       casa::Vector<casa::Double> sigma)
      {

	this->itsBoxFlux = 0.;
	for(uint i=0;i<f.size();i++) this->itsBoxFlux += f(i);

	this->itsSolution.resize();
	  bool thisFitGood = true;
	  for(int fitloop=0;fitloop<3;fitloop++){
	    try {
	      this->itsSolution = this->itsFitter.fit(pos, f, sigma, this->itsMaxRMS);
	    } catch (AipsError err) {
	      std::string message = err.getMesg().chars();
	      message = "FIT ERROR: " + message;
	      ASKAPLOG_ERROR(logger, message);
	      thisFitGood = false;
	    }
	    for(unsigned int i=0;i<this->itsNumGauss;i++){
	      this->itsSolution(i,5) = remainder(this->itsSolution(i,5), 2.*M_PI);
	    }
	    ASKAPLOG_INFO_STR(logger,  "Int. Solution #" << fitloop+1
			      <<": chisq=" << this->itsFitter.chisquared()
			      <<": Parameters are:"); 
	    logparameters(this->itsSolution);
	    if(!this->itsFitter.converged()) fitloop=9999;
	    else this->itsFitter.setFirstEstimate(this->itsSolution);
	  }


	  for(unsigned int i=0;i<this->itsNumGauss;i++){
	    this->itsSolution(i,5) = remainder(this->itsSolution(i,5), 2.*M_PI);
	  }

	  this->itsNDoF = f.size() - this->itsNumGauss*6 - 1;
	  this->itsRedChisq = this->itsFitter.chisquared() / float(this->itsNDoF);
	
	  cout.precision(6);
	  if(this->itsFitter.converged()){
	    ASKAPLOG_INFO_STR(logger, "Fit converged. Solution Parameters follow: "); 
	    logparameters(this->itsSolution);
	  }
	  else ASKAPLOG_INFO_STR(logger, "Fit did not converge");

	  std::stringstream outmsg;
	  outmsg << "Num Gaussians = " << this->itsNumGauss;
	  if( this->itsFitter.converged()) outmsg << ", Converged";
	  else outmsg << ", Failed";
	  outmsg << ", chisq = " << this->itsFitter.chisquared()
		 << ", chisq/nu =  "  << this->itsRedChisq
		 << ", dof = " << this->itsNDoF
		 << ", RMS = " << this->itsFitter.RMS();
	  ASKAPLOG_INFO_STR(logger, outmsg.str());

      }


      bool Fitter::acceptable(RadioSource *src)
      {

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

	  passConv  = this->itsFitter.converged();
	  passConv  = passConv && (this->itsFitter.chisquared()>0.);

	  passChisq = false;
	  passXLoc = passYLoc = passFlux = passSep = passPeak = passIntFlux = true;

	  if(passConv){

	    if(this->itsNDoF<343)
	      passChisq = chisqProb(this->itsNDoF,this->itsFitter.chisquared()) > this->itsChisqCutoff;
	    else 
	      passChisq = (this->itsRedChisq < 1.2);
	    
	    float intFlux = 0.;
	    for(unsigned int i=0;i<this->itsNumGauss;i++){
	      passXLoc = passXLoc && (this->itsSolution(i,1)>src->boxXmin()) && 
		(this->itsSolution(i,1)<src->boxXmax());
	      passYLoc = passYLoc && (this->itsSolution(i,2)>src->boxYmin()) && 
		(this->itsSolution(i,2)<src->boxYmax());
	      passFlux = passFlux && (this->itsSolution(i,0) > 0.);
	      passFlux = passFlux && (this->itsSolution(i,0) > 0.5*src->detectionThreshold());
 	      passPeak = passPeak && (this->itsSolution(i,0) < 2.*src->getPeakFlux());	    
	      
	      Gaussian2D<Double> component(this->itsSolution(i,0),this->itsSolution(i,1),this->itsSolution(i,2),
					   this->itsSolution(i,3),this->itsSolution(i,4),this->itsSolution(i,5));
	      intFlux += component.flux();
	      
	      for(unsigned int j=i+1;j<this->itsNumGauss;j++){
		float sep = hypot( this->itsSolution(i,1)-this->itsSolution(j,1) , 
				   this->itsSolution(i,2)-this->itsSolution(j,2) );
		passSep = passSep && (sep > 2.);
	      }
	    }
	    
	    passIntFlux = (intFlux < 2.*this->itsBoxFlux);

	  }

	  ASKAPLOG_INFO_STR(logger,"Passes: "<<passConv<<passChisq<<passXLoc<<passYLoc<<passSep
			    <<passFlux<<passPeak<<passIntFlux);

	  bool thisFitGood = passConv && passChisq && passXLoc && passYLoc && passSep && 
	    passFlux && passPeak && passIntFlux;

	  return thisFitGood;
      }


      std::multimap<double,int> Fitter::peakFluxList()
      {

	std::multimap<double,int> fitMap;
	for(uint i=0;i<this->itsNumGauss;i++) fitMap.insert(std::pair<double,int>(this->itsSolution(i,0),i));
	return fitMap;

      }

      casa::Gaussian2D<casa::Double> Fitter::gaussian(int num)
      {
	casa::Gaussian2D<casa::Double> 
	  gauss(this->itsSolution(num,0),
		this->itsSolution(num,1),this->itsSolution(num,2),
		this->itsSolution(num,3),this->itsSolution(num,4),this->itsSolution(num,5));
	return gauss;
      }

    }

  }

}
