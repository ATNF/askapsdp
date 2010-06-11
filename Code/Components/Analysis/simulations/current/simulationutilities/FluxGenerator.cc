/// @file
///
/// Provides mechanism for calculating flux values of a set of spectral channels.
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
#include <simulationutilities/FullStokesContinuum.h>

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
            this->itsNStokes = 1;
        }

      FluxGenerator::FluxGenerator(int numChan, int numStokes)
        {
            ASKAPASSERT(numChan >= 0);
            ASKAPASSERT(numStokes >= 1);
            this->itsNChan = numChan;
            this->itsNStokes = numStokes;
            this->itsFluxValues = std::vector< std::vector<float> >(numStokes);
	    for(int s=0;s<numStokes;s++) this->itsFluxValues[s] = std::vector<float>(numChan, 0.);
        }

        FluxGenerator::FluxGenerator(const FluxGenerator& f)
        {
            operator=(f);
        }

        FluxGenerator& FluxGenerator::operator= (const FluxGenerator& f)
        {
            if (this == &f) return *this;

            this->itsNChan      = f.itsNChan;
            this->itsNStokes    = f.itsNStokes;
            this->itsFluxValues = f.itsFluxValues;
            return *this;
        }

        void FluxGenerator::setNumChan(int numChan)
        {
            ASKAPASSERT(numChan >= 0);
            this->itsNChan = numChan;
            this->itsFluxValues = std::vector< std::vector<float> >(itsNStokes);
	    for(int s=0;s<this->itsNStokes;s++) this->itsFluxValues[s] = std::vector<float>(numChan, 0.);
        }

        void FluxGenerator::setNumStokes(int numStokes)
        {
            ASKAPASSERT(numStokes >= 1);
            this->itsNStokes = numStokes;
            this->itsFluxValues = std::vector< std::vector<float> >(numStokes);
	    if(this->itsNChan>0){
	      for(int s=0;s<this->itsNStokes;s++) this->itsFluxValues[s] = std::vector<float>(this->itsNChan, 0.);
	    }
	}

        void FluxGenerator::addSpectrum(Spectrum &spec, double &x, double &y, struct wcsprm *wcs)
        {
            /// @details This version of the add spectrum function simply
            /// uses the Spectrum object to find the flux at the centre of
            /// each channel. The x & y position are used along with the
            /// WCS specification to find the frequency value of each
            /// channel.
            /// @param spec The spectral profile being used.
            /// @param x The x-pixel location in the flux array
            /// @param y The y-pixel location in the flux array
            /// @param wcs The world coordinate system specfication

            if (this->itsNChan <= 0)
                ASKAPTHROW(AskapError, "FluxGenerator: Have not set the number of channels in the flux array.");

            double *pix = new double[3];
            double *wld = new double[3];
            pix[0] = x; pix[1] = y;

            for (double z = 0; z < this->itsNChan; z++) {
                pix[2] = z;
                pixToWCSSingle(wcs, pix, wld);
                float freq = wld[2];
                this->itsFluxValues[0][int(z)] += spec.flux(freq);
            }

            delete [] pix;
            delete [] wld;

        }

        void FluxGenerator::addSpectrumStokes(FullStokesContinuum &stokes, double &x, double &y, struct wcsprm *wcs)
        {
            /// @details This version of the add spectrum function simply
            /// uses the Spectrum object to find the flux at the centre of
            /// each channel. The x & y position are used along with the
            /// WCS specification to find the frequency value of each
            /// channel.
            /// @param spec The spectral profile being used.
            /// @param x The x-pixel location in the flux array
            /// @param y The y-pixel location in the flux array
            /// @param wcs The world coordinate system specfication
	    /// @todo Improve the polymorphism of this function...

            if (this->itsNChan <= 0)
                ASKAPTHROW(AskapError, "FluxGenerator: Have not set the number of channels in the flux array.");

            double *pix = new double[3];
            double *wld = new double[3];
            pix[0] = x; pix[1] = y;

	    for(int istokes=0; istokes < this->itsNStokes; istokes++){
	      for (double z = 0; z < this->itsNChan; z++) {
                pix[2] = z;
                pixToWCSSingle(wcs, pix, wld);
                float freq = wld[2];
                this->itsFluxValues[istokes][int(z)] += stokes.flux(istokes,freq);
	      }
	    }

            delete [] pix;
            delete [] wld;

        }

        void FluxGenerator::addSpectrumInt(Spectrum &spec, double &x, double &y, struct wcsprm *wcs)
        {
            /// @details This version of the add spectrum function simply
            /// uses the Spectrum object to find the total flux within
            /// each channel. The x & y position are used along with the
            /// WCS specification to find the frequency value of each
            /// channel.
            /// @param spec The spectral profile being used.
            /// @param x The x-pixel location in the flux array
            /// @param y The y-pixel location in the flux array
            /// @param wcs The world coordinate system specfication

            if (this->itsNChan <= 0)
                ASKAPTHROW(AskapError, "FluxGenerator: Have not set the number of channels in the flux array.");

            double *pix = new double[3*this->itsNChan];
            double *wld = new double[3*this->itsNChan];

            for (int i = 0; i < this->itsNChan; i++) {
                pix[3*i+0] = x;
                pix[3*i+1] = y;
                pix[3*i+2] = double(i);
            }

            pixToWCSMulti(wcs, pix, wld, this->itsNChan);

	    for(int istokes=0; istokes < this->itsNStokes; istokes++){
	      for (int z = 0; z < this->itsNChan; z++) {
                int i = 3 * z + 2;
                double df;
		
                if (z < this->itsNChan - 1) df = fabs(wld[i] - wld[i+3]);
                else df = fabs(wld[i] - wld[i-3]);

//      ASKAPLOG_DEBUG_STR(logger,"addSpectrumInt: freq="<<wld[i]<<", df="<<df<<", getting flux between "<<wld[i]-df/2.<<" and " <<wld[i]+df/2.);
                this->itsFluxValues[istokes][z] += spec.flux(wld[i] - df / 2., wld[i] + df / 2.);
	      }
	    }

            delete [] pix;
            delete [] wld;

        }

    }


}
