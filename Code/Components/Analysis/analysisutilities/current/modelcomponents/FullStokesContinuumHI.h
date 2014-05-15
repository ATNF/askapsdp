/// @file
///
/// Defines properties of continuum sources that have polarisation information
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
#ifndef ASKAP_SIMS_FSHI_H_
#define ASKAP_SIMS_FSHI_H_

#include <modelcomponents/FullStokesContinuum.h>
#include <modelcomponents/HIprofileS3SEX.h>

namespace askap {

  namespace analysisutilities {


    GALTYPE getGaltype(int sftype, int agntype);

    /// @brief A class to hold spectral information for a continuum spectrum with polarisation.
    /// @details This class holds information on the continuum
    /// properties of a spectral profile that also contains
    /// polarisation information. Everything is inherited from Continuum, and
    /// new items are the fluxes of various Stokes parameters and the Rotation
    /// Measure.

    class FullStokesContinuumHI : public FullStokesContinuum {
    public:

      /// @brief Default constructor
      FullStokesContinuumHI();
      /// @brief Constructor from FullStokesContinuumHI object
      FullStokesContinuumHI(FullStokesContinuumHI &s);
      /// @brief Constructor from ContinuumS3SEX object
      FullStokesContinuumHI(ContinuumS3SEX &s);
      /// @brief Constructor from Continuum object
      FullStokesContinuumHI(Continuum &s);
      /// @brief Constructor from Spectrum object
      FullStokesContinuumHI(Spectrum &s);
      /// @brief Set up parameters using a line of input from an ascii file
      FullStokesContinuumHI(std::string &line);
      /// @brief Destructor
      virtual ~FullStokesContinuumHI() {};
      /// @brief Copy constructor for FullStokesContinuumHI.
      FullStokesContinuumHI(const FullStokesContinuumHI& f);

      /// @brief Assignment operator for FullStokesContinuumHI.
      FullStokesContinuumHI& operator= (const FullStokesContinuumHI& c);
      /// @brief Assignment operator for FullStokesContinuumHI, using a ContinuumS3SEX object
      FullStokesContinuumHI& operator= (const ContinuumS3SEX& c);
      /// @brief Assignment operator for FullStokesContinuumHI, using a Continuum object
      FullStokesContinuumHI& operator= (const Continuum& c);
      /// @brief Assignment operator for FullStokesContinuumHI, using a Spectrum object
      FullStokesContinuumHI& operator= (const Spectrum& c);

      /// @brief Define using a line of input from an ascii file
      void define(const std::string &line);

      bool freqRangeOK(double freq1, double freq2){return itsHIprofile.freqRangeOK(freq1,freq2);}
      
      double flux(double freq, int istokes){return itsHIprofile.flux(freq,istokes);};
      double fluxInt(double freq1, double freq2, int istokes){return itsHIprofile.fluxInt(freq1,freq2,istokes);};

      void print(std::ostream& theStream);
      friend std::ostream& operator<<(std::ostream &theStream, FullStokesContinuumHI &stokes);

    protected:
		
      HIprofileS3SEX itsHIprofile;
      
    };

  }

}

#endif
