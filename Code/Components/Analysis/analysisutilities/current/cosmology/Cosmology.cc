/// @file
///
/// XXX Notes on program XXX
///
/// @copyright (c) 2011 CSIRO
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
#include <askap_analysisutilities.h>

#include <cosmology/Cosmology.h>
#include <math.h>

namespace askap
{

  namespace analysisutilities
  {

    namespace cosmology
    {

      Cosmology::Cosmology():
	itsHubble(HUBBLE_WMAP),itsOmegaM(OMEGAM_WMAP),itsOmegaL(OMEGAL_WMAP)
      {
      }

      Cosmology::Cosmology(double hubble, double omegaM, double omegaL):
	itsHubble(hubble),itsOmegaM(omegaM),itsOmegaL(omegaL)
      {
      }

      Cosmology::Cosmology(const Cosmology &other)
      {
	this->operator=(other);
      }

      Cosmology& Cosmology::operator=(const Cosmology &other)
      {
	if(this == &other) return *this;
	this->itsHubble = other.itsHubble;
	this->itsOmegaM = other.itsOmegaM;
	this->itsOmegaL = other.itsOmegaL;
	return *this;
      }

      double Cosmology::dlum(double z)
      {
	double dz = z/double(NUMINT);
	double rr = 0.;
	for(int i=0;i<NUMINT;i++){
	  double zp1 = (i+0.5)*dz + 1;
	  double temp = this->itsOmegaL + ((1.-this->itsOmegaL-this->itsOmegaM)*(zp1*zp1)) + 
	    (this->itsOmegaM*(zp1*zp1*zp1));
	  double drdz = 1./sqrt(temp);
	  rr = rr + drdz*dz;
	}
	
	double dl = rr * (1.+z) * (C_ms / 1000.) / this->itsHubble;  /* dlum in Mpc */
	dl = dl * MPC_m;                                  /* dlum in metres */
	
	return log10(dl);
	
      }

      double Cosmology::lum(double z, double flux)
      {
	return log10(4.*M_PI) + this->dlum(z) + flux;
      }

    }

  }

}


