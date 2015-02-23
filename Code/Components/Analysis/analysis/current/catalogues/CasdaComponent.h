/// @file
///
/// Class for specifying an entry in the Component catalogue
///
/// @copyright (c) 2014 CSIRO
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
#ifndef ASKAP_ANALYSIS_CASDA_COMPONENT_H_
#define ASKAP_ANALYSIS_CASDA_COMPONENT_H_

#include <catalogues/casda.h>
#include <catalogues/CatalogueEntry.h>
#include <sourcefitting/RadioSource.h>
#include <Common/ParameterSet.h>
#include <duchamp/Outputs/CatalogueSpecification.hh>
#include <duchamp/Outputs/columns.hh>
#include <vector>

namespace askap {

namespace analysis {

/// @brief A class defining an entry in the CASDA Component catalogue.
/// @details This class holds all information that will be written to
/// the CASDA component catalogue for a single fitted component. It
/// allows extraction from a RadioSource object and provides methods
/// to write out the Component to a VOTable or other type of catalogue
/// file.
class CasdaComponent : public CatalogueEntry {
    public:
        /// Constructor that builds the Component object from a
        /// RadioSource. It takes a single fitted component, indicated
        /// by the parameter fitNumber, from the fit results given by
        /// the fitType parameter. The parset is used to make the
        /// corresponding Island, to get the Island ID, and is passed
        /// to the CatalogueEntry constructor to get the SB and base
        /// ID.
        CasdaComponent(sourcefitting::RadioSource &obj,
                       const LOFAR::ParameterSet &parset,
                       const unsigned int fitNumber,
                       const std::string fitType = casda::componentFitType);

        /// Default destructor
        virtual ~CasdaComponent() {};

        /// Return the RA (in decimal degrees)
        const float ra();
        /// Return the Declination (in decimal degrees)
        const float dec();

        ///  Print a row of values for the Component into an
        ///  output table. Each column from the catalogue
        ///  specification is sent to printTableEntry for output.
        ///  \param stream Where the output is written
        ///  \param columns The vector list of Column objects
        void printTableRow(std::ostream &stream,
                           duchamp::Catalogues::CatalogueSpecification &columns);

        ///  Print a single value (a column) into an output table. The
        ///  column's correct value is extracted according to the
        ///  Catalogues::COLNAME key in the column given.
        ///  \param stream Where the output is written
        ///  \param column The Column object defining the formatting.
        void printTableEntry(std::ostream &stream,
                             duchamp::Catalogues::Column &column);

        /// Allow the Column provided to check its width against that
        /// required by the value for this Component, and increase its
        /// width if need be. The correct value is chose according to
        /// the COLNAME key. If a key is given that was not expected,
        /// an Askap Error is thrown. Column must be non-const as it
        /// could change.
        void checkCol(duchamp::Catalogues::Column &column);

        /// Perform the column check for all colums in specification.
        void checkSpec(duchamp::Catalogues::CatalogueSpecification &spec);

        /// Write the ellipse showing the component shape to the given
        /// Annotation file. This allows writing to Karma, DS9 or CASA
        /// annotation/region file.
        void writeAnnotation(boost::shared_ptr<duchamp::AnnotationWriter> &writer);

    protected:
        /// The ID of the island that this component came from.
        std::string itsIslandID;
        /// The unique ID for this component
        std::string itsComponentID;
        /// The J2000 IAU-format name
        std::string itsName;
        /// The RA in string format: 12:34:56.7
        std::string itsRAs;
        /// The Declination in string format: 12:34:56.7
        std::string itsDECs;
        /// The RA in decimal degrees
        double itsRA;
        /// The Declination in decimal degrees
        double itsDEC;
        /// The error in the RA value
        double itsRA_err;
        /// The error in the Declination value
        double itsDEC_err;
        /// The frequency of the image
        double itsFreq;
        /// The fitted peak flux of the component
        double itsFluxPeak;
        /// The error on the peak flux
        double itsFluxPeak_err;
        /// The integrated flux (fitted) of the component
        double itsFluxInt;
        /// The error on the integrated flux
        double itsFluxInt_err;
        /// The fitted major axis (FWHM)
        double itsMaj;
        /// The fitted minor axis (FWHM)
        double itsMin;
        /// The position angle of the fitted major axis
        double itsPA;
        /// The error on the fitted major axis
        double itsMaj_err;
        /// The error on the fitted minor axis
        double itsMin_err;
        /// The error on the fitted position angle
        double itsPA_err;
        /// The major axis after deconvolution
        double itsMaj_deconv;
        /// The minor axis after deconvolution
        double itsMin_deconv;
        /// The position angle of the major axis after deconvolution
        double itsPA_deconv;
        /// The chi-squared value from the fit
        double itsChisq;
        /// The RMS of the residual from the fit
        double itsRMSfit;
        /// The fitted spectral index of the component
        double itsAlpha;
        /// The fitted spectral curvature of the component
        double itsBeta;
        /// The local RMS noise of the image surrounding the component
        double itsRMSimage;
        /// A flag indicating whether more than one component was
        /// fitted to the island
        unsigned int itsFlagSiblings;
        /// A flag indicating the parameters of the component are from
        /// the initial estimate, and not the result of the fit
        unsigned int itsFlagGuess;
        /// A yet-to-be-identified quality flag
        unsigned int itsFlag3;
        /// A yet-to-be-identified quality flag
        unsigned int itsFlag4;
        /// A comment string, not used as yet.
        std::string itsComment;

        /// The following are not in the CASDA component catalogue at
        /// v1.7, but are reported in the fit catalogues of Selavy
        /// {
        /// The ID of the component, without the SB and image
        /// identifiers.
        std::string itsLocalID;
        /// The x-pixel location of the centre of the component
        double itsXpos;
        /// The y-pixel location of the centre of the component
        double itsYpos;
        /// The integrated flux of the island from which this
        /// component was derived
        double itsFluxInt_island;
        /// The peak flux of the island from which this component was
        /// derived
        double itsFluxPeak_island;
        /// The number of free parameters in the fit
        unsigned int itsNfree_fit;
        /// The number of degrees of freedom in the fit
        unsigned int itsNDoF_fit;
        /// The number of pixels used in the fit
        unsigned int itsNpix_fit;
        /// The number of pixels in the parent island.
        unsigned int itsNpix_island;
        /// }
};

}

}

#endif
