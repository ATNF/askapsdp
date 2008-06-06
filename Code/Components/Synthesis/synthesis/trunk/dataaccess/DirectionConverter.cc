/// @file DirectionConverter.cc
/// @brief A class for direction conversion
/// @details This is an implementation of the low-level interface,
/// which is used within the implementation of the data accessor.
/// The end user interacts with the  IDataConverter class. 
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
///

// CASA includes
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Quanta/MVDirection.h>

// own includes
#include <dataaccess/DirectionConverter.h>

using namespace askap;
using namespace askap::synthesis;
using namespace casa;

/// create a converter to the target frame
/// @param targetFrame a desired reference frame
///        Class defaults to J2000
DirectionConverter::DirectionConverter(const casa::MDirection::Ref
                          &targetFrame) : itsTargetFrame(targetFrame) {}


/// convert specified MEpoch to the target units/frame
/// @param in an epoch to convert. 
casa::MVDirection
    DirectionConverter::operator()(const casa::MDirection &in) const
{
    /// this class is supposed to be used in the most general case, hence
    /// we do all conversions. Specializations can be written for the cases
    /// when either frame or unit conversions are not required
    return MDirection::Convert(in.getRef(),
                             itsTargetFrame)(in).getValue();    
}

/// set a frame (i.e. time and/or position), where the
/// conversion is performed
/// @param frame  MeasFrame object (can be constructed from
///               MPosition or MEpoch on-the-fly)
void DirectionConverter::setMeasFrame(const casa::MeasFrame &frame)
{
  itsTargetFrame.set(frame);
}
