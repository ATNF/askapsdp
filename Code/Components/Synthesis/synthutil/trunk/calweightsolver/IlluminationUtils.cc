/// @file
/// 
/// @brief utilities related to illumination pattern
/// @details This class is written for experiments with eigenbeams and synthetic beams.
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

#include <calweightsolver/IlluminationUtils.h>

#include <images/Images/PagedImage.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
//#include <coordinates/Coordinates/StokesCoordinate.h>
//#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <coordinates/Coordinates/Projection.h>
#include <measures/Measures/MDirection.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <casa/Arrays/ArrayMath.h>


#include <APS/ParameterSet.h>


#include <gridding/UVPattern.h>
#include <gridding/VisGridderFactory.h>

#include <askap/AskapError.h>

using namespace askap;
using namespace askap::synthutils;
using namespace askap::synthesis;


/// @brief constructor
/// @details 
/// @param[in] illum illumination pattern to work with 
/// @param[in] size desired image size
/// @param[in] cellsize uv-cell size 
/// @param[in] oversample oversampling factor (default 1) 
IlluminationUtils::IlluminationUtils(const boost::shared_ptr<IBasicIllumination> &illum,
        size_t size, double cellsize, size_t oversample) :
        itsIllumination(illum), itsSize(size), itsCellSize(cellsize), itsOverSample(oversample)
{}

/// @brief constructor from a parset file 
/// @details
/// This version extracts all required parameters from the supplied parset file 
/// using the same factory, which provides illumination patterns for gridders.
/// @param[in] parset parset file name 
IlluminationUtils::IlluminationUtils(const std::string &parset)
{
  LOFAR::ACC::APS::ParameterSet params(parset);
  itsIllumination = VisGridderFactory::makeIllumination(params);
  itsCellSize = params.getDouble("cellsize");


  const int size = params.getInt32("size"); 
  ASKAPCHECK(size>0,"Size is supposed to be positive, you have "<<size);
  itsSize = size_t(size);
  
  const int oversample = params.getInt32("oversample");
  ASKAPCHECK(oversample>0,"Oversample is supposed to be positive, you have "<<oversample);
  itsOverSample = size_t(oversample);  
}
   
/// @brief save the pattern into an image
/// @details 
/// @param[in] name file name
/// @param[in] what type of the image requested, e.g. amplitude (default),
/// real, imag, phase, complex. Minimum match applies.
void IlluminationUtils::save(const std::string &name, const std::string &what)
{
   ASKAPDEBUGASSERT(itsIllumination);
   const double freq=1.4e9;
   UVPattern pattern(itsSize, itsSize, itsCellSize, itsCellSize, itsOverSample);
   itsIllumination->getPattern(freq, pattern);
   
   casa::Matrix<double> xform(2,2);
   xform = 0.; xform.diagonal() = 1.;
  // casa::DirectionCoordinate azel(casa::MDirection::AZEL, casa::Projection::SIN, 0.,0.,
  //               -itsCellSize, itsCellSize, xform, itsSize/2, itsSize/2);
   casa::Vector<casa::String> names(2);
   names[0]="U"; names[1]="V";
   
   casa::Vector<double> increment(2);
   increment[0]=-itsCellSize/double(itsOverSample);
   increment[1]=itsCellSize/double(itsOverSample);
   
   casa::LinearCoordinate linear(names, casa::Vector<casa::String>(2,"lambda"),
          casa::Vector<double>(2,0.), increment, xform, 
          casa::Vector<double>(2,double(itsSize)/2));
   
   casa::CoordinateSystem coords;
   coords.addCoordinate(linear);    
   //coords.addCoordinate(azel);    
   if (what.find("complex") == 0) {
       casa::PagedImage<casa::Complex> result(casa::TiledShape(casa::IPosition(2,itsSize,
               itsSize)), coords, name);
       casa::ArrayLattice<casa::Complex> patternLattice(pattern.pattern());                
       result.setUnits("Jy/pixel");             
   } else {
       casa::PagedImage<casa::Float> result(casa::TiledShape(casa::IPosition(2,itsSize,
               itsSize)), coords, name);
       casa::Array<casa::Float> workArray;
       if (what.find("amp")==0) {
           workArray = casa::amplitude(pattern.pattern());
       } else if (what.find("real") == 0) {
           workArray = casa::real(pattern.pattern());
       } else if (what.find("imag") == 0) {
           workArray = casa::imag(pattern.pattern());
       } else if (what.find("phase") == 0) {
           workArray = casa::phase(pattern.pattern());
       } else {
          ASKAPTHROW(AskapError, "Unknown type of image requested from IlluminationUtils::save ("<<
                     what<<")");
       }
       casa::ArrayLattice<casa::Float> patternLattice(workArray);
       result.copyData(patternLattice);
       result.setUnits("Jy/pixel");             
   }
}
