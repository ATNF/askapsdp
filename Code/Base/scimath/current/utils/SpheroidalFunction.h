/// @file 
/// @brief Calculation of speroidal function 
/// @details Speroidal function is used for gridding to achieve aliasing rejection.
///
/// @copyright (c) 2008 CSIRO
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

#ifndef SPEROIDAL_FUNCTION_H
#define SPEROIDAL_FUNCTION_H

/// @brief Calculation of speroidal function 
/// @details Speroidal function is used for gridding to achieve aliasing rejection.
struct SpheroidalFunction {
   /// @brief value of the function for argument nu
   /// @details
   /// @param[in] nu argument of the function
   /// @return value of the function 
   double operator()(double nu) const;
};

#endif // #ifndef SPEROIDAL_FUNCTION_H


