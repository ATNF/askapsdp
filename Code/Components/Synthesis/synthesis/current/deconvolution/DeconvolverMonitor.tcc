/// @file
/// @brief Base class for monitor of Deconvolver
/// @details All the monitoring is delegated to this class so that
/// more flexibility is possible.
/// @ingroup Deconvolver
///  
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
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>

#include <casa/aips.h>
#include <deconvolution/DeconvolverMonitor.h>

namespace askap {

namespace synthesis {

    /// Monitor the current state
    template<class T>
    void DeconvolverMonitor<T>::monitor(const DeconvolverState<T>& ds) {
      ASKAPLOG_INFO_STR(logger, "Iteration " << ds.currentIter()
                        << ", Objective function " << ds.objectiveFunction()
                        << ", Total flux " << ds.totalFlux());
    }

} // namespace synthesis

} // namespace askap


