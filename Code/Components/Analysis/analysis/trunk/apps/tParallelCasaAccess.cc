/// @file : testing ways to access Measurement Sets and related information
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

#include <analysisutilities/CasaImageUtil.h>
#include <parallelanalysis/DuchampParallel.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <askapparallel/AskapParallel.h>

#include <APS/ParameterSet.h>
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
using namespace askap::analysis;
using namespace askap::cp;

ASKAP_LOGGER(logger, "tCasaImageAccess.log");

class MyAskapParallel: public askap::cp::AskapParallel
{
public:
  MyAskapParallel(int argc, const char **argv):AskapParallel(argc,argv){};
  int nnode(){return itsNNode;};
  int rank(){return itsRank;};
  askap::cp::MPIConnectionSet::ShPtr connectionSet(){return itsConnectionSet;};

};

bool getSubImage(std::string name, SubImage<Float> &subimage, MyAskapParallel &parl)
{
      LatticeBase* lattPtr = ImageOpener::openImage (name);
      LatticeLocker *lock1 = new LatticeLocker (*lattPtr, FileLocker::Read); 
      lattPtr->unlock();
      ASKAPASSERT (lattPtr);      // to be sure the image file could be opened
      bool OK = (lattPtr != 0);
      ImageInterface<Float>* imagePtr = dynamic_cast<ImageInterface<Float>*>(lattPtr);
      IPosition shape = imagePtr->shape();
      std::cerr << shape << "\n";
      IPosition newLength = shape;
      newLength(0) = newLength(0) / (parl.nnode()-1);
      std::cerr << newLength << "\n";
      int startpos = (parl.rank()-1)*newLength(0);
      IPosition start(shape.size(),0);
      start(0) = startpos;
      std::cerr << start << " " << newLength << "\n";
      Slicer slice(start, newLength);
      SubImage<Float> sub(*imagePtr, slice, True);

      subimage = sub;

      return OK;
}

Float subimageMean(const Lattice<Float>& lat) {
  const uInt cursorSize = lat.advisedMaxPixels();
  const IPosition cursorShape = lat.niceCursorShape(cursorSize);
  const IPosition latticeShape = lat.shape();
  Float currentSum = 0.0f;
  uInt nPixels = 0u;
  RO_LatticeIterator<Float> iter(lat, 
                                   LatticeStepper(latticeShape, cursorShape));
  for (iter.reset(); !iter.atEnd(); iter++){
    currentSum += sum(iter.cursor());
    nPixels += iter.cursor().nelements();
  }
  return currentSum/nPixels;
}


int main(int argc, const char *argv[])
{

  try {
    std::string imageName;
    if(argc==1) imageName = "$ASKAP_ROOT/Code/Components/Synthesis/testdata/trunk/simulation/stdtest/image.i.10uJy_clean_stdtest";
    else imageName = argv[1];

    ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
   MyAskapParallel parl(argc,argv);

    if(!parl.isParallel()){
      ASKAPLOG_ERROR_STR(logger, "This needs to be run in parallel!");
      exit(1);
    }

    if(parl.isMaster()){

      ASKAPLOG_INFO_STR(logger, "In Master (#" << parl.rank() << " / " << parl.nnode() << ")");
//       std::stringstream ss;
//       bool OK;
//       for(int i=1;i<parl.nnode();i++){
// 	LOFAR::BlobString bs1;
// 	bs1.resize(0);
// 	LOFAR::BlobOBufString bob(bs1);
// 	LOFAR::BlobOStream out(bob);
// 	out.putStart("mw",1);
// 	out << i ;
// 	out.putEnd();
// 	parl.connectionSet()->writeAll(bs1);
// 	ASKAPLOG_INFO_STR(logger,"Master: Sent to worker #"<<i);
	
//        	LOFAR::BlobString bs2;
// 	parl.connectionSet()->read(i-1, bs2);
// 	LOFAR::BlobIBufString bib(bs2);
// 	LOFAR::BlobIStream in(bib);
// 	int version=in.getStart("wm");
// 	ASKAPASSERT(version==1);
// 	in >> OK;
// 	in.getEnd();
// 	ASKAPLOG_INFO_STR(logger,"Master: Read from worker #"<<i<<": OK="<<OK);
//       }

//       LOFAR::BlobString bs3;
//       bs3.resize(0);
//       LOFAR::BlobOBufString bob3(bs3);
//       LOFAR::BlobOStream out3(bob3);
//       out3.putStart("mw",1);
//       out3 << parl.nnode();
//       out3.putEnd();
//       parl.connectionSet()->writeAll(bs3);

//       float mean = 77.;
//       LOFAR::BlobString bs4;
//       bs4.resize(0);
//       LOFAR::BlobOBufString bob4(bs4);
//       LOFAR::BlobOStream out4(bob4);
//       out4.putStart("mean",1);
//       out4 << mean ;
//       out4.putEnd();
//       parl.connectionSet()->writeAll(bs4);

      std::cout << "Master done!\n";

    }
    else if(parl.isWorker()){

      ASKAPLOG_INFO_STR(logger, "In Worker #" << parl.rank());

      int rank;
      bool OK;

//       do{
// 	LOFAR::BlobString bs1;
// 	bs1.resize(0);
// 	parl.connectionSet()->read(0, bs1);
// 	LOFAR::BlobIBufString bib(bs1);
// 	LOFAR::BlobIStream in(bib);
// 	std::stringstream ss;
// 	int version=in.getStart("mw");
// 	ASKAPASSERT(version==1);
// 	in >> rank;
// 	in.getEnd();
//       }	while(rank != parl.rank());
      
      ASKAPLOG_INFO_STR(logger, "Worker #"<<parl.rank()<<" has the OK");
      
      SubImage<Float> subimage;
      OK = getSubImage(imageName,subimage,parl);
//       ASKAPASSERT(&subimage);
      ASKAPLOG_INFO_STR(logger,"Worker #"<<parl.rank()<<": Made a subimage with shape " << subimage.shape());
      ASKAPLOG_DEBUG_STR(logger,"Worker #"<<parl.rank()<<": sizeof(subimage) = " << sizeof(subimage));
      ASKAPLOG_INFO_STR(logger,"Worker #"<<parl.rank()<<": subimage mean = " << subimageMean(subimage));
      
//       LOFAR::BlobString bs2;
//       bs2.resize(0);
//       LOFAR::BlobOBufString bob(bs2);
//       LOFAR::BlobOStream out(bob);
//       out.putStart("wm",1);
//       out << OK;
//       out.putEnd();
//       parl.connectionSet()->write(0,bs2);

//       do{
// 	LOFAR::BlobString bs3;
// 	bs3.resize(0);
// 	parl.connectionSet()->read(0, bs3);
// 	LOFAR::BlobIBufString bib3(bs3);
// 	LOFAR::BlobIStream in3(bib3);
// 	std::stringstream ss;
// 	int version=in3.getStart("mw");
// 	ASKAPASSERT(version==1);
// 	in3 >> rank;
// 	in3.getEnd();
//       }	while(rank != parl.nnode());


      float mean;
// 	LOFAR::BlobString bs4;
// 	bs4.resize(0);
// 	parl.connectionSet()->read(0, bs4);
// 	LOFAR::BlobIBufString bib4(bs4);
// 	LOFAR::BlobIStream in4(bib4);
// 	std::stringstream ss;
// 	int version=in4.getStart("mean");
// 	while(version!=1) {}
// 	//	ASKAPASSERT(version==1);
// 	in4 >> mean;
// 	in4.getEnd();
// 	ASKAPLOG_INFO_STR(logger, "Worker #"<<parl.rank()<<" received mean of " << mean << " from Master.");
      
      std::cout << "Success! (" << parl.rank() <<")\n";

    }
  }
  catch (askap::AskapError& x)
    {
      ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
      std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
      exit(1);
    }
  catch (std::exception& x)
    {
      ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
      std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
      exit(1);
    }
  exit(0);
}
