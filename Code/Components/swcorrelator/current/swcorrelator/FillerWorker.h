/// @file 
///
/// @brief Writing thread of the MS filler
/// @details This class holds a shared pointer to the main filler and can call
/// its methods to get data and to synchronise.
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

#ifndef ASKAP_SWCORRELATOR_FILLER_WORKER
#define ASKAP_SWCORRELATOR_FILLER_WORKER

#include <swcorrelator/CorrFiller.h>
#include <boost/shared_ptr.hpp>

namespace askap {

namespace swcorrelator {

/// @brief Writing thread of the MS filler
/// @details This class holds a shared pointer to the main filler and can call
/// its methods to get data and to synchronise.
/// @ingroup swcorrelator
struct FillerWorker {

  /// @brief constructor, pass the shared pointer to the filler
  FillerWorker(const boost::shared_ptr<CorrFiller> &filler);

  /// @brief entry point for the parallel thread
  void operator()();
  
private:
  boost::shared_ptr<CorrFiller> itsFiller;     
};

} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_FILLER_WORKER


