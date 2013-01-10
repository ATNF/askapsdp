/// @file
///
/// Support for parallel statistics accululation to advise on imaging parameters
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

#include <parallel/AdviseParallel.h>
#include <askap/AskapError.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <dataaccess/TableDataSource.h>
#include <dataaccess/ParsetInterface.h>
#include <dataaccess/SharedIter.h>
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".parallel");

#include <casa/aips.h>
#include <casa/OS/Timer.h>


#include <vector>
#include <string>

namespace askap {

namespace synthesis {

/// @brief Constructor from ParameterSet
/// @details The parset is used to construct the internal state. We could
/// also support construction from a python dictionary (for example).
/// The command line inputs are needed solely for MPI - currently no
/// application specific information is passed on the command line.
/// @param comms communication object 
/// @param parset ParameterSet for inputs
AdviseParallel::AdviseParallel(askap::askapparallel::AskapParallel& comms, const LOFAR::ParameterSet& parset) :
    MEParallelApp(comms, parset), itsTangentDefined(false)
{
   itsWTolerance = parset.getDouble("wtolerance",-1.);
   if (parset.isDefined("tangent")) {
       const std::vector<std::string> direction = parset.getStringVector("tangent");
       ASKAPCHECK(direction.size() == 3, "Direction should have exactly 3 parameters, you have "<<direction.size());
       ASKAPCHECK(direction[2] == "J2000", "Only J2000 is implemented at the moment, you have requested "<<direction[2]);
      
       const double ra = SynthesisParamsHelper::convertQuantity(direction[0],"rad");
       const double dec = SynthesisParamsHelper::convertQuantity(direction[1],"rad");
       itsTangent = casa::MVDirection(ra,dec);
       itsTangentDefined = true;
   }
   itsNe.reset();
}    

/// @brief make the estimate
/// @details This method iterates over one or more datasets, accumulates and aggregates statistics. If
/// tangent point is not defined, two iterations are performed. The first one is to estimate the tangent
/// point and the second to obtain  
void AdviseParallel::estimate()
{
   if (itsTangentDefined) {
       itsEstimator.reset(new VisMetaDataStats(itsTangent, itsWTolerance));
       // we only need one iteration here
   } else {
       itsEstimator.reset(new VisMetaDataStats);
   }
   calcNE();
   if (!itsTangentDefined) {
       itsTangent = itsEstimator->centre();
       itsTangentDefined = true;
       ASKAPLOG_INFO_STR(logger, "Using tangent "<<printDirection(itsTangent)<<" (estimated most central direction)"); 
   }
}
   
/// @brief perform the accumulation for the given dataset
/// @details This method iterates over the given dataset, predicts visibilities according to the
/// model and subtracts these model visibilities from the original visibilities in the dataset.
/// This is the core operation of the doSubtraction method, which manages the parallel aspect of it.
/// All actual calculations are done inside this helper method.
/// @param[in] ms measurement set name
void AdviseParallel::calcOne(const std::string &ms)
{
   casa::Timer timer;
   timer.mark();
   ASKAPLOG_INFO_STR(logger, "Performing iteration to accumulate metadata statistics for " << ms );
   ASKAPDEBUGASSERT(itsEstimator);
   
   accessors::TableDataSource ds(ms, accessors::TableDataSource::MEMORY_BUFFERS, dataColumn());
   ds.configureUVWMachineCache(uvwMachineCacheSize(),uvwMachineCacheTolerance());      
   accessors::IDataSelectorPtr sel=ds.createSelector();
   sel << parset();
   accessors::IDataConverterPtr conv=ds.createConverter();
   conv->setFrequencyFrame(getFreqRefFrame(), "Hz");
   conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));
   conv->setEpochFrame(); // time since 0 MJD
   accessors::IDataSharedIter it=ds.createIterator(sel, conv);
   for (; it.hasMore(); it.next()) {
        // iteration over the dataset
        itsEstimator->process(*it);
   }
   
   ASKAPLOG_INFO_STR(logger, "Finished iteration for "<< ms << " in "<< timer.real()
                   << " seconds ");    
}
      
/// @brief calculate "normal equations", i.e. statistics for this dataset
void AdviseParallel::calcNE()
{
   if (itsComms.isWorker()) {
       ASKAPCHECK(itsNe, "Statistics estimator (stored as NormalEquations) is not defined");
       if (itsComms.isParallel()) {
           calcOne(measurementSets()[itsComms.rank()-1]);
           sendNE();
       } else {
          for (size_t iMs=0; iMs<measurementSets().size(); ++iMs) {
               calcOne(measurementSets()[iMs]);
          }
       }       
   }
}


} // namespace synthesis

} // namespace askap