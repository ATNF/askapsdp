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
#ifndef ASKAP_SIMS_CONT_ID_H_
#define ASKAP_SIMS_CONT_ID_H_

#include <modelcomponents/Spectrum.h>
#include <modelcomponents/Continuum.h>

namespace askap {

namespace analysisutilities {

/// @brief A class to hold spectral information for a continuum spectrum.
/// @details This class holds information on the continuum
/// properties of a spectral profile. The information kept is the spectral
/// index alpha, the spectral curvature parameter beta, and the
/// normalisation frequency. It inherits the position, shape and flux
/// normalisation from Spectrum. It is essentially the
/// Continuum class, but with the ability to read in an ID as
/// well as the other basic info.
///
/// The flux at a given frequency is given by the relation:
/// \f$F(\nu) = F(\nu_0) \left(\frac{\nu}{\nu_0}\right)^{\alpha + \beta\log(\nu/\nu_0)} \f$
class ContinuumID : public Continuum {
    public:
        /// @brief Default constructor
        ContinuumID();

        /// @brief Constructor from Spectrum object
        ContinuumID(const Spectrum &s);

        ///Constructs a ContinuumID object from a line of
        /// text from an ascii file. Uses the ContinuumID::define()
        /// function.
        ContinuumID(const std::string &line, const float nuZero = defaultFreq);

        /// @brief Define parameters directly
        ContinuumID(const float alpha,
                    const float beta,
                    const float nuZero);

        /// @brief Define parameters directly
        ContinuumID(const float alpha,
                    const float beta,
                    const float nuZero,
                    const float fluxZero);

        /// @brief Destructor
        virtual ~ContinuumID() {};
        /// @brief Copy constructor for ContinuumID.
        ContinuumID(const ContinuumID& f);

        /// @brief Assignment operator for ContinuumID.
        ContinuumID& operator= (const ContinuumID& c);
        /// @brief Assignment operator for ContinuumID, using a Continuum object
        ContinuumID& operator= (const Continuum& c);
        /// @brief Assignment operator for ContinuumID, using a Spectrum object
        ContinuumID& operator= (const Spectrum& c);

        /// Defines a ContinuumID object from a line of text from an
        /// ascii file. This line should be formatted in the following
        /// way, explicitly setting the ID: ID - RA - DEC - Flux -
        /// Alpha - Beta - Major axis - Minor axis - Pos.Angle (Alpha
        /// & Beta are the spectral index & spectral curvature). ***
        /// The Flux provided in the text file is no longer assumed to
        /// be in log space.***
        /// @param line A line from the ascii input file
        void define(const std::string &line);

        /// @brief Output the parameters for the source
        using Continuum::print;
        void print(std::ostream& theStream) const;
        friend std::ostream& operator<< (std::ostream& theStream, const ContinuumID &cont);

    protected:

};

}

}

#endif
