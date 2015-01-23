/// @file ExtractionFactory.cc
///
/// Front end handler to deal with all the different types of spectrum/image/cube extraction
///
/// @copyright (c) 2011 CSIRO
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
#include <extraction/ExtractionFactory.h>
#include <askap_analysis.h>
#include <extraction/SourceSpectrumExtractor.h>
#include <extraction/NoiseSpectrumExtractor.h>
#include <extraction/MomentMapExtractor.h>
#include <extraction/CubeletExtractor.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

//System includes
#include <vector>

//ASKAP includes
#include <askapparallel/AskapParallel.h>
#include <sourcefitting/RadioSource.h>

//3rd-party includes
#include <Common/ParameterSet.h>
using namespace LOFAR::TYPES;
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

#include <boost/shared_ptr.hpp>
using boost::shared_ptr;
#include <duchamp/param.hh>

ASKAP_LOGGER(logger, ".extractionfactory");

namespace askap {

namespace analysis {

ExtractionFactory::ExtractionFactory(askap::askapparallel::AskapParallel& comms,
                                     const LOFAR::ParameterSet& parset):
    itsComms(comms), itsParset(parset)
{
    itsParam = 0;
    itsSourceList = std::vector<sourcefitting::RadioSource>();
    itsObjectChoice = std::vector<bool>();
}

void ExtractionFactory::distribute()
{

    if (itsComms.isMaster()) {
        if (itsComms.isParallel()) {
            int16 rank;
            LOFAR::BlobString bs;

            // now send the individual sources to each worker in turn
            for (size_t i = 0;
                    i < itsSourceList.size() + itsComms.nProcs() - 1;
                    i++) {

                rank = i % (itsComms.nProcs() - 1);
                bs.resize(0);
                LOFAR::BlobOBufString bob(bs);
                LOFAR::BlobOStream out(bob);
                out.putStart("extsrc", 1);
                // the first time we write to each worker, send the
                // total number of sources
                if (i / (itsComms.nProcs() - 1) == 0) {
                    out << (unsigned int)(itsSourceList.size());
                }
                out << (i < itsSourceList.size());
                if (i < itsSourceList.size()) {
                    out << itsSourceList[i];
                }
                out.putEnd();
                itsComms.sendBlob(bs, rank + 1);
            }

        }
    }

    if (itsComms.isWorker()) {

        unsigned int totalSourceCount = 0;
        if (itsComms.isParallel()) {

            LOFAR::BlobString bs;
            // now read individual sources
            bool isOK = true;
            itsSourceList.clear();
            while (isOK) {
                sourcefitting::RadioSource src;
                itsComms.receiveBlob(bs, 0);
                LOFAR::BlobIBufString bib(bs);
                LOFAR::BlobIStream in(bib);
                int version = in.getStart("extsrc");
                ASKAPASSERT(version == 1);
                if (totalSourceCount == 0) in >> totalSourceCount;
                in >> isOK;
                if (isOK) {
                    in >> src;
                    itsSourceList.push_back(src);
                }
                in.getEnd();
            }

        } else {
            totalSourceCount = (unsigned int)(itsSourceList.size());
        }

        itsObjectChoice = itsParam->getObjectChoices(totalSourceCount);
    }
}


void ExtractionFactory::extract()
{

    if (itsComms.isWorker()) {

        const unsigned int numTypes = 4;

        std::string parsetNames[numTypes] = {"Spectra", "NoiseSpectra",
                                             "MomentMap", "Cubelet"
                                            };

        for (unsigned int type = 0; type < numTypes; type++) {

            std::string parameter = "extract" + parsetNames[type];
            bool flag = itsParset.getBool(parameter, false);
            if (flag) {
                std::vector<sourcefitting::RadioSource>::iterator src;
                LOFAR::ParameterSet extractSubset = itsParset.makeSubset(parameter + ".");
                ASKAPLOG_INFO_STR(logger, "Beginnging " << parsetNames[type] <<
                                  " extraction for " <<
                                  itsSourceList.size() << " sources");

                for (src = itsSourceList.begin();
                        src < itsSourceList.end();
                        src++) {

                    if (itsObjectChoice.at(src->getID() - 1)) {
                        boost::shared_ptr<SourceDataExtractor> extractor;
                        switch (type) {
                            case 0:
                                extractor = boost::shared_ptr<SourceDataExtractor>(
                                                new SourceSpectrumExtractor(extractSubset));
                                break;
                            case 1:
                                extractor = boost::shared_ptr<SourceDataExtractor>(
                                                new NoiseSpectrumExtractor(extractSubset));
                                break;
                            case 2:
                                extractor = boost::shared_ptr<SourceDataExtractor>(
                                                new MomentMapExtractor(extractSubset));
                                break;
                            case 3:
                                extractor = boost::shared_ptr<SourceDataExtractor>(
                                                new CubeletExtractor(extractSubset));
                                break;
                            default:
                                ASKAPTHROW(AskapError,
                                           "ExtractionFactory - unknown extraction type : " <<
                                           type);
                                break;
                        }

                        extractor->setSource(&*src);
                        extractor->extract();
                        extractor->writeImage();

                    }
                }
            }

        }

    }

}

}


}

