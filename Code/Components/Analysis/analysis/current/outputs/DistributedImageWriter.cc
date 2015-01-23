/// @file DistributedImageWriter.cc
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

#include <askap_analysis.h>
#include <outputs/ImageWriter.h>
#include <outputs/DistributedImageWriter.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askapparallel/AskapParallel.h>

#include <string>
#include <duchamp/Cubes/cubes.hh>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>
#include <casa/aipstype.h>

#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>


///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".distributedimagewriter");

namespace askap {

namespace analysis {

DistributedImageWriter::DistributedImageWriter(askap::askapparallel::AskapParallel& comms, duchamp::Cube *cube, std::string imageName):
    ImageWriter(cube, imageName), itsComms(&comms)
{

}

void DistributedImageWriter::create()
{
    if (!itsComms->isParallel() || itsComms->isMaster()) {
        // If serial mode, or we're on the master node, create the image
        this->ImageWriter::create();
    }
}

void DistributedImageWriter::write(const casa::Array<casa::Float> &data,
                                   const casa::IPosition &loc, bool accumulate)
{

    if (itsComms->isParallel()) {

        bool OK;
        LOFAR::BlobString bs;

        if (itsComms->isMaster()) {
            for (int i = 1; i < itsComms->nProcs(); i++) {
                // First send the node number
                bs.resize(0);
                LOFAR::BlobOBufString bob(bs);
                LOFAR::BlobOStream out(bob);
                out.putStart("goWrite", 1);
                out << i ;
                out.putEnd();
                itsComms->sendBlob(bs, i);
                // Then wait for the OK from that node
                bs.resize(0);
                itsComms->receiveBlob(bs, i);
                LOFAR::BlobIBufString bib(bs);
                LOFAR::BlobIStream in(bib);
                int version = in.getStart("writeDone");
                ASKAPASSERT(version == 1);
                in >> OK;
                in.getEnd();
                if (!OK) ASKAPTHROW(AskapError, "Staged writing of image failed.");
            }

        } else if (itsComms->isWorker()) {

            OK = true;
            int rank;
            int version;

            if (itsComms->isParallel()) {
                do {
                    bs.resize(0);
                    itsComms->receiveBlob(bs, 0);
                    LOFAR::BlobIBufString bib(bs);
                    LOFAR::BlobIStream in(bib);
                    version = in.getStart("goWrite");
                    ASKAPASSERT(version == 1);
                    in >> rank;
                    in.getEnd();
                    OK = (rank == itsComms->rank());
                } while (!OK);
            }

            if (OK) {
                this->ImageWriter::write(data, loc, accumulate);

                // Return the OK to the master to say that we've written to the image
                if (itsComms->isParallel()) {
                    bs.resize(0);
                    LOFAR::BlobOBufString bob(bs);
                    LOFAR::BlobOStream out(bob);
                    out.putStart("writeDone", 1);
                    out << OK;
                    out.putEnd();
                    itsComms->sendBlob(bs, 0);
                }
            }

        }

    } else {

        this->ImageWriter::write(data, loc);

    }

}

}

}


