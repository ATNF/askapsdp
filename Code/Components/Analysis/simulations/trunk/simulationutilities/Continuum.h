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
#ifndef ASKAP_SIMS_CONT_H_
#define ASKAP_SIMS_CONT_H_

#include <simulationutilities/Spectrum.h>

namespace askap {

    namespace simulations {

      class Continuum : public Spectrum {
            public:
                Continuum();
		Continuum(Spectrum &s);
		Continuum(std::string &line);
		Continuum(float alpha, float beta, float nuZero){defineSource(alpha,beta,nuZero);};
		Continuum(float alpha, float beta, float nuZero,float fluxZero){defineSource(alpha,beta,nuZero); setFluxZero(fluxZero);};
                virtual ~Continuum() {};
		/// @brief Copy constructor for Continuum.
		Continuum(const Continuum& f);
		
		/// @brief Assignment operator for Continuum.
		Continuum& operator= (const Continuum& c);
		Continuum& operator= (const Spectrum& c);

		void defineSource(float alpha, float beta, float nuZero);

		void setNuZero(float n){itsNuZero = n;};

		double alpha(){return itsAlpha;};
		double beta(){return itsBeta;};
		double nuZero(){return itsNuZero;};

		double flux(double freq);

            protected:
                double itsAlpha;
                double itsBeta;
                double itsNuZero;

        };

    }

}

#endif
