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

      //==================================

      HIprofile::HIprofile():
	Spectrum()
      {
	this->itsRedshift = 0.;
	this->itsMHI = 0.;
      }

      HIprofile::HIprofile(const HIprofile& h):
	Spectrum(h)
      {
	operator=(h);
      }

      HIprofile& HIprofile::operator= (const HIprofile& h)
      {
	if(this == &h) return *this;
	((Spectrum &) *this) = h;
	this->itsRedshift=h.itsRedshift;
	this->itsMHI=h.itsMHI;
	return *this;
      }
 
      double HIprofile::integratedFlux(double z, double mhi)
      {
	this->itsRedshift = z;
	this->itsMHI = mhi;
	double dist = redshiftToDist(z); // in Mpc
	double intFlux = 4.24e-6 * mhi / (dist*dist);
// 	double intFlux = 4.24e-6 * pow(10.,mhi) / (dist*dist);
	return intFlux;
      }

      std::ostream& operator<< ( std::ostream& theStream, HIprofile &prof)
      {
	theStream << "HI profile summary:\n";
	theStream << "z=" << prof.itsRedshift << "\n";
	theStream << "M_HI=" << prof.itsMHI << "\n";
	return theStream;
      }

    }

}
