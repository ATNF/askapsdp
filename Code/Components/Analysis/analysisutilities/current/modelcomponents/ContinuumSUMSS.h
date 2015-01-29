/// @file
///
/// Continuum source from the SUMSS catalogue, version 2.1, as obtained from http://www.physics.usyd.edu.au/sifa/Main/SUMSS
///
/// @copyright (c) 2010 CSIRO
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
#ifndef ASKAP_SIMS_CONTSUMSS_H_
#define ASKAP_SIMS_CONTSUMSS_H_

#include <modelcomponents/Continuum.h>

namespace askap {

namespace analysisutilities {

/// @brief A class to hold information for a continuum source
/// taken from the SUMSS.  @details This class is a
/// specialisation of simulations::Continuum, adapted for
/// sources from the Sydney University Molonglo Sky Survey
/// (D. Bock, M.I. Large and E.M. Sadler (1999) AJ 117,
/// 1578-1593), using version 2.1 of the catalogue as
/// described in T. Mauch, T. Murphy, H.J. Buttery, J. Curran,
/// R.W. Hunstead, B. Piestrzynska, J.G. Robertson and
/// E.M. Sadler (2003), MNRAS, 342, 1117-1130
///
/// The flux at a given frequency is given by the relation:
/// \f$F(\nu) = F(\nu_0) \left(\frac{\nu}{\nu_0}\right)^{\alpha + \beta\log(\nu/\nu_0)} \f$
class ContinuumSUMSS : public Continuum {
    public:
        /// @brief Default constructor
        ContinuumSUMSS();
        /// @brief Constructor from Spectrum object
        ContinuumSUMSS(Spectrum &s);
        /// @brief Constructor from Continuum object
        ContinuumSUMSS(Continuum &c);
        /// @brief Set up parameters using a line of input from an ascii file
        /// @details Constructs a Continuum object from a line of
        /// text from an ascii file. Uses the ContinuumSUMSS::define()
        /// function.
        ContinuumSUMSS(std::string &line, float nuZero);
        /// @brief Define parameters directly
        ContinuumSUMSS(float alpha, float beta, float nuZero)
        {
            defineSource(alpha, beta, nuZero);
        };
        /// @brief Define parameters directly
        ContinuumSUMSS(float alpha, float beta, float nuZero, float fluxZero)
        {
            defineSource(alpha, beta, nuZero); setFluxZero(fluxZero);
        };
        /// @brief Destructor
        virtual ~ContinuumSUMSS() {};
        /// @brief Copy constructor for ContinuumSUMSS.
        ContinuumSUMSS(const ContinuumSUMSS& f);

        /// @brief Assignment operator for Continuum.
        ContinuumSUMSS& operator= (const ContinuumSUMSS& c);
        /// @brief Assignment operator for Continuum, using a Continuum object
        ContinuumSUMSS& operator= (const Continuum& c);
        /// @brief Assignment operator for Continuum, using a Spectrum object
        ContinuumSUMSS& operator= (const Spectrum& c);

        /// @brief Define using a line of input from an ascii file
        /// @details Defines a Continuum object from a line of
        /// text from an ascii file. This line should be taken
        /// from the CDS output from an SUMSS query, formatted in
        /// ascii text/plain format.
        ///  @param line A line from the ascii input file
        void define(const std::string &line);

        void print(std::ostream& theStream);
        /// @brief Output the parameters for the source
        /// @details Prints a summary of the parameters to the stream
        /// @param theStream The destination stream
        /// @param prof The profile object
        /// @return A reference to the stream
        friend std::ostream& operator<< (std::ostream& theStream, ContinuumSUMSS &cont);

    protected:

        std::string itsInputLine;
        std::string itsRAh;
        std::string itsRAm;
        std::string itsRAs;
        std::string itsDECd;
        std::string itsDECm;
        std::string itsDECs;
        float itsRAerr;
        float itsDECerr;
        float itsPeakFlux;
        float itsPeakFluxErr;
        float itsTotalFlux;
        float itsTotalFluxErr;
        float itsFittedMajorAxis;
        float itsFittedMinorAxis;
        float itsFittedPositionAngle;
        float itsDeconvMajorAxis;
        float itsDeconvMinorAxis;
        std::string itsDeconvPositionAngleString;
        std::string itsMosaicName;
        int itsNumMosaics;
        float itsXpos;
        float itsYpos;

};

}

}

#endif
