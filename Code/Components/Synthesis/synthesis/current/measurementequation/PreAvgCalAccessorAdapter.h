/// @file
/// 
/// @brief This class is used inside the measurement equation object implementing
/// pre-averaging (or pre-summing to be exact) algorithm for calibration. Strictly
/// speaking it is not an adapter and it doesn't behave as an accessor. However, it
/// mimics the accessor interface, so we can reuse the existing code to a greater
/// extent. In addition, we can extend the code to a more complicated types of calibration 
/// later (i.e. with equations using more metadata). Current implementation is derived 
/// from DataAccessorAdapter just to speed up the development. None of the functionality
/// of this base class is used (except throwing exceptions if methods which are not
/// intended to be used are called). The plan is to always keep the DataAccessorAdapter
/// in the detached state.
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
