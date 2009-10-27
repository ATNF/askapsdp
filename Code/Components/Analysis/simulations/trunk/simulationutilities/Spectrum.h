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
#ifndef ASKAP_SIMS_SPEC_H_
#define ASKAP_SIMS_SPEC_H_

#include <iostream>
#include <sstream>

#include <sourcefitting/Component.h>

namespace askap {

  namespace simulations {

      class Spectrum {
	public:
	  Spectrum(){};
	  Spectrum(std::string &line);
	  virtual ~Spectrum() {};
	  Spectrum(const Spectrum& s);

	  std::string ra(){return itsRA;};
	  std::string dec(){return itsDec;};
	  double fluxZero(){return itsComponent.peak();};
	  double maj(){return itsComponent.maj();};
	  double min(){return itsComponent.min();};
	  double pa(){return itsComponent.pa();};

	  void setFluxZero(float f){itsComponent.setPeak(f);};
	  void setMaj(float f){itsComponent.setMajor(f);};
	  void setMin(float f){itsComponent.setMinor(f);};
	  void setPA(float f){itsComponent.setPA(f);};
	  
	  virtual double flux(double freq)  {return -77.;};
	  virtual double flux(double freq1, double freq2)  {return -79.;};

	protected:
	  std::string itsRA;
	  std::string itsDec;
	  analysis::sourcefitting::SubComponent itsComponent;

        };

    }

}

#endif
