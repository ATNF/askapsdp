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
#include <simulationutilities/FluxGenerator.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <wcslib/wcs.h>
#include <wcslib/wcsunits.h>
#include <duchamp/Utils/utils.hh>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <utility>
#include <string>
#include <stdlib.h>
#include <math.h>

ASKAP_LOGGER(logger, ".fluxgen");

namespace askap {

    namespace simulations {

      FluxGenerator::FluxGenerator()
      {
	this->itsAlpha = 0.;
	this->itsBeta = 0.;
	this->itsNuZero = 1400.;
	this->itsNChan = 0;
      }

      FluxGenerator::FluxGenerator(const FluxGenerator& f)
      {
	operator=(f);
      }

      FluxGenerator& FluxGenerator::operator= (const FluxGenerator& f)
      {
	if(this == &f) return *this;
	this->itsAlpha      = f.itsAlpha;
	this->itsBeta       = f.itsBeta;
	this->itsNuZero     = f.itsNuZero;
	this->itsFluxZero   = f.itsFluxZero;
	this->itsNChan      = f.itsNChan;
	this->itsFluxValues = f.itsFluxValues;
	return *this;	  
      }
      
      void FluxGenerator::defineSource(float alpha, float beta, float nuZero, float fluxZero)
      {
	this->itsAlpha = alpha;
	this->itsBeta = beta;
	this->itsNuZero = nuZero;
	this->itsFluxZero = fluxZero;
      }

      void FluxGenerator::calcFluxes(double &x, double &y, struct wcsprm *wcs)
      {
	
	if(this->itsNChan==0)
	  ASKAPTHROW(AskapError, "FluxGenerator: Have not set the number of channels in the flux array.");
	
	this->itsFluxValues = std::vector<float>(this->itsNChan);
	double *pix = new double[3];
	double *wld = new double[3];
	pix[0] = x; pix[1] = y;

	for(double z=0; z<this->itsNChan; z++)
	  {
	    pix[2] = z;
	    pixToWCSSingle(wcs,pix,wld);
	    float freq = wld[2];
	    float powerTerm = this->itsAlpha+this->itsBeta*log(freq/this->itsNuZero);
	    this->itsFluxValues[int(z)] = this->itsFluxZero * pow(freq/this->itsNuZero, powerTerm);
// 	    ASKAPLOG_DEBUG_STR(logger, "z=" << z << ", freq = " << freq << ", alpha=" << this->itsAlpha
// 			       << ", beta = " << this->itsBeta << ", freq0 = " << this->itsNuZero
// 			       << ", flux0 = " << this->itsFluxZero);
	  }

	delete [] pix;
	delete [] wld;

      }
      
    }


}
