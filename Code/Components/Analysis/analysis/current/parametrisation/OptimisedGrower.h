/// @file
///
/// Implementation of WALLABY's recommended algorithms for optimising
/// the mask
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#ifndef ASKAP_ANALYSIS_OPTIMISED_GROWER_H_
#define ASKAP_ANALYSIS_OPTIMISED_GROWER_H_

#include <askap_analysis.h>

#include <duchamp/param.hh>
#include <duchamp/Detection/ObjectGrower.hh>
#include <duchamp/Detection/detection.hh>

#include <Common/ParameterSet.h>

using namespace duchamp;

namespace askap {

  namespace analysis {

    // from http://stackoverflow.com/questions/6823278/c-extend-an-enum-definition
    // -- want to add a new member to ENUM - should update the Duchamp
    // code, but till then...
    static const duchamp::STATE NEW = static_cast<duchamp::STATE>(duchamp::MW+1);

    class OptimisedGrower : public duchamp::ObjectGrower
    {
    public:
      OptimisedGrower();
      OptimisedGrower(const LOFAR::ParameterSet &parset);
      virtual ~OptimisedGrower(){};
  
      void grow(duchamp::Detection *object);

      void setFlag(int x, int y, int z, duchamp::STATE newstate);
      void setFlag(size_t pos, duchamp::STATE newstate){itsFlagArray[pos] = newstate;};
      void setFlag(Voxel vox, duchamp::STATE newstate){setFlag(vox.getX(),vox.getY(),vox.getZ(),newstate);};
      void setMaxIter(int i){maxIterations=i;};
      void setMaxMinZ(int max, int min){zmin=min;zmax=max;};
      void setClobber(bool b){clobberPrevious=b;};
      void findEllipse();
      duchamp::Detection growMask();

    protected:
      double ell_a,ell_b,ell_theta;
      int maxIterations;
      float totalFlux, maxFlux;
      duchamp::Detection *itsObj;
      int xObj, yObj, zmin, zmax;
      bool clobberPrevious;
    };

  }

}

#endif
