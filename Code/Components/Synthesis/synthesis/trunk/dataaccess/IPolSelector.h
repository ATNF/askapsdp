/// @file IPolSelector.h
/// @brief Interface to select a number of polarization products
/// @details Examples of the polarization selection are, e.g. circulars
/// with cross products, full stokes, stokes I only, etc.
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

#ifndef I_POL_SELECTOR_H
#define I_POL_SELECTOR_H

namespace askap {

namespace synthesis {

/// @brief Interface to select a number of polarization products
/// @details Examples of the polarization selection are, e.g. circulars
/// with cross products, full stokes, stokes I only, etc.
/// @ingroup dataaccess_i
class IPolSelector {
public:
    /// non-explicit constructor to allow conversion from the strings like
    /// "XX,YY,XY,YX"
    /// @param[in] stokes requested polarizations, i.e. a string like
    /// "XX,YY,XY,YX"
    IPolSelector(const std::string &stokes);
    
    /// an empty virtual destructor to make the compiler happy
    virtual ~IPolSelector();

    /// @return a number of polarization products selected
    size_t nPol() const throw();

    /// @return true if the selection is a subset of circular polarization
    ///              products
    bool isCircular() const throw();

    /// @return true if the selection is a subset of linear polarization
    ///              products
    bool isLinear() const throw();

    /// @return true if the selection is a subset of stokes parameters
    bool isStokes() const throw();
};
} // namespace synthesis

} // namespace askap

#endif // #ifndef I_POL_SELECTOR_H
