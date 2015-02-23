/// @file
///
/// All that's needed to define a catalogue of Fitted Components
/// (slightly different in form to the CASDA component catalogue).
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
#ifndef ASKAP_ANALYSIS_FIT_CAT_H_
#define ASKAP_ANALYSIS_FIT_CAT_H_

#include <catalogues/CasdaComponent.h>
#include <sourcefitting/RadioSource.h>
#include <Common/ParameterSet.h>
#include <duchamp/Outputs/CatalogueSpecification.hh>
#include <vector>

namespace askap {

namespace analysis {

/// @brief A class holding all necessary information describing a
/// catalogue of fitted Components, with an emphasis on the fit
/// results.
/// @details This class holds both the set of fitted components for a
/// given image as well as the specification detailing how the
/// information should be written to a catalogue. It provides methods
/// to write the information to VOTable and ASCII format files. It
/// differs from the ComponentCatalogue class by focusing on the
/// fitted results and including items like the number of degrees of
/// freedom in the fit. The outputs are what Selavy would
/// traditionally produce in the "fit results" file. This class also
/// provides methods to produce annotation files showing the location
/// of fitted components.
class FitCatalogue {
    public:
        /// Constructor, that calls defineComponents to define the
        /// catalogue from a set of RadioSource object, and defineSpec
        /// to set the column specification. The filenames are set
        /// based on the output file given in the parset.
        FitCatalogue(std::vector<sourcefitting::RadioSource> &srclist,
                     const LOFAR::ParameterSet &parset,
                     duchamp::Cube &cube,
                     const std::string fitType);

        /// Default destructor
        virtual ~FitCatalogue() {};

        /// Check the widths of the columns based on the values within
        /// the catalogue.
        void check();

        /// Write the catalogue to the ASCII & VOTable files (acts as
        /// a front-end to the writeVOT() and writeASCII() functions),
        /// and produce the annotations files (via the
        /// writeAnnotations() function)
        void write();

    protected:
        /// Define the vector list of Components using the input list
        /// of RadioSource objects and the parset. One component is
        /// created for each fitted Gaussian component from each
        /// RadioSource, then added to its Components.
        void defineComponents(std::vector<sourcefitting::RadioSource> &srclist,
                              const LOFAR::ParameterSet &parset);

        /// Define the catalogue specification. This function
        /// individually defines the columns used in describing the
        /// catalogue, using the Duchamp interface.
        void defineSpec();

        /// Writes the catalogue to a VOTable. It has the necessary
        /// header information and a table entry for each Component in
        /// the catalogue.
        void writeVOT();

        /// Writes the catalogue to an ASCII text file that is
        /// human-readable (with space-separated and aligned
        /// columns). It has a commented line (ie. starting with '#')
        /// with the column titles, another with the units, then one
        /// line for each Component.
        void writeASCII();

        /// Write annotation files for use with Karma, DS9 and CASA
        /// viewers. The annotations show the location and size of the
        /// components, drawing them as ellipses where appropriate. The
        /// filenames have the same form as the votable and ascii files,
        /// but with .ann/.reg/.crf suffixes.
        void writeAnnotations();

        /// The fit type that is used. This variable is used to refer to
        /// the correct set of fit results in the RadioSource objects. It
        /// takes one of the following values: best, full, psf, height,
        /// shape. It is passed to the CasdaComponent constructor.
        std::string itsFitType;

        /// The list of catalogued Components.
        std::vector<CasdaComponent> itsComponents;

        /// The specification for the individual columns
        duchamp::Catalogues::CatalogueSpecification itsSpec;

        /// The duchamp::Cube, used to help instantiate the classes to
        /// write out the ASCII and VOTable files.
        duchamp::Cube &itsCube;

        /// The filename of the VOTable output file
        std::string itsVotableFilename;

        /// The filename of the ASCII text output file.
        std::string itsAsciiFilename;

        /// The filename of the Karma annotation file
        std::string itsKarmaFilename;

        /// The filename of the CASA region file
        std::string itsCASAFilename;

        /// The filename of the DS9 region file
        std::string itsDS9Filename;

        /// The version of the catalogue - in this case, the software
        /// version given by ASKAP_PACKAGE_VERSION
        std::string itsVersion;

};

}

}

#endif
