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
#ifndef ASKAP_SIMS_FSCONT_H_
#define ASKAP_SIMS_FSCONT_H_

#include <modelcomponents/Spectrum.h>
#include <modelcomponents/Continuum.h>
#include <modelcomponents/ContinuumS3SEX.h>

namespace askap {

  namespace analysisutilities {

    const double C=299279458.;
    const float POLREFFREQ=1.42e9;  // Reference frequency for polarisation angle - each component of a source has the same angle at this freq

    /// @brief A class to hold spectral information for a continuum spectrum with polarisation.
    /// @details This class holds information on the continuum
    /// properties of a spectral profile that also contains
    /// polarisation information. Everything is inherited from Continuum, and
    /// new items are the fluxes of various Stokes parameters and the Rotation
    /// Measure.

    class FullStokesContinuum : public ContinuumS3SEX {
    public:

      /// @brief Default constructor
      FullStokesContinuum();
      /// @brief Constructor from ContinuumS3SEX object
      FullStokesContinuum(ContinuumS3SEX &s);
      /// @brief Constructor from Continuum object
      FullStokesContinuum(Continuum &s);
      /// @brief Constructor from Spectrum object
      FullStokesContinuum(Spectrum &s);
      /// @brief Set up parameters using a line of input from an ascii file
      FullStokesContinuum(std::string &line);
      /// @brief Destructor
      virtual ~FullStokesContinuum() {};
      /// @brief Copy constructor for FullStokesContinuum.
      FullStokesContinuum(const FullStokesContinuum& f);

      /// @brief Assignment operator for FullStokesContinuum.
      FullStokesContinuum& operator= (const FullStokesContinuum& c);
      /// @brief Assignment operator for FullStokesContinuum, using a ContinuumS3SEX object
      FullStokesContinuum& operator= (const ContinuumS3SEX& c);
      /// @brief Assignment operator for FullStokesContinuum, using a Continuum object
      FullStokesContinuum& operator= (const Continuum& c);
      /// @brief Assignment operator for FullStokesContinuum, using a Spectrum object
      FullStokesContinuum& operator= (const Spectrum& c);

      /// @brief Define using a line of input from an ascii file
      void define(const std::string &line);

      double flux(double freq, int istokes);
      //      double flux(double freq1, double freq2, int istokes);

      double polAngle(){return itsPolAngleRef;};

      void print(std::ostream& theStream);
      friend std::ostream& operator<<(std::ostream &theStream, FullStokesContinuum &stokes);

    protected:
		
      int    itsClusterID;
      int    itsSFtype;
      int    itsAGNtype;
      double itsDistance;
      double itsRedshift;
      double itsCosVA;

      double itsStokesRefFreq;
      double itsStokesIref;
      double itsStokesQref;
      double itsStokesUref;
      double itsStokesVref;
      double itsPolFluxRef;
      double itsPolFracRef;
      double itsPolAngleRef;
      double itsRM;
      double itsRMflag;

    };

  }

}

#endif
