/// @file 
///
/// @brief parallel thread which runs the correlator
/// @details This class is analogous to the main method of the stand alone correlator 
/// application. It can run the correlator, get monitoring data and stop when necessary.
/// The primary goal for this interface is to run the software correlator from the epics
/// CA server. We use parallel thread to get the asynchronous behavior. This class represents
/// the child thread and CorrRunner the main thread.
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

#ifndef ASKAP_CORRINTERFACES_CORR_RUNNER_THREAD_H
#define ASKAP_CORRINTERFACES_CORR_RUNNER_THREAD_H

// boost includes
#include <boost/shared_ptr.hpp>

// other 3rd party
#include <Common/ParameterSet.h>

namespace askap {

namespace swcorrelator {

// forward declaration to allow passing a pointer
struct CorrRunner;

/// @brief parallel thread which runs the correlator
/// @details This class is analogous to the main method of the stand alone correlator 
/// application. It can run the correlator, get monitoring data and stop when necessary.
/// The primary goal for this interface is to run the software correlator from the epics
/// CA server. We use parallel thread to get the asynchronous behavior. This class represents
/// the child thread and CorrRunner the main thread.
/// @ingroup corrinterfaces
struct CorrRunnerThread  {
   
   /// @brief constructor
   /// @details
   /// @param[in] parent shared pointer to an instance of the main thread class
   /// @param[in] parset shared pointer to the parset
   CorrRunnerThread(const boost::shared_ptr<CorrRunner> &parent, const boost::shared_ptr<LOFAR::ParameterSet> &parset);
   
   /// @brief the entry point for the parallel thread
   void operator()();
   
private:
   /// @brief shared pointer to an instance of the main thread class (to allow status update)   
   boost::shared_ptr<CorrRunner> itsParent;
   
   /// @brief shared pointer to the parset with parameters
   boost::shared_ptr<LOFAR::ParameterSet> itsParset;
};

} // namespace swcorrelator

} // namespace askap


#endif // #ifndef ASKAP_CORRINTERFACES_CORR_RUNNER_THREAD_H

