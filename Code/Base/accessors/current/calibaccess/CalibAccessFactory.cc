/// @file
/// @brief factory creating calibration parameter accessor
/// @details This factory creates an actual instance of the calibration
/// parameter accessor and returns a generic instance via shared pointer.
/// Different implementations are possible: parset-based, casa table-based,
/// ICE-based. We could even load the actual code from a shared library as it
/// is done for gridders (may help to break dependencies). For now this factory
/// method is in calibaccess, but can be moved somewhere else later, especially
/// when an ICE-based implementation is ready to be plugged in.
///
/// @copyright (c) 2011 CSIRO
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
/// @author Max Voronkov <Maxim.Voronkov@csiro.au>

#include <calibaccess/CalibAccessFactory.h>
#include <calibaccess/ParsetCalSolutionSource.h>
#include <calibaccess/ParsetCalSolutionConstSource.h>
#include <askap/AskapError.h>

// logging stuff
#include <askap_accessors.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".calibaccess");

namespace askap {

namespace accessors {


/// @brief Build an appropriate "calibration source" class
/// @details This is a factory method generating a shared pointer to the calibration
/// solution source according to the parset file which allows write operation.
/// @param[in] parset parameters containing description of the class to be constructed 
/// (without leading Cimager., etc)
/// @return shared pointer to the calibration solution source object
boost::shared_ptr<ICalSolutionSource> CalibAccessFactory::rwCalSolutionSource(const LOFAR::ParameterSet &parset)
{
  const boost::shared_ptr<ICalSolutionSource> css = boost::dynamic_pointer_cast<ICalSolutionSource>(calSolutionSource(parset,false));
  ASKAPCHECK(css, "Unable to cast calibration solution source to read-write type. This shouldn't have happened. It's a bug!");
  return css;
}

/// @brief Build an appropriate "calibration source" class
/// @details This is a factory method generating a shared pointer to the calibration
/// solution source according to the parset file. The code for read-only and 
/// read-write operations is similar. Therefore, it is handy to contain it in one method only.
/// @param[in] parset parameters containing description of the class to be constructed 
/// (without leading Cimager., etc)
/// @param[in] readonly true if a read-only solution source is required
/// @return shared pointer to the calibration solution source object
boost::shared_ptr<ICalSolutionConstSource> CalibAccessFactory::calSolutionSource(const LOFAR::ParameterSet &parset, bool readonly)
{
   const std::string calAccType = parset.getString("calibaccess","parset");
   ASKAPCHECK(calAccType == "parset", 
       "Only parset-based implementation is supported by the calibration access factory at the moment; you request: "<<calAccType);
   const std::string fname = parset.getString("calibaccess.parset", "result.dat");
   ASKAPLOG_INFO_STR(logger, "Using implementation of the calibration solution accessor working with parset file "<<fname);
   boost::shared_ptr<ICalSolutionConstSource> result;
   if (readonly) {
      result.reset(new ParsetCalSolutionConstSource(fname));
   } else {
      result.reset(new ParsetCalSolutionSource(fname));
   }
   // further configuration fine tuning can happen here
   ASKAPDEBUGASSERT(result);
   return result;
}


} // namespace accessors

} // namespace askap

