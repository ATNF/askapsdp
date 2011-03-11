/// @file DirectionConverter.h
/// @brief A class for direction conversion
/// @details
/// DirectionConverter is a class for direction conversion. This is an
/// implementation of the low-level interface, which is used within the
/// implementation of the data accessor. The end user interacts with the
/// IDataConverter class. 
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

#ifndef DIRECTION_CONVERTER_H
#define DIRECTION_CONVERTER_H

// CASA includes
#include <measures/Measures/MDirection.h>

// own includes
#include <dataaccess/IDirectionConverter.h>

namespace askap {

namespace accessors {

/// @brief An implementation of the direction converter.
/// @details This class just call the appropriate functionality
/// of the direction measures.
/// @todo we probably need a class where default input frame can be
/// specified at construction (e.g. operator() can receive MVDirection or
/// even Doubles). Such class can be derived from this one
/// @ingroup dataaccess_conv
struct DirectionConverter : public IDirectionConverter {
    /// create a converter to the target frame
    /// @param targetFrame a desired reference frame
    ///        Class defaults to J2000
    DirectionConverter(const casa::MDirection::Ref &targetFrame =
                             casa::MDirection::Ref(casa::MDirection::J2000));

    /// convert specified MEpoch to the target units/frame
    /// @param in an epoch to convert. 
    virtual casa::MVDirection operator()(const casa::MDirection &in) const;

    /// set a frame (i.e. time and/or position), where the
    /// conversion is performed
    /// @param frame  MeasFrame object (can be constructed from
    ///               MPosition or MEpoch on-the-fly)
    virtual void setMeasFrame(const casa::MeasFrame &frame);

private:
    casa::MDirection::Ref itsTargetFrame;    
};

} // namespace accessors

} // namespace askap

#endif // EPOCH_CONVERTER_H
