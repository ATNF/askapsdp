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
#include <simulationutilities/SimulationUtilities.h>

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

  namespace simFITS
  {


    float normalRandomVariable(float mean, float sigma)
    {
      /// @details Simulate a normal random variable from a
      /// distribution with mean given by mean and standard deviation
      /// given by sigma. The variable is simulated via the polar
      /// method.
      /// @param mean The mean of the normal distribution
      /// @param sigma The standard deviation of the normal distribution
      /// @return A random variable.

      float v1,v2,s;
      // simulate a standard normal RV via polar method
      do{
	v1 = 2.*(1.*random())/(RAND_MAX+1.0) - 1.;
	v2 = 2.*(1.*random())/(RAND_MAX+1.0) - 1.;
	s = v1*v1+v2*v2;
      }while(s>1);
      float z = sqrt(-2.*log(s)/s)*v1;
      return z*sigma + mean;
    }
    
    void addGaussian(float *array, std::vector<int> axes, casa::Gaussian2D<casa::Double> gauss)
    {
      /// @details Adds the flux of a given 2D Gaussian to the pixel
      /// array.  Only look at pixels within a box defined by the
      /// distance along the major axis where the flux of the Gaussian
      /// falls below the minimum float value. This uses the MAXFLOAT
      /// constant from math.h.  Checks are made to make sure that
      /// only pixels within the boundary of the array (defined by the
      /// axes vector) are added.
      /// @param array The array of pixel flux values to be added to.
      /// @param axes The dimensions of the array: axes[0] is the x-dimension and axes[1] is the y-dimension
      /// @param gauss The 2-dimensional Gaussian component.

      float majorSigma = gauss.majorAxis()/(4.*M_LN2);
      float zeroPoint = majorSigma * sqrt(-2.*log(1./(MAXFLOAT*gauss.height())));
      int xmin = std::max(int(gauss.xCenter()-zeroPoint),0);
      int xmax = std::min(int(gauss.xCenter()+zeroPoint),axes[0]-1);
      int ymin = std::max(int(gauss.yCenter()-zeroPoint),0);
      int ymax = std::min(int(gauss.yCenter()+zeroPoint),axes[1]-1);

      for(int x=xmin;x<=xmax;x++){
	for(int y=ymin;y<=ymax;y++){
	  Vector<Double> loc(2);
	  loc(0) = x; loc(1) = y;
	  int pix = x + y * axes[0];
	  array[pix] += gauss(loc);
	}
      }

    }



  }

}
