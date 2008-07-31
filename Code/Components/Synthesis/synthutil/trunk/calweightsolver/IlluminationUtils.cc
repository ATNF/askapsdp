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
#include <coordinates/Coordinates/Projection.h>
#include <measures/Measures/MDirection.h>
#include <lattices/Lattices/ArrayLattice.h>

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
/// @details name file name
void IlluminationUtils::save(const std::string &name)
{
   ASKAPDEBUGASSERT(itsIllumination);
   const double freq=1.4e9;
   UVPattern pattern(itsSize, itsSize, itsCellSize, itsCellSize, itsOverSample);
   itsIllumination->getPattern(freq, pattern);
   
   casa::Matrix<double> xform(2,2);
   xform = 0.; xform.diagonal() = 1.;
   casa::DirectionCoordinate azel(casa::MDirection::AZEL, casa::Projection::SIN, 0.,0.,
                 -itsCellSize, itsCellSize, xform, itsSize/2, itsSize/2);
   casa::CoordinateSystem coords;
   coords.addCoordinate(azel);    
   casa::PagedImage<casa::Complex> result(casa::TiledShape(casa::IPosition(itsSize,
             itsSize)), coords, name);
   casa::ArrayLattice<casa::Complex> patternLattice(pattern.pattern());                
   result.copyData(patternLattice);
   result.setUnits("Jy/pixel");             
}
