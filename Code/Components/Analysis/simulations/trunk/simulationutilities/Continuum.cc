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
#include <simulationutilities/Spectrum.h>
#include <simulationutilities/Continuum.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <utility>
#include <string>
#include <stdlib.h>
#include <math.h>

ASKAP_LOGGER(logger, ".continuum");

namespace askap {

    namespace simulations {

      Continuum::Continuum()
      {
	this->itsAlpha = 0.;
	this->itsBeta = 0.;
	this->itsNuZero = 1400.;
	this->itsFluxZero = 1;
      }

      Continuum::Continuum(const Continuum& c)
      {
	operator=(c);
      }

      Continuum& Continuum::operator= (const Continuum& c)
      {
	if(this == &c) return *this;
	this->itsAlpha      = c.itsAlpha;
	this->itsBeta       = c.itsBeta;
	this->itsNuZero     = c.itsNuZero;
	this->itsFluxZero   = c.itsFluxZero;
	return *this;	  
      }
      
      void Continuum::defineSource(float alpha, float beta, float nuZero, float fluxZero)
      {
	this->itsAlpha = alpha;
	this->itsBeta = beta;
	this->itsNuZero = nuZero;
	this->itsFluxZero = fluxZero;
      }
      

      float Continuum::flux(float freq)
      {
	float powerTerm = this->itsAlpha+this->itsBeta*log(freq/this->itsNuZero);
	return this->itsFluxZero * pow(freq/this->itsNuZero, powerTerm);
      }


    }


}
