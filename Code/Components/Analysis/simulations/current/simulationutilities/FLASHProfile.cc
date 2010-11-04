/// @file
///
/// Base functions for spectral-line profile classes
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#include <askap_simulations.h>

#include <simulationutilities/FLASHProfile.h>
#include <simulationutilities/GaussianProfile.h>
#include <simulationutilities/SimulationUtilities.h>
#include <simulationutilities/SpectralUtilities.h>
#include <sourcefitting/Component.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>

#include <scimath/Functionals/Gaussian1D.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

ASKAP_LOGGER(logger, ".flashprofile");


namespace askap {

    namespace simulations {

 
        FLASHProfile::FLASHProfile():
                GaussianProfile()
        {
	  this->itsAxisType = FREQUENCY;
	  this->itsFlagContinuumSubtracted = true;
        }

        FLASHProfile::FLASHProfile(double &height, double &centre, double &width, AXISTYPE &type):
	  GaussianProfile(height,centre,width,type)
	{
	  this->itsAxisType = FREQUENCY;
	  this->itsFlagContinuumSubtracted = true;
	}

        FLASHProfile::FLASHProfile(const FLASHProfile& h):
                GaussianProfile(h)
        {
            operator=(h);
        }

        FLASHProfile& FLASHProfile::operator= (const FLASHProfile& h)
        {
            if (this == &h) return *this;

	    ((GaussianProfile &) *this) = h;
            return *this;
        }

        void FLASHProfile::define(std::string &line)
        {
            /// @details Defines a FLASHProfile object from a line of
            /// text from an ascii file. This line should be formatted in
            /// the correct way to match the output from the appropriate
            /// python script. The columns should be: RA - DEC - Flux -
            /// Peak optical depth - central position - FWHM. 
	    /// The flux is used to scale the depth of the Gaussian, and, 
	  /// if itsFlagContinuumSubtracted is true, the component flux 
	  ///is then set to zero.
	  /// The central position is assumed to be in units of redshift.
	  /// The FWHM is assumed to be in units of velocity [km/s], 
	  /// and is converted to redshift.
            /// @param line A line from the ascii input file

	    double flux, maj, min, pa, peak, centre, width;
            std::stringstream ss(line);
            ss >> this->itsRA >> this->itsDec >> flux >> maj >> min >> pa >> peak >> centre >> width;
            this->itsComponent.setPeak(flux);
	    if(maj>=min){
	      this->itsComponent.setMajor(maj);
	      this->itsComponent.setMinor(min);
	    } else{
	      this->itsComponent.setMajor(min);
	      this->itsComponent.setMinor(maj);
	    }
            this->itsComponent.setPA(pa);

	    double depth = (exp(-peak)-1.)*flux;
	    this->itsGaussian.setHeight(depth);

	    double centreFreq = redshiftToFreq(centre,this->itsRestFreq);
	    this->itsGaussian.setCenter(centreFreq);

	    double freqmax=velToFreq(width/2.,centreFreq);
	    double freqmin=velToFreq(-width/2.,centreFreq);
	    this->itsGaussian.setWidth(fabs(freqmax-freqmin));

	    if(this->itsFlagContinuumSubtracted) this->itsComponent.setPeak(0.);

	    ASKAPLOG_DEBUG_STR(logger, "Defined source with Component: " << itsComponent << " and Gaussian " << itsGaussian);

        }

        std::ostream& operator<< (std::ostream& theStream, FLASHProfile &prof)
        {
            /// @details Prints a summary of the parameters to the stream
            /// @param theStream The destination stream
            /// @param prof The profile object
            /// @return A reference to the stream

            theStream << "FLASH profile summary:\n";
            theStream << prof.itsGaussian << "\n";
            return theStream;
        }

    }

}
