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

        /// Default destructor
        virtual ~ResultsWriter() {};

        /// Set the flag indicating whether the image is a continuum
        /// image or not
        void setFlag2D(bool flag2D);

        /// Writes the standard Duchamp output files. This includes
        /// the results files and annotation files. These are done
        /// with the standard Duchamp functionality. This will also
        /// include the writing of the binary catalogue and the
        /// text-based spectra.
        void duchampOutput();

        /// Writes out the CASDA island catalogue, using the
        /// IslandCatalogue class to handle the writing (which will
        /// create VOTable and ASCII text versions of the catalogue).
        void writeIslandCatalogue();

        /// Writes out the CASDA component catalogue, using the
        /// ComponentCatalogue class to handle the writing (which will
        /// create VOTable and ASCII text versions of the catalogue).
        void writeComponentCatalogue();

        /// Writes out the catalogue of 2D Gaussian fits, using the
        /// FitCatalogue class to handle the writing (which will
        /// create VOTable and ASCII text versions of the catalogue,
        /// as well as annotation files showing the location of the
        /// fitted components).
        void writeFitResults();

        /// This function writes an annotation file showing the
        /// location and shape of the fitted 2D Gaussian
        /// components. It makes use of the
        /// RadioSource::writeFitToAnnotationFile() function. The file
        /// written to is given by the input parameter
        /// fitAnnotationFile.
        void writeFitAnnotations();

        /// Writes out a parset that details the set of components,
        /// optionally limited to the nth-brightest components. Such a
        /// parset is suitable for use with csimulator or ccalibrator.
        void writeComponentParset();


    protected:

        /// The input parset
        LOFAR::ParameterSet &itsParset;

        /// The Duchamp cube structure
        duchamp::Cube &itsCube;

        /// The list of RadioSource detections
        std::vector<sourcefitting::RadioSource> &itsSourceList;

        /// The Fitting Parameters
        sourcefitting::FittingParameters &itsFitParams;

        /// Whether we are dealing with a 2D image.
        bool itsFlag2D;
};

}
}

#endif
