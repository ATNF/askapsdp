///
/// @file : statistics of the image
///
/// This program is intended to be used in scripts (to extract statistics from an image)
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#include <casa/Arrays/IPosition.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageStatistics.h>
#include <CommandLineParser.h>
#include <askap/AskapError.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/Coordinate.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>
#include <casa/Quanta/MVDirection.h>


#include <stdexcept>
#include <iostream>

using namespace askap;

void printDirection(ostream &os,const casa::MDirection &dir)  {
    double lngbuf=dir.getValue().getLong("deg").getValue();
    if (lngbuf<0) lngbuf+=360.;
    os<<(dir.getRefString()!="GALACTIC"?casa::MVAngle::Format(casa::MVAngle::TIME):
          casa::MVAngle::Format(casa::MVAngle::ANGLE))<<casa::MVAngle(casa::Quantity(lngbuf,"deg"))<<" "<<
          casa::MVAngle(dir.getValue().getLat("deg"))<<
          " ("<<dir.getRefString()<<")";
}


// Main function
int main(int argc, const char** argv) { 
  try {
     cmdlineparser::Parser parser; // a command line parser
	 // command line parameter
	 cmdlineparser::GenericParameter<std::string> imgfile;
	 parser.add(imgfile);

	 // I hope const_cast is temporary here
	 parser.process(argc, const_cast<char**> (argv));
     casa::PagedImage<casa::Float> img(imgfile.getValue());
     casa::ImageStatistics<casa::Float> imstat(img, casa::False);
     float tmin,tmax;
     imstat.getFullMinMax(tmin,tmax);
     casa::IPosition minPos,maxPos;
     imstat.getMinMaxPos(minPos,maxPos);
     casa::Int direction_coordinate = img.coordinates().findCoordinate(casa::Coordinate::DIRECTION);
     ASKAPASSERT(direction_coordinate>=0);
     ASKAPASSERT(maxPos.nelements()>=2);
     const casa::DirectionCoordinate &dc = img.coordinates().directionCoordinate(direction_coordinate);
     casa::Vector<casa::Double> pixel(2);
     pixel(0)=casa::Double(maxPos[0]);
     pixel(1)=casa::Double(maxPos[1]);
     casa::MDirection res;
     ASKAPASSERT(dc.toWorld(res,pixel));
     
     // print peak in the image and position of the peak 
     std::cout<<tmax<<" ";
     printDirection(cout,res);
     std::cout<<" # Max RA Dec (Epoch)"<<std::endl;
     
     std::cout<<std::setprecision(15)<<res.getValue().getLong("deg").getValue()<<" "<<
                res.getValue().getLat("deg").getValue()<<" # RA DEC"<<std::endl;
     
     casa::Array<float> statBuf;
     if (imstat.getConvertedStatistic(statBuf,casa::LatticeStatsBase::RMS)) {
         casa::Vector<float> statVec(statBuf.reform(casa::IPosition(1,statBuf.nelements())));
         ASKAPCHECK(statVec.nelements() == 1, "Expect exactly one element in the array returned by getConvertedStatistics; you have: "<<statVec);         
         std::cout<<statVec[0]<<" ";
     }
     if (imstat.getConvertedStatistic(statBuf,casa::LatticeStatsBase::MEDIAN)) {
         casa::Vector<float> statVec(statBuf.reform(casa::IPosition(1,statBuf.nelements())));
         ASKAPCHECK(statVec.nelements() == 1, "Expect exactly one element in the array returned by getConvertedStatistics; you have: "<<statVec);         
         std::cout<<statVec[0]<<" # RMS MEDIAN"<<std::endl;
     }     
  }
  ///==============================================================================
  catch (const cmdlineparser::XParser &ex) {
	 std::cerr << "Usage: " << argv[0] << " imagefile"
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
