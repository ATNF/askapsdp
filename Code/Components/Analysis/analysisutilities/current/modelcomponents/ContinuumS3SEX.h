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
#ifndef ASKAP_SIMS_CONT_S3SEX_H_
#define ASKAP_SIMS_CONT_S3SEX_H_

#include <modelcomponents/Spectrum.h>
#include <modelcomponents/Continuum.h>
#include <vector>

namespace askap {

namespace analysisutilities {

enum SEDTYPE { SIMPLE_POWERLAW, POWERLAW, FIT };
const float freqValuesS3SEX[5] = {151.e6, 610.e6, 1400.e6, 4860.e6, 18000.e6};

/// @brief A class to hold spectral information for a continuum spectrum.
/// @details This class holds information on the continuum
/// properties of a spectral profile. The information kept is the spectral
/// index alpha, the spectral curvature parameter beta, and the
/// normalisation frequency. It inherits the position, shape and flux
/// normalisation from Spectrum.
///
/// The flux at a given frequency is given by the relation:
/// \f$F(\nu) = F(\nu_0) \left(\frac{\nu}{\nu_0}\right)^{\alpha + \beta\log(\nu/\nu_0)} \f$
class ContinuumS3SEX : public Continuum {
    public:
        /// @brief Default constructor
        ContinuumS3SEX();
        /// @brief Constructor from Spectrum object
        ContinuumS3SEX(const Spectrum &s);
        /// @brief Constructor from Continuum object
        ContinuumS3SEX(const Continuum &c);
        /// @brief Set up parameters using a line of input from an ascii file
        /// @details Constructs a Continuum object from a line of
        /// text from an ascii file. Uses the ContinuumS3SEX::define()
        /// function.
        ContinuumS3SEX(const std::string &line, const float nuZero = defaultFreq);
        /// @brief Define parameters directly
        ContinuumS3SEX(const float alpha,
                       const float beta,
                       const float nuZero);
        /// @brief Define parameters directly
        ContinuumS3SEX(const float alpha,
                       const float beta,
                       const float nuZero,
                       const float fluxZero);
        /// @brief Destructor
        virtual ~ContinuumS3SEX() {};
        /// @brief Copy constructor for ContinuumS3SEX.
        ContinuumS3SEX(const ContinuumS3SEX& f);

        /// @brief Assignment operator for Continuum.
        ContinuumS3SEX& operator= (const ContinuumS3SEX& c);
        /// @brief Assignment operator for Continuum, using a Continuum object
        ContinuumS3SEX& operator= (const Continuum& c);
        /// @brief Assignment operator for Continuum, using a Spectrum object
        ContinuumS3SEX& operator= (const Spectrum& c);

        /// @brief Define using a line of input from an ascii file
        /// @details Defines a Continuum object from a line of
        /// text from an ascii file. This line should be formatted in
        /// the correct way to match the output from the appropriate
        /// python script. The columns should accepted by this function are:
        /// RA - DEC - Flux - Alpha - Beta - Major axis - Minor axis - Pos.Angle
        /// (Alpha & Beta are the spectral index & spectral curvature).
        /// @param line A line from the ascii input file
        void define(const std::string &line);

        /// @brief Return the component type. Discs for
        /// structure=lobe(2) or SF disc (4). Point source for
        /// structure=core(1) or hotspot(3)
        virtual const ComponentType type()
        {
            if (itsStructure == 2 || itsStructure == 4) return DISC;
            else return POINT;
        };

        /// @brief Set the type of SED to apply
        void setSEDtype(const SEDTYPE type) {itsSEDtype = type;};
        void defaultSEDtype() {itsSEDtype = FIT;};

        const double I151() {return itsI151;};
        const double I610() {return itsI610;};
        const double I1400() {return itsI1400;};
        const double I4860() {return itsI4860;};
        const double I18000() {return itsI18000;};

        /// @brief Define the flux & spectral slope/curvature based on the catalogue fluxes.
        /// @details Define the values of the flux, the spectral
        /// index (alpha) and curvature (beta), based on the five
        /// flux values provided.
        void prepareForUse();

        using Continuum::print;
        void print(std::ostream& theStream);
        /// @brief Output the parameters for the source
        /// @details Prints a summary of the parameters to the stream
        /// @param theStream The destination stream
        /// @param prof The profile object
        /// @return A reference to the stream
        friend std::ostream& operator<< (std::ostream& theStream, ContinuumS3SEX &cont);

    protected:
        long itsComponentNum;
        long itsGalaxyNum;
        short itsStructure;
        double itsI151;
        double itsI610;
        double itsI1400;
        double itsI4860;
        double itsI18000;

        SEDTYPE itsSEDtype;

        std::vector<float> itsFreqValues;
};

}

}

#endif
