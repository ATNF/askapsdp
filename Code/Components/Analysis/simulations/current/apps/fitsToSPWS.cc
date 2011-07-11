///
/// @file : Create an SPWS file appropriate for an existing FITS file.
///
/// Control parameters are passed in from a LOFAR ParameterSet file.
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
#include <askap_simulations.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <string>
#include <iomanip>

#include <casa/aipstype.h>
#include <images/Images/FITSImage.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/SubImage.h>
#include <lattices/Lattices/LatticeLocker.h>
#include <coordinates/Coordinates/Coordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MFrequency.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Containers/RecordInterface.h>
#include <casa/Containers/RecordField.h>
#include <casa/Containers/RecordFieldId.h>
#include <casa/Quanta.h>
#include <casa/Quanta/Unit.h>

using namespace askap;
using namespace casa;

ASKAP_LOGGER(logger, "fitsToSPWS.log");

void usage() {
  std::cout << "fitsToSPWS [options]\n"
	    << "Write out the channel information for an image in a form suitable for ASKAP spws input\n"
	    << "Options:\n"
	    << "     -i: FITS image. NO DEFAULT!\n"
	    << "     -n: Base name for spws entries. Default is taken from fits filename (without .fits if present)\n"
	    << "     -b: Spectral binning (number of channels to combine per entry) [default=1]\n"
	    << "     -p: Polarisation info: either number of polarisations or specific polarisation string\n"
	    << "         [default is 2 pol, \"XX YY\"]\n"
	    << "     -u: Spectral units [default=MHz]\n"
	    << "     -P: Precision for frequency & increment values [default=3]\n"
	    << "     -g: Group size [default=0=no groups]\n";
}

std::string baseify(std::string name)
{
  std::string input = name;
  if(input.substr(input.size()-5,5)==".fits") return input.substr(0,input.size()-5);
  else return input;
}

int main(int argc, char *argv[])
{
  try {

    std::string image="",basename="",pol="XX YY";
    casa::Unit units="MHz";
    int binning = 1,ch,prec=3,group=0;
    while ((ch = getopt(argc, argv, "i:n:b:p:u:P:g:h")) != -1) {
      switch (ch) {
      case 'i':
	image = std::string(optarg);
	break;
      case 'n':
	basename = std::string(optarg);
	break;
      case 'b':
	binning = atoi(optarg);
	break;
      case 'p':
	pol = std::string(optarg);
	break;
      case 'u':
	units = casa::String(optarg);
	break;
      case 'P':
	prec = atoi(optarg);
	break;
      case 'g':
	group = atoi(optarg);
	break;
      case 'h':
      default:
	usage();
	exit(0);
      }
    }
    ASKAPCHECK(image!="","Need to supply a FITS image via the -i option.");

    if(basename=="") basename = baseify(image);

    ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
    const LatticeBase* lattPtr = ImageOpener::openImage(image);
    if (lattPtr == 0)
      ASKAPTHROW(AskapError, "Requested image \"" << image << "\" does not exist or could not be opened.");
    const ImageInterface<Float>* imagePtr = dynamic_cast<const ImageInterface<Float>*>(lattPtr);
    Int index=imagePtr->coordinates().findCoordinate(casa::Coordinate::SPECTRAL);
    Int axis=imagePtr->coordinates().worldAxes(index)[0];
    
    IPosition shape = imagePtr->shape().nonDegenerate();
    SpectralCoordinate specCoo = imagePtr->coordinates().spectralCoordinate(index);

    casa::Vector<casa::Double> inc = specCoo.increment();
    MFrequency increment(Quantity(inc[0],specCoo.worldAxisUnits()[0]));

    std::cout << "spws.names = [";
    int count=0;
    for (int z=0;z < shape(axis); z++) {
      if(z%binning==0){
	std::stringstream groupname;
	if(group>1) groupname << (z/binning)/group << "_" << (z/binning)%group;
	else groupname << z/binning;
	std::cout << basename << groupname.str();
	if(++count < (shape(axis)/binning)) std::cout << ",";
      }
    }
    std::cout << "]\n\n";

    for (int z=0;z < shape(axis); z++) {
      if(z%binning==0){
	MFrequency freq;
	specCoo.toWorld(freq,double(z));
	std::stringstream groupname;
	if(group>1) groupname << (z/binning)/group << "_" << (z/binning)%group;
	else groupname << z/binning;
	std::cout.setf(std::ios::fixed);
	std::cout << "spws."<<basename<<groupname.str() <<"   = ["
		  << binning << ", "
		  << std::setprecision(prec) << freq.get(units) <<", "
		  << std::setprecision(prec) << increment.get(units)<< ", "
		  << "\""<<pol << "\"]\n";
      }
    }

  } catch (const askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    } catch (const std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    }

    return 0;
}
