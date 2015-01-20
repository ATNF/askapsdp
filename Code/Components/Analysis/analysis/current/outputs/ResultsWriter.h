/// @file ResultsWriter.h
///
/// Principal class to handle writing of all catalogues and annotation files
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
#ifndef ASKAP_ANALYSIS_RESULTS_H_
#define ASKAP_ANALYSIS_RESULTS_H_

#include <parallelanalysis/DuchampParallel.h>
#include <duchamp/Cubes/cubes.hh>
#include <Common/ParameterSet.h>
#include <sourcefitting/RadioSource.h>
#include <sourcefitting/FittingParameters.h>
#include <outputs/AskapVOTableCatalogueWriter.h>

namespace askap {

namespace analysis {

/// A class to handle the output of all catalogues and
/// annotation/region files. This class enables the writing of .txt
/// and .xml versions of all catalogues (duchamp results, islands, and
/// components in both CASDA and Selavy formats), as well as the
/// Karma, DS9 and CASA annotation files for the islands and
/// components.
class ResultsWriter {
    public:
        /// Initialise with the parset, used to access parameters
        /// defining aspects of the output
        ResultsWriter(DuchampParallel *finder);
        virtual ~ResultsWriter() {};

        /// Set the Duchamp Cube object
        void setCube(duchamp::Cube &cube);

        /// Set the parset, used to access parameters defining aspects
        /// of the output
        void setParset(LOFAR::ParameterSet &parset);

        /// Set the list of RadioSource objects (that include the
        /// components)
        void setSourceList(std::vector<sourcefitting::RadioSource> &srclist);

        /// Set the list of fitting parameters
        void setFitParams(sourcefitting::FittingParameters &fitparams);

        /// Set the flag indicating whether the image is a continuum
        /// image or not
        void setFlag2D(bool flag2D);

        /// Writes the standard Duchamp output files. This includes the
        /// results files and annotation files. These are done with the
        /// standard Duchamp functionality. This will also include the
        /// writing of the binary catalogue and the text-based spectra.
        void duchampOutput();

        /// Writes a single PARAM to the header of the given VOTable that
        /// records the frequency at which the image was taken.
        void writeFrequencyParam(AskapVOTableCatalogueWriter &vowriter);

        void writeIslandCatalogue();

        void writeComponentCatalogue();

        void writeFitResults();

        /// This function writes an annotation file showing the location
        /// and shape of the fitted 2D Gaussian components. It makes use
        /// of the RadioSource::writeFitToAnnotationFile() function. The
        /// file written to is given by the input parameter
        /// fitAnnotationFile.
        void writeFitAnnotations();

        void writeComponentParset();


    protected:
        LOFAR::ParameterSet &itsParset;
        duchamp::Cube &itsCube;
        std::vector<sourcefitting::RadioSource> &itsSourceList;
        sourcefitting::FittingParameters &itsFitParams;
        bool itsFlag2D;
};

}
}

#endif
