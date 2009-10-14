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


namespace askap {

    namespace simulations {

      float luminosityDistance(float z, float hubble, float omegaM, float omegaL)
      {
	const int NUMINT=10000;
	float dz = z/float(NUMINT);
	float rr = 0.;
	for(int i=0;i<NUMINT;i++){
	  float zp1 = (i+0.5)*dz + 1;
	  float temp = omegaL + ((1.-omegaL-omegaM)*(zp1*zp1)) + (omegaM*(zp1*zp1*zp1));
	  float drdz = 1./sqrt(temp);
	  rr = rr + drdz*dz;
	}
	
	float dl = rr * (1.+z) * C_kms / hubble;  // luminosity distance in Mpc
	
	return dl;

      }

      float redshiftToDist(float z, float hubble, float omegaM, float omegaL)
      {
	return luminosityDistance(z);
	// return redshiftToVel(z) / hubble;
      }

      float redshiftToVel(float z)
      {
	float v = ((z+1.)*(z+1.) - 1.)/((z+1.)*(z+1.) + 1.);
	return C_kms * v;
      }

      float redshiftToHIFreq(float z)
      {
	return nu0_HI / (z+1);
      }

      float freqToHIVel(float nu)
      {
	float z = nu0_HI/nu - 1.;
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
	theStream << "Profile int. flux=" << prof.itsProfileFlux << "\n";
	return theStream;
      }

      float HIprofile::integratedFlux(float z, float mhi)
      {
	this->itsRedshift = z;
	this->itsMHI = mhi;
	float dist = redshiftToDist(z); // in Mpc
	float intFlux = 4.24e-6 * pow(10.,mhi) / (dist*dist);
	return intFlux;
      }

      void HIprofile::define(GALTYPE type, float z, float mhi, float maj, float min)
      {
	const float twoRootPi = 4. * M_SQRT1_2 / M_2_SQRTPI;  // from math.h: M_SQRT1_2=1/sqrt(2) and M_2_SQRTPI=2/sqrt(pi), 

	this->itsIntFlux=this->integratedFlux(z,mhi);
	this->itsVRot = vrotMin[type] + (vrotMax[type]-vrotMin[type])*random()/(RAND_MAX+1.0);
	if(maj==min) this->itsDeltaVel = 0.01 * this->itsVRot;
	else this->itsDeltaVel = this->itsVRot * sin( acos( min/maj ) );

	this->itsVelZero = redshiftToVel(z);

	this->itsSigmaEdge = normalRandomVariable(doubleHornShape[EDGE_SIG_MEAN],doubleHornShape[EDGE_SIG_SD]);
	this->itsSigmaEdge=std::max(this->itsSigmaEdge,doubleHornShape[EDGE_SIG_MIN]);
	this->itsSigmaEdge=std::min(this->itsSigmaEdge,doubleHornShape[EDGE_SIG_MAX]);
	this->itsMaxVal = 1./ (twoRootPi*this->itsSigmaEdge);
	
	float ampDipFactor = doubleHornShape[DIP_MIN] + (doubleHornShape[DIP_MAX]-doubleHornShape[DIP_MIN])*random()/(RAND_MAX+1.0);
	this->itsDipAmp = ampDipFactor * this->itsMaxVal;
	this->itsSigmaDip = doubleHornShape[DIP_SIG_SCALE] * this->itsDeltaVel;

	this->itsProfileFlux = this->itsMaxVal*twoRootPi*this->itsSigmaEdge + 
	  2. * this->itsMaxVal * this->itsDeltaVel -
	  this->itsDipAmp * twoRootPi * this->itsSigmaDip * erf(this->itsDeltaVel / (M_SQRT2 * this->itsSigmaDip));

      }

      float HIprofile::flux(float nu)
      {

	float flux;
	float vdiff = freqToHIVel(nu) - this->itsVelZero;
	if(vdiff <  (-this->itsDeltaVel) ){
	  float v = vdiff+this->itsDeltaVel;
	  flux = this->itsMaxVal * exp( -(v*v)/(2.*this->itsSigmaEdge*this->itsSigmaEdge));
	}
	else if(vdiff > this->itsDeltaVel) {
	  float v = vdiff - this->itsDeltaVel;
	  flux = this->itsMaxVal * exp( -(v*v)/(2.*this->itsSigmaEdge*this->itsSigmaEdge));
	}
	else {
	  flux = this->itsMaxVal - this->itsDipAmp*exp(-vdiff*vdiff/(2.*this->itsSigmaDip*this->itsSigmaDip)) + 
	    this->itsDipAmp*exp(-this->itsDeltaVel*this->itsDeltaVel/(2.*this->itsSigmaDip*this->itsSigmaDip));
	}

	return flux * this->itsIntFlux / this->itsProfileFlux;

      }


    }

}
