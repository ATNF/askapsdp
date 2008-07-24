/// @file
///
/// Provides utility functions for simulations package 
///
/// @copyright (c) 2007 CSIRO
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
#include <FITS/FITSfile.h>

#include <APS/ParameterSet.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <scimath/Functionals/Gaussian3D.h>
#include <casa/namespace.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <utility>
#include <string>
#include <stdlib.h>
#include <math.h>

ASKAP_LOGGER(logger, ".simutils");

namespace askap
{

  namespace simulations
  {


    float normalRandomVariable(){return normalRandomVariable(0.,1.);}

    float normalRandomVariable(float mean, float sigma)
    {
      float v1,v2,s;
      // simulate a standard normal RV via polar method
      do{
	v1 = 2.*(1.*rand())/(RAND_MAX+1.0) - 1.;
	v2 = 2.*(1.*rand())/(RAND_MAX+1.0) - 1.;
	s = v1*v1+v2*v2;
      }while(s>1);
      float z = sqrt(-2.*log(s)/s)*v1;
      return z*sigma + mean;
    }
    
    float addGaussian(float *array, std::vector<int> axes, casa::Gaussian2D gauss)
    {

      for(int x=0;x<axes[0];x++){
	for(int y=0;y<axes[1];y++){
	  Vector<Double> loc(2);
	  loc(0) = x; loc(1) = y;
	  int pix = x + y * axes[0];
	  array[pix] += gauss(loc);
	}
      }

    }



  }

}
