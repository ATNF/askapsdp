#include <casa/Arrays/IPosition.h>
#include <CommandLineParser.h>
#include <askap/AskapError.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/Coordinate.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <casa/Quanta/MVDirection.h>
#include <imageaccess/CasaImageAccess.h>


#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace askap;
using namespace std;

// Main function
int main(int argc, const char** argv) { 
  try {
     if (argc < 3) {
         throw cmdlineparser::XParser();
     }
     cmdlineparser::Parser parser; // a command line parser
     // command line parameter
     std::vector<cmdlineparser::GenericParameter<std::string> > inputParameters(argc-2);     
     for (std::vector<cmdlineparser::GenericParameter<std::string> >::iterator it = inputParameters.begin();
          it!= inputParameters.end(); ++it) {
          parser.add(*it);
     }
     cmdlineparser::GenericParameter<std::string> outfile;
     parser.add(outfile);
          
     parser.process(argc, argv);
     
     std::vector<std::string> inputFiles(inputParameters.size());
     for (size_t i=0;i<inputFiles.size();++i) {
          inputFiles[i] = inputParameters[i].getValue();
          std::cout<<"Input image "<<i<<" is "<<inputFiles[i]<<std::endl;
     }
     std::cout<<"Output will be stored to "<<outfile.getValue()<<std::endl;
     ASKAPCHECK(inputFiles.size()>0, "At least one input image should be defined");

     synthesis::CasaImageAccess ia;
     const casa::IPosition shape = ia.shape(inputFiles[0]);
     ASKAPCHECK(shape.nelements()>=2,"Work with at least 2D images!");
          
     casa::IPosition newShape(shape.nelements()+1);
     for (int i = 0; i<int(shape.nelements()); ++i) {
          newShape[i] = shape[i];
     }
     newShape[shape.nelements()] = int(inputFiles.size());
     casa::IPosition where(newShape.nelements(),0);
     
     casa::CoordinateSystem csys = ia.coordSys(inputFiles[0]);     
     csys.addCoordinate(casa::LinearCoordinate(1));
     
     ia.create(outfile.getValue(), newShape, csys);
     for (size_t i=0; i<inputFiles.size(); ++i) {
          casa::Array<float> buf = ia.read(inputFiles[i]);
          where[shape.nelements()] = int(i);
          ia.write(outfile.getValue(),buf,where);
     }         
  }
  ///==============================================================================
  catch (const cmdlineparser::XParser &ex) {
	 std::cerr << "Usage: " << argv[0] << " input_cube1 [input_cube2 ... input_cubeLast] output_image"
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
