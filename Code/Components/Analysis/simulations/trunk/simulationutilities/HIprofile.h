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
#ifndef ASKAP_SIMS_HI_PROFILE_H_
#define ASKAP_SIMS_HI_PROFILE_H_

#include <simulationutilities/Spectrum.h>
#include <iostream>

namespace askap {

    namespace simulations {

      enum SHAPEPARS {EDGE_SIG_MEAN,EDGE_SIG_SD,EDGE_SIG_MIN,EDGE_SIG_MAX,DIP_MIN,DIP_MAX,DIP_SIG_SCALE};
      const float doubleHornShape[7]={12.0,6.0,5.,20.,0.0,0.3,0.3};
      enum GALTYPE {RQAGN,FRI,FRII,SBG,SFG};
      const float vrotMin[5]={0., 0., 0., 20., 40.};
      const float vrotMax[5]={0., 0., 0., 70., 140.};

      const float nu0_HI = 1420405751.786;
      const float C_kms = 299792.458;
      const float HUBBLE_WMAP = 71.;
      const float OMEGAM_WMAP = 0.27;
      const float OMEGAL_WMAP = 0.73;

      float luminosityDistance(float z, float H0=HUBBLE_WMAP, float omegaM=OMEGAM_WMAP, float omegaL=OMEGAL_WMAP);
      float redshiftToDist(float z, float H0=HUBBLE_WMAP, float omegaM=OMEGAM_WMAP, float omegaL=OMEGAL_WMAP);
      float redshiftToVel(float z);
      float redshiftToHIFreq(float z);
      float freqToHIVel(float nu);

      class HIprofile : public Spectrum {
      public:
	HIprofile(){};
	virtual ~HIprofile(){};
	HIprofile(const HIprofile& h);
	HIprofile& operator= (const HIprofile& h);

	float integratedFlux(float z, float mhi);

	void define(GALTYPE type, float z, float mhi, float maj, float min);

	float flux(float nu);

	friend std::ostream& operator<< ( std::ostream& theStream, HIprofile &prof);

      private:
	float itsRedshift;
	float itsMHI;
	float itsVelZero;
	float itsVRot;
	float itsDeltaVel;
	float itsDipAmp;
	float itsSigmaEdge;
	float itsSigmaDip;
	float itsMaxVal;
	float itsIntFlux;
	float itsProfileFlux;
      };

    }

}

#endif
