/// @file
///
/// Obtaining image statistics through distributed processing
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
#include <parallelanalysis/ParallelStats.h>

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <askapparallel/AskapParallel.h>
#include <mathsutils/MathsUtils.h>
#include <duchamp/Cubes/cubes.hh>

#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
using namespace LOFAR::TYPES;


///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".parallelstats");

namespace askap {

namespace analysis {

ParallelStats::ParallelStats(askap::askapparallel::AskapParallel& comms,
                             duchamp::Cube *cube):
    itsComms(&comms), itsCube(cube)
{
}

void ParallelStats::findDistributedStats()
{
    if (itsComms->isParallel()) {
        ASKAPLOG_INFO_STR(logger, "Finding stats via distributed analysis.");
        this->findMeans();
        this->combineMeans();
        this->broadcastMean();
        this->findStddevs();
        this->combineStddevs();
    }
}

void ParallelStats::findMeans()
{

    if (itsComms->isWorker()) {

        if (itsCube->pars().getFlagATrous()) {
            itsCube->ReconCube();
        } else if (itsCube->pars().getFlagSmooth()) {
            itsCube->SmoothCube();
        }

        int32 size = 0;
        float mean = 0.;
        if (!itsCube->pars().getFlagStatSec() ||
                itsCube->pars().statsec().isValid()) {

            float *array = 0;
            // make a mask in case there are blank pixels.
            std::vector<bool> mask = itsCube->pars().makeStatMask(itsCube->getArray(),
                                     itsCube->getDimArray());
            for (size_t i = 0; i < itsCube->getSize(); i++) {
                if (mask[i]) {
                    size++;
                }
            }

            if (size > 0) {
                if (itsCube->pars().getFlagATrous()) {
                    array = itsCube->getArray();
                } else if (itsCube->pars().getFlagSmooth()) {
                    array = itsCube->getRecon();
                } else {
                    array = itsCube->getArray();
                }

                // calculate mean
                if (itsCube->pars().getFlagRobustStats()) {
                    mean = findMedian<float>(array, mask, itsCube->getSize());
                } else {
                    mean = findMean<float>(array, mask, itsCube->getSize());
                }
            }
            ASKAPLOG_INFO_STR(logger, "Mean (Worker #" << itsComms->rank() << ") = " << mean);
        } else {
            // No good points in the stats section
            mean = 0.;
        }
        double dmean = mean;
        LOFAR::BlobString bs;
        bs.resize(0);
        LOFAR::BlobOBufString bob(bs);
        LOFAR::BlobOStream out(bob);
        out.putStart("meanW2M", 1);
        int16 rank = itsComms->rank();
        out << rank << dmean << size;
        out.putEnd();
        itsComms->sendBlob(bs, 0);
    }
}

void ParallelStats::findStddevs()
{

    if (itsComms->isWorker()) {
        // first read in the overall mean for the cube
        double mean = 0;

        LOFAR::BlobString bs1;
        itsComms->receiveBlob(bs1, 0);
        LOFAR::BlobIBufString bib(bs1);
        LOFAR::BlobIStream in(bib);
        int version = in.getStart("meanM2W");
        ASKAPASSERT(version == 1);
        in >> mean;
        in.getEnd();

        // use it to calculate the stddev for this section
        int32 size = 0;
        double stddev = 0.;
        if (itsCube->pars().getFlagStatSec() && !itsCube->pars().statsec().isValid()) {
            // Only way to get here is if flagStatSec=true but statsec
            // is invalid (ie. has no pixels in this worker)
            stddev = 0.;
        } else {
            std::vector<float> array(itsCube->getSize(), 0.);

            for (size_t i = 0; i < itsCube->getSize(); i++) {
                if (itsCube->pars().getFlagATrous()) {
                    // create an array that has the residual
                    // values from the reconstruction
                    array[i] = itsCube->getPixValue(i) - itsCube->getReconValue(i);
                } else if (itsCube->pars().getFlagSmooth()) {
                    array[i] = itsCube->getReconValue(i);
                } else {
                    array[i] = itsCube->getPixValue(i);
                }
            }

            std::vector<bool> mask = itsCube->pars().makeStatMask(array.data(),
                                     itsCube->getDimArray());

            for (size_t i = 0; i < itsCube->getSize(); i++) {
                if (mask[i]) {
                    size++;
                }
            }

            if (size > 0) {
                bool flagRobust = itsCube->pars().getFlagRobustStats();
                stddev = analysisutilities::findSpread(flagRobust, mean, array, mask);
            }

            ASKAPLOG_INFO_STR(logger, "StdDev (Worker #" << itsComms->rank() << ") = " << stddev);
        }

        // return it to the master
        LOFAR::BlobString bs;
        bs.resize(0);
        LOFAR::BlobOBufString bob(bs);
        LOFAR::BlobOStream out(bob);
        out.putStart("stddevW2M", 1);
        int16 rank = itsComms->rank();
        out << rank << stddev << size;
        out.putEnd();
        itsComms->sendBlob(bs, 0);
    }
}

void ParallelStats::combineMeans()
{

    if (itsComms->isMaster()) {
        // get the means from the workers
        LOFAR::BlobString bs;
        int64 size = 0;
        double av = 0;

        for (int i = 1; i < itsComms->nProcs(); i++) {
            itsComms->receiveBlob(bs, i);
            LOFAR::BlobIBufString bib(bs);
            LOFAR::BlobIStream in(bib);
            int version = in.getStart("meanW2M");
            ASKAPASSERT(version == 1);
            double newav;
            int32 newsize;
            int16 rank;
            in >> rank >> newav >> newsize;
            in.getEnd();
            if (newsize > 0) {
                size += newsize;
                av += newav * newsize;
            }
        }

        if (size > 0) {
            av /= double(size);
        }

        ASKAPLOG_INFO_STR(logger, "Overall size = " << size);
        ASKAPLOG_INFO_STR(logger, "Overall mean = " << av);

        itsCube->stats().setMean(av);
    }
}

void ParallelStats::broadcastMean()
{
    if (itsComms->isMaster()) {
        double av = itsCube->stats().getMean();
        LOFAR::BlobString bs;
        bs.resize(0);
        LOFAR::BlobOBufString bob(bs);
        LOFAR::BlobOStream out(bob);
        out.putStart("meanM2W", 1);
        out << av;
        out.putEnd();
        for (int i = 1; i < itsComms->nProcs(); ++i) {
            itsComms->sendBlob(bs, i);
        }
    }
}

void ParallelStats::combineStddevs()
{

    if (itsComms->isMaster()) {
        // get the means from the workers
        LOFAR::BlobString bs;
        int64 size = 0;
        double stddev = 0;

        for (int i = 1; i < itsComms->nProcs(); i++) {
            itsComms->receiveBlob(bs, i);
            LOFAR::BlobIBufString bib(bs);
            LOFAR::BlobIStream in(bib);
            int version = in.getStart("stddevW2M");
            ASKAPASSERT(version == 1);
            double newstddev;
            int32 newsize;
            int16 rank;
            in >> rank >> newstddev >> newsize;
            in.getEnd();
            if (newsize > 0) {
                size += newsize;
                stddev += (newstddev * newstddev * (newsize - 1));
            }
        }

        if (size > 0) {
            stddev = sqrt(stddev / double(size - 1));
        }

        itsCube->stats().setStddev(stddev);
        itsCube->stats().setRobust(false);
        itsCube->stats().define(itsCube->stats().getMiddle(), 0.F,
                                itsCube->stats().getSpread(), 1.F);

        if (!itsCube->pars().getFlagUserThreshold()) {
            ASKAPLOG_INFO_STR(logger, "Setting threshold to be " <<
                              itsCube->pars().getCut() << " sigma");
            itsCube->stats().setThresholdSNR(itsCube->pars().getCut());
            ASKAPLOG_INFO_STR(logger, "Threshold now " << itsCube->stats().getThreshold() <<
                              " since middle = " << itsCube->stats().getMiddle() <<
                              " and spread = " << itsCube->stats().getSpread());
            itsCube->pars().setFlagUserThreshold(true);
            itsCube->pars().setThreshold(itsCube->stats().getThreshold());
        }

        ASKAPLOG_INFO_STR(logger, "Overall StdDev = " << stddev);
    }
}

void ParallelStats::printStats()
{
/// @todo Write the printStats function!
}

}

}
