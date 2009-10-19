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
#include <simulationutilities/Spectrum.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <wcslib/wcs.h>
#include <duchamp/Utils/utils.hh>

#include <vector>

ASKAP_LOGGER(logger, ".fluxgen");

namespace askap {

    namespace simulations {

      FluxGenerator::FluxGenerator()
      {
	this->itsNChan = 0;
      }

      FluxGenerator::FluxGenerator(int numChan)
      {
	this->itsNChan = numChan;
	this->itsFluxValues = std::vector<float>(numChan,0.);
      }

      FluxGenerator::FluxGenerator(const FluxGenerator& f)
      {
	operator=(f);
      }

      FluxGenerator& FluxGenerator::operator= (const FluxGenerator& f)
      {
	if(this == &f) return *this;
	this->itsNChan      = f.itsNChan;
	this->itsFluxValues = f.itsFluxValues;
	return *this;	  
      }

      void FluxGenerator::setNumChan(int numChan)
      {	
	this->itsNChan = numChan;
	this->itsFluxValues = std::vector<float>(numChan,0.);
      }
      
      void FluxGenerator::addSpectrum(Spectrum &spec, double &x, double &y, struct wcsprm *wcs)
      {
	
	if(this->itsNChan<=0)
	  ASKAPTHROW(AskapError, "FluxGenerator: Have not set the number of channels in the flux array.");
	
	double *pix = new double[3];
	double *wld = new double[3];
	pix[0] = x; pix[1] = y;

	for(double z=0; z<this->itsNChan; z++)
	  {
	    pix[2] = z;
	    pixToWCSSingle(wcs,pix,wld);
	    float freq = wld[2];
	    this->itsFluxValues[int(z)] += spec.flux(freq);
	  }

	delete [] pix;
	delete [] wld;

      }
      
      void FluxGenerator::addSpectrumInt(Spectrum &spec, double &x, double &y, struct wcsprm *wcs)
      {
	
	if(this->itsNChan<=0)
	  ASKAPTHROW(AskapError, "FluxGenerator: Have not set the number of channels in the flux array.");
	
	double *pix = new double[3*this->itsNChan];
	double *wld = new double[3*this->itsNChan];
	for(int i=0;i<this->itsNChan;i++) {
	  pix[3*i+0] = x; 
	  pix[3*i+1] = y;
	  pix[3*i+2] = double(i);
	}
	pixToWCSMulti(wcs,pix,wld,this->itsNChan);
	for(int z=0; z<this->itsNChan; z++)
	  {
	    int i=3*z+2;
	    double df;
	    if(z<this->itsNChan-1) df = fabs(wld[i]-wld[i+3]);
	    else df = fabs(wld[i]-wld[i-3]);
	    ASKAPLOG_DEBUG_STR(logger,"addSpectrumInt: freq="<<wld[i]<<", df="<<df<<", getting flux between "<<wld[i]-df/2.<<" and " <<wld[i]+df/2.);
	    this->itsFluxValues[i] += spec.flux(wld[i]-df/2.,wld[i]+df/2.);
	  }

	delete [] pix;
	delete [] wld;

      }
      
    }


}
