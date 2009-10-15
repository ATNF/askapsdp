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

#include <simulationutilities/HIprofile.h>
#include <simulationutilities/SimulationUtilities.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

ASKAP_LOGGER(logger, ".hiprofile");


namespace askap {

    namespace simulations {

      double luminosityDistance(double z, double hubble, double omegaM, double omegaL)
      {
	const int NUMINT=10000;
	double dz = z/double(NUMINT);
	double rr = 0.;
	for(int i=0;i<NUMINT;i++){
	  double zp1 = (i+0.5)*dz + 1;
	  double temp = omegaL + ((1.-omegaL-omegaM)*(zp1*zp1)) + (omegaM*(zp1*zp1*zp1));
	  double drdz = 1./sqrt(temp);
	  rr = rr + drdz*dz;
	}
	
	double dl = rr * (1.+z) * C_kms / hubble;  // luminosity distance in Mpc
	
	return dl;

      }

      double redshiftToDist(double z, double hubble, double omegaM, double omegaL)
      {
	return luminosityDistance(z);
	// return redshiftToVel(z) / hubble;
      }

      double redshiftToVel(double z)
      {
	double v = ((z+1.)*(z+1.) - 1.)/((z+1.)*(z+1.) + 1.);
	return C_kms * v;
      }

      double redshiftToHIFreq(double z)
      {
	return nu0_HI / (z+1);
      }

      double freqToHIVel(double nu)
      {
	double z = nu0_HI/nu - 1.;
	return redshiftToVel(z);
      }
      //===============================

      HIprofile::HIprofile(const HIprofile& h)
      {
	operator=(h);
      }

      HIprofile& HIprofile::operator= (const HIprofile& h)
      {
	if(this == &h) return *this;
	this->itsRedshift=h.itsRedshift;
	this->itsMHI=h.itsMHI;
	this->itsVelZero=h.itsVelZero;
	this->itsVRot=h.itsVRot;
	this->itsDeltaVel=h.itsDeltaVel;
	this->itsDipAmp=h.itsDipAmp;
	this->itsSigmaEdge=h.itsSigmaEdge;
	this->itsSigmaDip=h.itsSigmaDip;
	this->itsMaxVal=h.itsMaxVal;
	this->itsIntFlux=h.itsIntFlux;
	this->itsEdgeFlux=h.itsEdgeFlux;
	this->itsMiddleFlux=h.itsMiddleFlux;
	this->itsProfileFlux=h.itsProfileFlux;
	return *this;
      }
      
      std::ostream& operator<< ( std::ostream& theStream, HIprofile &prof)
      {
	theStream << "HI profile summary:\n";
	theStream << "z=" << prof.itsRedshift << "\n";
	theStream << "M_HI=" << prof.itsMHI << "\n";
	theStream << "V_0=" << prof.itsVelZero << "\n";
	theStream << "Vrot=" << prof.itsVRot << "\n";
	theStream << "Vwidth=" << prof.itsDeltaVel << "\n";
	theStream << "Dip Amplitude=" << prof.itsDipAmp << "\n";
	theStream << "Sigma_edge=" << prof.itsSigmaEdge << "\n";
	theStream << "Sigma_dip=" << prof.itsSigmaDip << "\n";
	theStream << "Peak value=" << prof.itsMaxVal << "\n";
	theStream << "Integrated Flux=" << prof.itsIntFlux << "\n";
	theStream << "Edge int. flux=" << prof.itsEdgeFlux << "\n";
	theStream << "Middle int. flux=" << prof.itsMiddleFlux << "\n";
	theStream << "Profile int. flux=" << prof.itsProfileFlux << "\n";
	return theStream;
      }

      double HIprofile::integratedFlux(double z, double mhi)
      {
	this->itsRedshift = z;
	this->itsMHI = mhi;
	double dist = redshiftToDist(z); // in Mpc
	double intFlux = 4.24e-6 * pow(10.,mhi) / (dist*dist);
	return intFlux;
      }

      void HIprofile::define(GALTYPE type, double z, double mhi, double maj, double min)
      {
	const double rootTwoPi = 4. * M_SQRT1_2 / M_2_SQRTPI;  // sqrt(2pi), using, from math.h: M_SQRT1_2=1/sqrt(2) and M_2_SQRTPI=2/sqrt(pi), 

	this->itsIntFlux=this->integratedFlux(z,mhi);
	this->itsVRot = vrotMin[type] + (vrotMax[type]-vrotMin[type])*random()/(RAND_MAX+1.0);
	if(maj==min) this->itsDeltaVel = 0.01 * this->itsVRot;
	else this->itsDeltaVel = this->itsVRot * sin( acos( min/maj ) );

	this->itsVelZero = redshiftToVel(z);

	this->itsSigmaEdge = normalRandomVariable(doubleHornShape[EDGE_SIG_MEAN],doubleHornShape[EDGE_SIG_SD]);
	this->itsSigmaEdge=std::max(this->itsSigmaEdge,doubleHornShape[EDGE_SIG_MIN]);
	this->itsSigmaEdge=std::min(this->itsSigmaEdge,doubleHornShape[EDGE_SIG_MAX]);
	this->itsMaxVal = 1./ (rootTwoPi*this->itsSigmaEdge);
	
	double ampDipFactor = doubleHornShape[DIP_MIN] + (doubleHornShape[DIP_MAX]-doubleHornShape[DIP_MIN])*random()/(RAND_MAX+1.0);
	this->itsDipAmp = ampDipFactor * this->itsMaxVal;
	this->itsSigmaDip = doubleHornShape[DIP_SIG_SCALE] * this->itsDeltaVel;

	this->itsEdgeFlux = 0.5 * this->itsMaxVal*rootTwoPi*this->itsSigmaEdge;
	this->itsMiddleFlux = 2. * this->itsDeltaVel * 
	  (this->itsMaxVal + this->itsDipAmp/exp(this->itsDeltaVel*this->itsDeltaVel/(2.*this->itsSigmaDip*this->itsSigmaDip))) -
	  this->itsDipAmp * rootTwoPi * this->itsSigmaDip * erf(this->itsDeltaVel / (M_SQRT2 * this->itsSigmaDip));

	this->itsProfileFlux = 2.*this->itsEdgeFlux + this->itsMiddleFlux;

      }

      double HIprofile::flux(double nu)
      {

	double flux;
	double vdiff = freqToHIVel(nu) - this->itsVelZero;
	if(vdiff <  (-this->itsDeltaVel) ){
	  double v = vdiff+this->itsDeltaVel;
	  flux = this->itsMaxVal * exp( -(v*v)/(2.*this->itsSigmaEdge*this->itsSigmaEdge));
	}
	else if(vdiff > this->itsDeltaVel) {
	  double v = vdiff - this->itsDeltaVel;
	  flux = this->itsMaxVal * exp( -(v*v)/(2.*this->itsSigmaEdge*this->itsSigmaEdge));
	}
	else {
	  flux = this->itsMaxVal - this->itsDipAmp*exp(-vdiff*vdiff/(2.*this->itsSigmaDip*this->itsSigmaDip)) + 
	    this->itsDipAmp*exp(-this->itsDeltaVel*this->itsDeltaVel/(2.*this->itsSigmaDip*this->itsSigmaDip));
	}

	return flux * this->itsIntFlux / this->itsProfileFlux;

      }


      double HIprofile::flux(double nu1, double nu2)
      {

	const double rootPiOnTwo = 2.* M_SQRT1_2 / M_2_SQRTPI; // sqrt(pi/2), using, from math.h: M_SQRT1_2=1/sqrt(2) and M_2_SQRTPI=2/sqrt(pi), 

	double v[2],f[2];
	v[0] = freqToHIVel(std::max(nu1,nu2)); // lowest velocty
	v[1] = freqToHIVel(std::min(nu1,nu2)); // highest velocity
	f[0] = f[1] = 0.;
	int loc[2];

	double minPeak = this->itsVelZero - this->itsDeltaVel;
	double maxPeak = this->itsVelZero + this->itsDeltaVel;
	ASKAPLOG_DEBUG_STR(logger, "Finding flux b/w " << nu1 << " & " << nu2 << " --> or " << v[0] << " and " << v[1] << "  (with minpeak="<<minPeak<<" and maxpeak="<<maxPeak<<")");
	for(int i=0;i<2;i++){
	  if(v[i] < minPeak ){
	    f[i] += rootPiOnTwo * this->itsMaxVal * this->itsSigmaEdge * erfc( (minPeak-v[i])/(M_SQRT2*this->itsSigmaEdge) );
	    loc[i]=1;
	  }
	  else{
	    f[i] += this->itsEdgeFlux;
	    if(v[i] < maxPeak){
	      double norm = (v[i]-minPeak)*(this->itsMaxVal+this->itsDipAmp/exp(this->itsDeltaVel*this->itsDeltaVel/(2.*this->itsSigmaDip*this->itsSigmaDip)));
	      double dip = rootPiOnTwo*this->itsDipAmp*this->itsSigmaDip * (erfc(-1.*this->itsDeltaVel/(M_SQRT2*this->itsSigmaDip))-
									    erfc((v[i]-this->itsVelZero)/(M_SQRT2*this->itsSigmaDip)));
	      f[i] += (norm-dip);
	      ASKAPLOG_DEBUG_STR(logger, "In loc 2, norm="<<norm<<", dip="<<dip);
	      loc[i]=2;
	    }
	    else {
	      f[i] += this->itsMiddleFlux;
	      f[i] += rootPiOnTwo * this->itsMaxVal * this->itsSigmaEdge * erf( (v[i]-maxPeak)/(M_SQRT2*this->itsSigmaEdge) );
	      loc[i]=3;
	    }
	  }
	}

	double flux = (f[1] - f[0])/(v[1]-v[0]);
	ASKAPLOG_DEBUG_STR(logger, "Fluxes: " << f[1] << "  " << f[0] << "  ---> " << flux << "    locations="<<loc[1]<<","<<loc[0]);
	return flux * this->itsIntFlux / this->itsProfileFlux;

      }


    }

}
