/// @file
///
/// @copyright (c) 2010 CSIRO
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

#ifndef ASKAP_CP_INGEST_CHANNELFLAGTASK_H
#define ASKAP_CP_INGEST_CHANNELFLAGTASK_H

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "cpcommon/VisChunk.h"
#include "utils/DelayEstimator.h"

// casa includes
#include <casa/Arrays/Matrix.h>


// Local package includes
#include "ingestpipeline/ITask.h"
#include "configuration/Configuration.h" // Includes all configuration attributes too
#include "configuration/BaselineMap.h"

// std includes
#include <fstream>
#include <string>
#include <vector>

namespace askap {
namespace cp {
namespace ingest {

/// @brief task for flagging selected channels based on an ascii file
/// @details This task is intended to be used in early commissioning experiments. Due to imperfections of the
/// correlator/early system there are many spikes in the data which complicate initial setup and analysis. Although
/// we can take care of them off-line using cflag, it is handy to be able to see clear data in the on-the-fly monitoring
/// and average in frequency on-the-fly. 
/// This class is not intended to survive in its current form in the long term. It will probably have some logic
/// hard coded (to suit the early commissioning tests). It is not intended to be used in the MPI case
class ChannelFlagTask : public askap::cp::ingest::ITask {
public:

   /// @breif Constructor
   /// @param[in] parset the configuration parameter set.
   /// @param[in] config configuration
   ChannelFlagTask(const LOFAR::ParameterSet& parset,
             const Configuration& config);

   /// @brief destructor
   ~ChannelFlagTask();

   /// @brief Flag seleted channels in the specified VisChunk.
   /// @details This method applies static flags to excise the spikes like
   /// CFB DC offset. Note, the intention is to run this task early in the chain
   /// to work on full resolution. There is no check of any kind that the supplied
   /// channel numbers are valid.
   ///
   /// @param[in,out] chunk  the instance of VisChunk for which the
   ///                       flags will be applied.
   virtual void process(askap::cp::common::VisChunk::ShPtr chunk);

protected:   
   /// @details Flag  one row of data.
   /// @param[in,out] vis vis spectrum for the given baseline/pol index to work with
   /// @param[in,out] flag vis spectrum for the given baseline/pol index to work with
   /// @param[in] baseline baseline ID 
   /// @param[in] beam beam ID
   void processRow(casa::Vector<casa::Complex> &vis, casa::Vector<casa::Bool> &flag, const casa::uInt baseline, const casa::uInt beam);

private:

   /// @brief list of channels to flag for each baselineid
   std::vector<std::vector<size_t> > itsChannelsToFlag;

   /// @brief baselines/polarisation indices to flag
   /// @details One can setup a subset of baselines to flag.
   /// The mapping is setup the same way as for the 
   /// main baseline map, but parset prefixes are different, for example:
   ///
   /// tasks.ChannelFlag.params.baselineids = [0]
   /// tasks.ChannelFlag.params.0 = [1,2,XX]
   /// tasks.ChannelFlag.params.flagfiles = [channel_list.txt]
   ///
   BaselineMap itsBaselineMap;

}; // ChannelFlagTask class

} // ingest

} // cp

} // askap

#endif // #ifndef ASKAP_CP_INGEST_CHANNELFLAGTASK_H

