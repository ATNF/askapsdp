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
#include <outputs/AskapVOTableCatalogueWriter.h>
#include <outputs/AskapAsciiCatalogueWriter.h>
#include <outputs/AskapComponentParsetWriter.h>
#include <outputs/CataloguePreparation.h>

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
    itsParset(finder->parset()), itsCube(finder->cube()),
    itsSourceList(finder->rSourceList()), itsFitParams(*finder->fitParams())
{
}

void ResultsWriter::setCube(duchamp::Cube &cube)
{
    itsCube = cube;
}

void ResultsWriter::setParset(LOFAR::ParameterSet &parset)
{
    itsParset = parset;
}

void ResultsWriter::setSourceList(std::vector<sourcefitting::RadioSource> &srclist)
{
    itsSourceList = srclist;
}

void ResultsWriter::setFitParams(sourcefitting::FittingParameters &fitparams)
{
    itsFitParams = fitparams;
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
        std::string filename = itsCube.pars().getOutFile();
        filename.replace(filename.rfind(".txt"),
                         std::string::npos, ".islands.xml");
        ASKAPLOG_INFO_STR(logger, "Writing the island catalogue to " << filename);
        duchamp::Catalogues::CatalogueSpecification islandColumns =
            IslandCatalogue(itsCube.header());
        islandColumns.checkAll(itsCube.ObjectList(), itsCube.header());

        AskapVOTableCatalogueWriter vowriter(filename);
        vowriter.setup(&itsCube);
        vowriter.setEntryType(ISLAND);
        vowriter.setFitType("best");
        ASKAPLOG_DEBUG_STR(logger, "Writing island table to the VOTable " <<
                           filename);
        vowriter.setColumnSpec(&islandColumns);
        vowriter.setSourceList(&itsSourceList);
        vowriter.openCatalogue();
        vowriter.writeHeader();
        std::string tableVersion = "casda.continuum_island_description_v0.5";
        duchamp::VOParam version("table_version", "meta.version", "char",
                                 tableVersion, 39, "");
        vowriter.writeParameter(version);
        vowriter.writeParameters();
        vowriter.writeStats();
        vowriter.writeTableHeader();
        vowriter.writeEntries();
        vowriter.writeFooter();
        vowriter.closeCatalogue();
    }

}

void ResultsWriter::writeFrequencyParam(AskapVOTableCatalogueWriter &vowriter)
{
    double ra, dec, freq;
    int spec = itsCube.header().WCS().spec;
    if (spec >= 0) { // if there is a spectral axis, write the frequency of the image
        itsCube.header().pixToWCS(itsCube.getDimX() / 2., itsCube.getDimY() / 2., 0.,
                                  ra, dec, freq);
        std::string frequnits(itsCube.header().WCS().cunit[spec]);
        duchamp::VOParam freqParam("Reference frequency", "em.freq;meta.main", "float",
                                   freq, 0, frequnits);
        vowriter.writeParameter(freqParam);
    }
}

void ResultsWriter::writeComponentCatalogue()
{
    if (itsFlag2D) {
        duchamp::Catalogues::CatalogueSpecification casdaColumns =
            ComponentCatalogue(itsCube.header());
        setupCols(casdaColumns, itsSourceList, "best");

        std::string filename = itsCube.pars().getOutFile();
        filename = filename.replace(filename.rfind(".txt"),
                                    std::string::npos, ".components.xml");

        ASKAPLOG_DEBUG_STR(logger,
                           "Writing CASDA-style Fit results to the VOTable " << filename);

        AskapVOTableCatalogueWriter vowriter(filename);
        vowriter.setup(&itsCube);
        vowriter.setFitType("best");
        vowriter.setColumnSpec(&casdaColumns);
        vowriter.setSourceList(&itsSourceList);
        vowriter.openCatalogue();
        vowriter.writeHeader();
        std::string tableVersion = "casda.continuum_component_description_v1.6";
        duchamp::VOParam version("table_version", "meta.version", "char", tableVersion, 42, "");
        vowriter.writeParameter(version);
        vowriter.writeParameters();
        writeFrequencyParam(vowriter);
        vowriter.writeStats();
        vowriter.writeTableHeader();
        vowriter.writeEntries();
        vowriter.writeFooter();
        vowriter.closeCatalogue();

    }

}

void ResultsWriter::writeFitResults()
{
    if (itsFitParams.doFit()) {

        std::vector<std::string> outtypes = this->itsFitParams.fitTypes();
        outtypes.push_back("best");

        for (size_t t = 0; t < outtypes.size(); t++) {

            duchamp::Catalogues::CatalogueSpecification columns =
                fullCatalogue(itsCube.getFullCols(), itsCube.header());
            setupCols(columns, itsSourceList, outtypes[t]);

            std::string filename = itsParset.getString("fitResultsFile", "selavy-fitResults.txt");
            filename = sourcefitting::convertSummaryFile(filename, outtypes[t]);

            AskapAsciiCatalogueWriter writer(filename);
            ASKAPLOG_DEBUG_STR(logger, "Writing Fit results to " << filename);
            writer.setup(&itsCube);
            writer.setFitType(outtypes[t]);
            writer.setColumnSpec(&columns);
            writer.setSourceList(&itsSourceList);
            writer.openCatalogue();
            writer.writeTableHeader();
            writer.writeEntries();
            writer.writeFooter();
            writer.closeCatalogue();

            filename = filename.replace(filename.rfind(".txt"), 4, ".xml");

            AskapVOTableCatalogueWriter vowriter(filename);
            vowriter.setup(&itsCube);
            vowriter.setFitType(outtypes[t]);
            ASKAPLOG_DEBUG_STR(logger, "Writing Fit results to the VOTable " << filename);
            vowriter.setColumnSpec(&columns);
            vowriter.setSourceList(&itsSourceList);
            vowriter.openCatalogue();
            vowriter.writeHeader();
            vowriter.writeParameters();
            if (itsFlag2D) {
                writeFrequencyParam(vowriter);
            }
            vowriter.writeStats();
            vowriter.writeTableHeader();
            vowriter.writeEntries();
            vowriter.writeFooter();
            vowriter.closeCatalogue();
        }

    }

}

void ResultsWriter::writeFitAnnotations()
{
    if (itsFitParams.doFit()) {

        std::string fitAnnotationFile = itsParset.getString("fitAnnotationFile",
                                        "selavy-fitResults.ann");
        std::string fitBoxAnnotationFile = itsParset.getString("fitBoxAnnotationFile",
                                           "selavy-fitResults.boxes.ann");
        bool doBoxAnnot = !itsFitParams.fitJustDetection() &&
                          (fitAnnotationFile != fitBoxAnnotationFile);

        if (this->itsSourceList.size() > 0) {

            for (int i = 0; i < 3; i++) {
                boost::shared_ptr<duchamp::AnnotationWriter> writerFit;
                boost::shared_ptr<duchamp::AnnotationWriter> writerBox;
                switch (i) {
                    case 0: //Karma
                        if (this->itsCube.pars().getFlagKarma()) {
                            writerFit = boost::shared_ptr<KarmaAnnotationWriter>(
                                            new KarmaAnnotationWriter(fitAnnotationFile));
                            ASKAPLOG_INFO_STR(logger,
                                              "Writing fit results to karma annotation file: "
                                              << fitAnnotationFile
                                              << " with address of writer = " << writerFit);
                            if (doBoxAnnot)
                                writerBox = boost::shared_ptr<KarmaAnnotationWriter>(
                                                new KarmaAnnotationWriter(fitBoxAnnotationFile));
                            break;
                        case 1://DS9
                            if (this->itsCube.pars().getFlagDS9()) {
                                std::string filename = fitAnnotationFile;
                                size_t loc = filename.rfind(".ann");
                                if (loc == std::string::npos) filename += ".reg";
                                else filename.replace(loc, 4, ".reg");
                                writerFit = boost::shared_ptr<DS9AnnotationWriter>(
                                                new DS9AnnotationWriter(filename));
                                ASKAPLOG_INFO_STR(logger,
                                                  "Writing fit results to DS9 annotation file: "
                                                  << filename
                                                  << " with address of writer = " << writerFit);
                                if (doBoxAnnot) {
                                    filename = fitBoxAnnotationFile;
                                    size_t loc = filename.rfind(".ann");
                                    if (loc == std::string::npos) filename += ".reg";
                                    else filename.replace(loc, 4, ".reg");
                                    writerBox = boost::shared_ptr<DS9AnnotationWriter>(
                                                    new DS9AnnotationWriter(filename));
                                }
                            }
                            break;
                        case 2://CASA
                            if (this->itsCube.pars().getFlagCasa()) {
                                std::string filename = fitAnnotationFile;
                                size_t loc = filename.rfind(".ann");
                                if (loc == std::string::npos) filename += ".crf";
                                else filename.replace(loc, 4, ".crf");
                                writerFit = boost::shared_ptr<CasaAnnotationWriter>(
                                                new CasaAnnotationWriter(filename));
                                ASKAPLOG_INFO_STR(logger,
                                                  "Writing fit results to casa annotation file: "
                                                  << filename
                                                  << " with address of writer = " << writerFit);
                                if (doBoxAnnot) {
                                    filename = fitBoxAnnotationFile;
                                    size_t loc = filename.rfind(".ann");
                                    if (loc == std::string::npos) filename += ".reg";
                                    else filename.replace(loc, 4, ".reg");
                                    writerBox =
                                        boost::shared_ptr<CasaAnnotationWriter>(
                                            new CasaAnnotationWriter(filename));
                                }
                            }
                            break;
                        }
                }

                if (writerFit.get() != 0) {
                    writerFit->setup(&this->itsCube);
                    writerFit->openCatalogue();
                    writerFit->setColourString("BLUE");
                    writerFit->writeHeader();
                    writerFit->writeParameters();
                    writerFit->writeStats();
                    writerFit->writeTableHeader();
                    // writer->writeEntries();

                    if (writerBox.get() != 0) {
                        writerBox->setup(&this->itsCube);
                        writerBox->openCatalogue();
                        writerBox->setColourString("BLUE");
                        writerBox->writeHeader();
                        writerBox->writeParameters();
                        writerBox->writeStats();
                        writerBox->writeTableHeader();
                    }

                    std::vector<sourcefitting::RadioSource>::iterator src;
                    int num = 1;
                    for (src = this->itsSourceList.begin();
                            src < this->itsSourceList.end(); src++) {
                        src->writeFitToAnnotationFile(writerFit.get(), num, true,
                                                      (fitAnnotationFile == fitBoxAnnotationFile));
                        if (doBoxAnnot && writerBox != 0)
                            src->writeFitToAnnotationFile(writerBox.get(), num, false, true);
                        num++;
                    }

                    writerFit->writeFooter();
                    writerFit->closeCatalogue();
                    if (writerBox != 0) {
                        writerBox->writeFooter();
                        writerBox->closeCatalogue();
                    }
                }

                writerFit.reset();
                writerBox.reset();

            }

        }

    }

}

void ResultsWriter::writeComponentParset()
{
    if (itsFitParams.doFit()) {
        std::string filename = itsParset.getString("outputComponentParset", "");
        if (filename != "") {
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
