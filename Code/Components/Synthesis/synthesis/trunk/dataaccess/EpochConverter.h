/// @file EpochConverter.h
/// @brief A class for epoch conversion
/// @details This is an implementation
/// of the low-level interface, which is used within the implementation of
/// the data accessor. The end user interacts with the IDataConverter
/// class. 
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

#ifndef EPOCH_CONVERTER_H
#define EPOCH_CONVERTER_H

// CASA includes
#include <measures/Measures/MEpoch.h>

// own includes
#include <dataaccess/IEpochConverter.h>

namespace askap {

namespace synthesis {

/// @brief An implementation of the epoch converter.
/// @details This class just call the appropriate functionality of
/// the (epoch) measures.
/// @todo we probably need a class where default input frame can be
/// specified at construction (e.g. operator() can receive MVEpoch or
/// even a Double). Such class can be derived from this one
/// @ingroup dataaccess_conv
struct EpochConverter : public IEpochConverter {
    /// create a converter to the target frame/unit
    /// @param targetOrigin a measure describing target reference frame
    ///        and origin (e.g. w.r.t. midnight 30/05/2007 UTC)
    ///        Class defaults to MJD 0 UTC
    /// @param targetUnit desired units in the output. Class defaults
    ///        to seconds
    EpochConverter(const casa::MEpoch &targetOrigin = casa::MEpoch(),
                   const casa::Unit &targetUnit = "s");

    /// convert specified MEpoch to the target units/frame
    /// @param in an epoch to convert. 
    casa::Double operator()(const casa::MEpoch &in) const;

    /// Reverse conversion (casa::Double to full measure)
    /// @param in an epoch given as Double in the target units/frame
    /// @return the same epoch as a fully qualified measure
    casa::MEpoch toMeasure(casa::Double in) const;

    /// Reverse conversion (casa::MVEpoch to full measure)
    /// @param in an epoch given as MVEpoch in the target frame
    /// @return the same epoch as a fully qualified measure
    casa::MEpoch toMeasure(const casa::MVEpoch &in) const throw();


    /// set a frame (for epochs it is just a position), where the
    /// conversion is performed
    virtual void setMeasFrame(const casa::MeasFrame &frame);

private:
    casa::MVEpoch itsTargetOrigin;
    casa::MEpoch::Ref itsTargetRef;
    casa::Unit  itsTargetUnit;
};

} // namespace synthesis

} // namespace askap

#endif // EPOCH_CONVERTER_H
