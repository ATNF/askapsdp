#include <casa/Arrays/IPosition.h>
#include <CommandLineParser.h>
#include <askap/AskapError.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/Coordinate.h>
#include <casa/Quanta/MVDirection.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/SubImage.h>



#include <stdexcept>
#include <iostream>

using namespace askap;
using namespace std;

// Main function
int main(int argc, const char** argv) { 
  try {
     cmdlineparser::Parser parser; // a command line parser
     // command line parameter
     cmdlineparser::FlaggedParameter<int> nchan("-n",1);
     cmdlineparser::GenericParameter<std::string> imgfile;
     cmdlineparser::GenericParameter<std::string> outfile;
	 cmdlineparser::GenericParameter<int> startchan;
     
     parser.add(nchan,cmdlineparser::Parser::return_default); // optional
     parser.add(startchan);
	 parser.add(imgfile);
	 parser.add(outfile);
	
	 parser.process(argc, argv);
     casa::PagedImage<casa::Float> img(imgfile.getValue());
     ASKAPCHECK(img.ok(),"Error loading "<<imgfile.getValue());
     ASKAPCHECK(img.shape().nelements()>=3,"Work with at least 3D cubes!");
     
     const casa::IPosition shape = img.shape();
     
     casa::IPosition blc(shape.nelements(),0);
     blc[3] = startchan;
     casa::IPosition trc(shape);
     trc[3] = nchan;
     ASKAPCHECK(blc[3]>=0 && blc[3]<shape[3], "Start channel is outside the number of channels or negative, shape: "<<shape);
     ASKAPCHECK(trc[3]+blc[3]<=shape[3], "Subcube extends beyond the original cube, shape:"<<shape);
     
     casa::Slicer slc(blc,trc,casa::IPosition(shape.nelements(),1));
     
     casa::SubImage<casa::Float> si = casa::SubImage<casa::Float>(img,slc,casa::AxesSpecifier(casa::False));
     casa::PagedImage<casa::Float> res(si.shape(),si.coordinates(),std::string(outfile.getValue()));
     res.put(si.get());
  }
  ///==============================================================================
  catch (const cmdlineparser::XParser &ex) {
	 std::cerr << "Usage: " << argv[0] << " [-n number_of_chan] start_chan input_cube output_image"
			<< std::endl;
  }

  catch (const askap::AskapError& x) {
     std::cerr << "Askap error in " << argv[0] << ": " << x.what()
        << std::endl;
     exit(1);
  } 
  catch (const std::exception& x) {
	 std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what()
			<< std::endl;
	 exit(1);
  }
  exit(0);  
}
