/// @file
///
/// XXX Notes on program XXX
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
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#include <askap_simulations.h>

#include <simulationutilities/HIprofileS3SAX.h>
#include <simulationutilities/HIprofileS3SAX.h>
#include <simulationutilities/SimulationUtilities.h>
#include <iostream>
#include <sstream>
#include <math.h>
#include <stdlib.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

ASKAP_LOGGER(logger, ".hiprofiles3sax");


namespace askap {

    namespace simulations {

      HIprofileS3SAX::HIprofileS3SAX(const HIprofileS3SAX& h):
	HIprofile(h)
      {
	operator=(h);
      }

      HIprofileS3SAX& HIprofileS3SAX::operator= (const HIprofileS3SAX& h)
      {
	if(this == &h) return *this;
	((HIprofile &) *this) = h;
	this->itsFluxPeak = h.itsFluxPeak;
	this->itsFlux0 = h.itsFlux0;
	this->itsWidthPeak = h.itsWidthPeak;
	this->itsWidth50 = h.itsWidth50;
	this->itsWidth20 = h.itsWidth20;
	this->itsIntFlux = h.itsIntFlux;
	this->itsSideFlux = h.itsSideFlux;
	this->itsMiddleFlux = h.itsMiddleFlux;
	this->itsKpar = h.itsKpar;
	return *this;
      }
      
      std::ostream& operator<< ( std::ostream& theStream, HIprofileS3SAX &prof)
      {
	theStream << "HI profile summary:\n";
	theStream << "z=" << prof.itsRedshift << "\n";
	theStream << "M_HI=" << prof.itsMHI << "\n";
	theStream << "Fpeak=" << prof.itsFluxPeak << "\n";
	theStream << "F0=" << prof.itsFlux0 << "\n";
	theStream << "Wpeak=" << prof.itsWidthPeak << "\n";
	theStream << "W50=" << prof.itsWidth50 << "\n";
	theStream << "W20=" << prof.itsWidth20 << "\n";
	theStream << "IntFlux=" << prof.itsIntFlux << "\n";
	theStream << "Side Flux=" << prof.itsSideFlux << "\n";
	theStream << "Middle Flux=" << prof.itsMiddleFlux << "\n";
	theStream << "K[] = [" << prof.itsKpar[0];
	for(int i=1;i<5;i++)
	  theStream << "," << prof.itsKpar[i];
	theStream << "]\n";
	return theStream;
      }

      HIprofileS3SAX::HIprofileS3SAX(std::string &line)
      {
	float maj,min,pa,alpha,beta;
	std::stringstream ss(line);
	ss >> this->itsRA >> this->itsDec >> this->itsIntFlux >> alpha >> beta >> maj >> min >> pa >> this->itsRedshift >> this->itsMHI 
	   >> this->itsFlux0 >> this->itsFluxPeak >> this->itsWidthPeak >> this->itsWidth50 >> this->itsWidth20;
	this->itsComponent.setPeak(this->itsFluxPeak * this->itsIntFlux);
	this->itsComponent.setMajor(maj);
	this->itsComponent.setMinor(min);
	this->itsComponent.setPA(pa);
	this->define();
      }

      void HIprofileS3SAX::define()
      {
	const double lnhalf=log(0.5);
	const double lnfifth=log(0.2);
	this->itsKpar=std::vector<double>(5);
	double a=this->itsFlux0, b=this->itsFluxPeak, c=this->itsWidthPeak, d=this->itsWidth50, e=this->itsWidth20;
	this->itsKpar[0] = 0.25 * (lnhalf*(c*c-e*e) + lnfifth*(d*d-c*c)) / (lnhalf*(c-e) + lnfifth*(d-c));
	this->itsKpar[1] = (0.25*(c*c-d*d) + this->itsKpar[0]*(d-c)) / lnhalf;
	this->itsKpar[2] = b * exp( (2.*this->itsKpar[0]-c)*(2.*this->itsKpar[0]-c)/(4.*this->itsKpar[1]));
	this->itsKpar[3] = c*c*b*b/(4.*(b*b-a*a));
	this->itsKpar[4] = a * sqrt(this->itsKpar[3]);
	
	this->itsSideFlux = (this->itsKpar[2]*sqrt(this->itsKpar[1])/M_2_SQRTPI) * erfc( (0.5*c-this->itsKpar[0])/sqrt(this->itsKpar[1]) );
	this->itsMiddleFlux = 2. * this->itsKpar[4] * atan( c / sqrt(4.*this->itsKpar[3]-c*c) );

      }

      double HIprofileS3SAX::flux(double nu)
      {
	double flux;
	double dvel = freqToHIVel(nu) - redshiftToVel(this->itsRedshift);
	if( fabs(dvel) < 0.5*this->itsWidthPeak)
	  flux = this->itsKpar[4] / sqrt(this->itsKpar[3]-dvel*dvel);
	else{
	  double temp = fabs(dvel)-this->itsKpar[0];
	  flux = this->itsKpar[2] * exp( -temp*temp/this->itsKpar[1] );
	}
	return flux * this->itsIntFlux;
      }


      double HIprofileS3SAX::flux(double nu1, double nu2)
      {
	double f[2],dv[2];
	double c=this->itsWidthPeak;
	int loc[2];
	dv[0] = freqToHIVel(std::max(nu1,nu2)) - redshiftToVel(this->itsRedshift); // lowest relative velocty
	dv[1] = freqToHIVel(std::min(nu1,nu2)) - redshiftToVel(this->itsRedshift); // highest relative velocity
	f[0] = f[1] = 0.;
// 	ASKAPLOG_DEBUG_STR(logger, "Finding flux b/w " << nu1 << " & " << nu2 << " --> or " << dv[0] << " and " << dv[1] << "  (with peaks at +-"<<0.5*this->itsWidthPeak<<")");

	for(int i=0;i<2;i++){
	  if(dv[i] < -0.5*this->itsWidthPeak ){
	    f[i] += (this->itsKpar[2]*sqrt(this->itsKpar[1])/M_2_SQRTPI) * erfc( (0.-dv[i]-this->itsKpar[0])/sqrt(this->itsKpar[1]) );
	    loc[i]=1;
	  }
	  else{
	    f[i] += this->itsSideFlux;
	    if(dv[i] < 0.5*this->itsWidthPeak){
	      f[i] += this->itsKpar[4] * ( atan(dv[i]/sqrt(this->itsKpar[3]-dv[i]*dv[i])) + atan(c/sqrt(4.*this->itsKpar[3]-c*c)) );
	      loc[i]=2;
	    }
	    else {
	      f[i] += this->itsMiddleFlux;
	      f[i] += this->itsSideFlux - (this->itsKpar[2]*sqrt(this->itsKpar[1])/M_2_SQRTPI) * erfc( (dv[i]-this->itsKpar[0])/sqrt(this->itsKpar[1]) );
	      loc[i]=3;
	    }
	  }

	}
	  
	double flux = (f[1]-f[0])/(dv[1]-dv[0]);
//  	ASKAPLOG_DEBUG_STR(logger, "Fluxes: " << f[1] << "  " << f[0] << "  ---> " << flux << "    locations="<<loc[1]<<","<<loc[0]);
	return flux * this->itsIntFlux;
      
      }

    }

}
