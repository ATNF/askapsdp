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
      std::stringstream ss;
      int rank;
      bool OK;
      for(int i=1;i<parl.nnode();i++){
	LOFAR::BlobString bs1;
	bs1.resize(0);
	LOFAR::BlobOBufString bob(bs1);
	LOFAR::BlobOStream out(bob);
	out.putStart("mw",1);
	out << i ;
	out.putEnd();
	parl.connectionSet()->writeAll(bs1);
	ASKAPLOG_INFO_STR(logger,"Sent to worker #"<<i);
	
	//	while(parl.connectionSet()->size()<2*(i-1)){}
       	LOFAR::BlobString bs2;
	parl.connectionSet()->read(i-1, bs2);
	LOFAR::BlobIBufString bib(bs2);
	LOFAR::BlobIStream in(bib);
	int version=in.getStart("wm");
	ASKAPASSERT(version==1);
	in >> OK;
	in.getEnd();
	ASKAPLOG_INFO_STR(logger,"Read from worker #"<<i<<": OK="<<OK);
      }

    }
    else if(parl.isWorker()){

      ASKAPLOG_INFO_STR(logger, "In Worker #" << parl.rank());

      int rank;
      bool OK;

      do{
	LOFAR::BlobString bs1;
	bs1.resize(0);
	std::cerr << parl.connectionSet()->size() << " " << parl.rank() << "\n";
	//      while(parl.connectionSet()->size()<2*(parl.rank()-1)){}
	parl.connectionSet()->read(0, bs1);
	LOFAR::BlobIBufString bib(bs1);
	LOFAR::BlobIStream in(bib);
	std::stringstream ss;
	int version=in.getStart("mw");
	ASKAPASSERT(version==1);
	in >> rank;
	in.getEnd();
      }	while(rank != parl.rank());
      
      ASKAPLOG_INFO_STR(logger, "Worker #"<<parl.rank()<<" has the OK");

      LatticeBase* lattPtr = ImageOpener::openImage (imageName);
      ASKAPASSERT (lattPtr);      // to be sure the image file could be opened
      OK = (lattPtr != 0);
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
      SubImage<Float> subimage(*imagePtr, slice, True);
      ASKAPLOG_INFO_STR(logger,"Made a subimage with shape " << subimage.shape());

      LOFAR::BlobString bs2;
      bs2.resize(0);
      LOFAR::BlobOBufString bob(bs2);
      LOFAR::BlobOStream out(bob);
      out.putStart("wm",1);
      out << OK;
      out.putEnd();
      parl.connectionSet()->write(0,bs2);



      
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
