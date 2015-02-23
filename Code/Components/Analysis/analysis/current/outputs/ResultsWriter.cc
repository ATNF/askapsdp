/// @file
///
///
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
#include <outputs/ResultsWriter.h>
#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <parallelanalysis/DuchampParallel.h>
#include <sourcefitting/RadioSource.h>
#include <sourcefitting/FittingParameters.h>
#include <outputs/AskapComponentParsetWriter.h>
#include <outputs/CataloguePreparation.h>
#include <catalogues/IslandCatalogue.h>
#include <catalogues/ComponentCatalogue.h>
#include <catalogues/FitCatalogue.h>

#include <duchamp/Cubes/cubes.hh>
#include <duchamp/Outputs/CatalogueSpecification.hh>
#include <duchamp/Outputs/KarmaAnnotationWriter.hh>
#include <duchamp/Outputs/DS9AnnotationWriter.hh>
#include <duchamp/Outputs/CasaAnnotationWriter.hh>
#include <Common/ParameterSet.h>
#include <boost/shared_ptr.hpp>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".resultsWriter");

using namespace duchamp;

namespace askap {

namespace analysis {

ResultsWriter::ResultsWriter(DuchampParallel *finder):
    itsParset(finder->parset()),
    itsCube(finder->cube()),
    itsSourceList(finder->rSourceList()),
    itsFitParams(finder->fitParams()),
    itsFlag2D(finder->is2D())
{
}

void ResultsWriter::setFlag2D(bool flag2D)
{
    itsFlag2D = flag2D;
}


void ResultsWriter::duchampOutput()
{

    // Write standard Duchamp results file
    ASKAPLOG_INFO_STR(logger, "Writing to output catalogue " << itsCube.pars().getOutFile());
    itsCube.outputCatalogue();

    if (itsCube.pars().getFlagLog() && (itsCube.getNumObj() > 0)) {
        // Write the log summary only if required
        itsCube.logSummary();
    }

    // Write all Duchamp annotation files
    itsCube.outputAnnotations();

    if (itsCube.pars().getFlagVOT()) {
        ASKAPLOG_INFO_STR(logger, "Writing to output VOTable " << itsCube.pars().getVOTFile());
        // Write the standard Duchamp VOTable (not the CASDA islands table!)
        itsCube.outputDetectionsVOTable();
    }

    if (itsCube.pars().getFlagTextSpectra()) {
        ASKAPLOG_INFO_STR(logger, "Saving spectra to text file " <<
                          itsCube.pars().getSpectraTextFile());
        // Write a text file containing identified spectra
        itsCube.writeSpectralData();
    }

    if (itsCube.pars().getFlagWriteBinaryCatalogue() &&
            (itsCube.getNumObj() > 0)) {
        ASKAPLOG_INFO_STR(logger,
                          "Creating binary catalogue of detections, called " <<
                          itsCube.pars().getBinaryCatalogue());
        // Write the standard Duchamp-format binary catalogue.
        itsCube.writeBinaryCatalogue();
    }

}

void ResultsWriter::writeIslandCatalogue()
{
    if (itsFlag2D) {

        IslandCatalogue cat(itsSourceList, itsParset, itsCube);
        cat.check();
        cat.write();

    }

}

void ResultsWriter::writeComponentCatalogue()
{
    if (itsFitParams.doFit()) {

        ComponentCatalogue cat(itsSourceList, itsParset, itsCube);
        cat.check();
        cat.write();

    }

}

void ResultsWriter::writeFitResults()
{
    if (itsFitParams.doFit()) {

        std::vector<std::string> outtypes = itsFitParams.fitTypes();
        outtypes.push_back("best");

        for (size_t t = 0; t < outtypes.size(); t++) {

            FitCatalogue cat(itsSourceList, itsParset, itsCube, outtypes[t]);
            cat.check();
            cat.write();


        }

    }

}

void ResultsWriter::writeFitAnnotations()
{
    if (itsFitParams.doFit()) {

        std::string fitBoxAnnotationFile = itsParset.getString("fitBoxAnnotationFile",
                                           "selavy-fitResults.boxes.ann");
        if (!itsFitParams.fitJustDetection()) {

            if (itsSourceList.size() > 0) {

                for (int i = 0; i < 3; i++) {
                    boost::shared_ptr<duchamp::AnnotationWriter> writerFit;
                    boost::shared_ptr<duchamp::AnnotationWriter> writerBox;
                    std::string filename;
                    size_t loc;
                    switch (i) {
                        case 0: //Karma
                            writerBox = boost::shared_ptr<KarmaAnnotationWriter>(
                                            new KarmaAnnotationWriter(fitBoxAnnotationFile));
                            break;
                        case 1://DS9
                            filename = fitBoxAnnotationFile;
                            loc = filename.rfind(".ann");
                            if (loc == std::string::npos) filename += ".reg";
                            else filename.replace(loc, 4, ".reg");
                            writerBox = boost::shared_ptr<DS9AnnotationWriter>(
                                            new DS9AnnotationWriter(filename));
                            break;
                        case 2://CASA
                            filename = fitBoxAnnotationFile;
                            loc = filename.rfind(".ann");
                            if (loc == std::string::npos) filename += ".reg";
                            else filename.replace(loc, 4, ".reg");
                            writerBox =
                                boost::shared_ptr<CasaAnnotationWriter>(
                                    new CasaAnnotationWriter(filename));
                            break;
                    }

                    if (writerBox.get() != 0) {
                        writerBox->setup(&itsCube);
                        writerBox->openCatalogue();
                        writerBox->setColourString("BLUE");
                        writerBox->writeHeader();
                        writerBox->writeParameters();
                        writerBox->writeStats();
                        writerBox->writeTableHeader();

                        std::vector<sourcefitting::RadioSource>::iterator src;
                        int num = 1;
                        for (src = itsSourceList.begin(); src < itsSourceList.end(); src++) {
                            src->writeFitToAnnotationFile(writerBox, num++, false, true);
                        }

                        writerBox->writeFooter();
                        writerBox->closeCatalogue();
                    }

                    writerBox.reset();

                }

            }

        }

    }
}

void ResultsWriter::writeComponentParset()
{
    if (itsFitParams.doFit()) {
        std::string filename = itsParset.getString("outputComponentParset", "");
        if (filename != "") {
            /// @todo Instantiate the writer from a parset - then don't have to find the flags etc
            AskapComponentParsetWriter pwriter(filename);
            ASKAPLOG_INFO_STR(logger, "Writing Fit results to parset named " << filename);
            pwriter.setup(&itsCube);
            pwriter.setFitType("best");
            pwriter.setSourceList(&itsSourceList);
            std::string param = "outputComponentParset.reportSize";
            pwriter.setFlagReportSize(itsParset.getBool(param, true));
            param = "outputComponentParset.maxNumComponents";
            pwriter.setMaxNumComponents(itsParset.getInt(param, -1));
            pwriter.openCatalogue();
            pwriter.writeTableHeader();
            pwriter.writeEntries();
            pwriter.writeFooter();
            pwriter.closeCatalogue();
        }
    }
}


}

}
