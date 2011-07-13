//==============================================================
/// @file : testing ways to access Measurement Sets and related
/// information. The specific test is whether we are able to access a
/// given casa image using MPI without the scheduling used in the
/// cduchamp code (i.e. in analysisutilities/CasaImageUtil.cc)
/// The key routines are in the function getSubImage.
//==============================================================
///
/// @copyright (c) 2007 CSIRO
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <mwcommon/AskapParallel.h>

#include <Common/ParameterSet.h>
#include <Common/LofarTypedefs.h>
using namespace LOFAR::TYPES;
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

#include <casa/aipstype.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <images/Images/ImageOpener.h>
#include <lattices/Lattices/LatticeLocker.h>
#include <images/Images/FITSImage.h>
#include <images/Images/SubImage.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Containers/RecordInterface.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <lattices/Lattices/LatticeBase.h>
#include <images/Images/ImageOpener.h>
#include <casa/Utilities/Assert.h>

#include <string>
#include <iostream>

#include <duchamp/Cubes/cubes.hh>
#include <duchamp/Utils/Section.hh>

#include <wcslib/wcs.h>

using namespace casa;
using namespace askap;
using namespace askap::mwcommon;

ASKAP_LOGGER(logger, "tParallelCasaAccess.log");


/// A simple front-end to AskapParallel that allows direct access of
/// the node & rank numbers, plus the connectionSet.
class MyAskapParallel: public askap::mwcommon::AskapParallel {
    public:
        MyAskapParallel(int argc, const char **argv): AskapParallel(argc, argv) {};
        int nnode() {return itsNNode;};
        int rank() {return itsRank;};
        askap::mwcommon::MPIConnectionSet::ShPtr connectionSet() {return itsConnectionSet;};

};

// bool getSubImage(std::string name, SubImage<Float> &subimage, MyAskapParallel &parl)
bool getSubImage(std::string name, SubImage<Float> &subimage, AskapParallel &parl)
{
    /// Trying ways of accessing an image in a way that allows
    /// simultaneous access from different workers.
    ASKAPLOG_INFO_STR(logger, "Worker #" << parl.rank() << ": About to open image " << name);
    LatticeBase* lattPtr = ImageOpener::openImage(name);
    ASKAPLOG_INFO_STR(logger, "Worker #" << parl.rank() << ": Done!");
//    LatticeLocker *lock1 = new LatticeLocker (*lattPtr, FileLocker::Write);
//   LatticeLocker lock1 (*lattPtr, FileLocker::Write);
//   lattPtr->unlock();
    ASKAPASSERT(lattPtr);       // to be sure the image file could be opened
    bool OK = (lattPtr != 0);
    ImageInterface<Float>* imagePtr = dynamic_cast<ImageInterface<Float>*>(lattPtr);
    ASKAPASSERT(imagePtr);
//    lattPtr->unlock();
//   delete lock1;
    IPosition shape = imagePtr->shape();
    ASKAPLOG_DEBUG_STR(logger, "Worker #" << parl.rank() << ": Shape of original image = " << shape);
    IPosition newLength = shape;
//    newLength(0) = newLength(0) / (parl.nnode() - 1);
    newLength(0) = newLength(0) / (parl.nNodes() - 1);
    ASKAPLOG_DEBUG_STR(logger, "Worker #" << parl.rank() << ": New shape = " << newLength);
    int startpos = (parl.rank() - 1) * newLength(0);
    IPosition start(shape.size(), 0);
    start(0) = startpos;
    ASKAPLOG_DEBUG_STR(logger, "Worker #" << parl.rank() << ": Start position = " << start);
    Slicer slice(start, newLength);
    SubImage<Float> sub(*imagePtr, slice, True);
    subimage = sub;
    delete imagePtr;
    return OK;
}


Float subimageMean(const Lattice<Float>& lat)
{
    /// Get the mean pixel value from the subimage.
    const uInt cursorSize = lat.advisedMaxPixels();
    const IPosition cursorShape = lat.niceCursorShape(cursorSize);
    const IPosition latticeShape = lat.shape();
    Float currentSum = 0.0f;
    uInt nPixels = 0u;
    RO_LatticeIterator<Float> iter(lat, LatticeStepper(latticeShape, cursorShape));

    for (iter.reset(); !iter.atEnd(); iter++) {
        currentSum += sum(iter.cursor());
        nPixels += iter.cursor().nelements();
    }

    return currentSum / nPixels;
}


int main(int argc, const char *argv[])
{
    try {
        std::string imageName = std::string(getenv("ASKAP_ROOT"));

        if (argc == 1) imageName += "/Code/Components/Synthesis/testdata/current/simulation/stdtest/image.i.10uJy_clean_stdtest";
        else imageName = argv[1];

        ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
//         MyAskapParallel parl(argc, argv);
        AskapParallel parl(argc, argv);

        if (!parl.isParallel()) {
            ASKAPLOG_ERROR_STR(logger, "This needs to be run in parallel!");
            exit(1);
        }

        if (parl.isMaster()) {
//             ASKAPLOG_INFO_STR(logger, "In Master (#" << parl.rank() << " / " << parl.nnode() << ")");
            ASKAPLOG_INFO_STR(logger, "In Master (#" << parl.rank() << " / " << parl.nNodes() << ")");
            ASKAPLOG_INFO_STR(logger, "Master done!");
        } else if (parl.isWorker()) {
            ASKAPLOG_INFO_STR(logger, "In Worker #" << parl.rank());
            SubImage<Float> subimage;
            bool OK = getSubImage(imageName, subimage, parl);

            if (!OK) ASKAPLOG_ERROR_STR(logger, "Worker #" << parl.rank() << ": ERROR with getting subimage!");

//       ASKAPASSERT(&subimage);
            ASKAPLOG_INFO_STR(logger, "Worker #" << parl.rank() << ": Made a subimage with shape " << subimage.shape());
            ASKAPLOG_DEBUG_STR(logger, "Worker #" << parl.rank() << ": sizeof(subimage) = " << sizeof(subimage));
            ASKAPLOG_INFO_STR(logger, "Worker #" << parl.rank() << ": subimage mean = " << subimageMean(subimage));
            ASKAPLOG_INFO_STR(logger, "Success for Worker #" << parl.rank());
        }
    } catch (askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    } catch (std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    }

    exit(0);
}
