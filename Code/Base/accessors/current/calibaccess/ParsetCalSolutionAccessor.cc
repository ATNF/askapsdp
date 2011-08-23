/// @file
/// @brief Parset file-based implementation of the calibration solution accessor
/// @details This implementation is to be used with pre-existing code writing/reading
/// the parset directly and with a number of tests. It doesn't implement anything related
/// to bandpass table (and always returns 1. for bandpass and throws an exception if one
/// attempts to write a bandpass). This is because none of the code written so far deals with
/// bandpass tables (and any future code will used in conjunction with more a flexible 
/// implementation, e.g. table-based). This implementation is just to convert the legacy code.
/// There is only one implementation of this class which is used for both reading and writing.
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

#include <calibaccess/ParsetCalSolutionAccessor.h>
#include <askap/AskapError.h>

// LOFAR
#include <Common/ParameterSet.h>
#include <Common/Exceptions.h>

// logging stuff
#include <askap_accessors.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".calibaccess");

// std includes
#include <vector>
#include <fstream>

namespace askap {

namespace accessors {

/// @brief constructor 
/// @details It reads the given parset file, if it exists, and caches the values. Write
/// operations are performed via this cache which is stored into file in the destructor.
/// @param[in] parset parset file name
ParsetCalSolutionAccessor::ParsetCalSolutionAccessor(const std::string &parset) : itsParsetFileName(parset), 
        itsWriteRequired(false), itsFirstWrite(true)
{
  try {
     cache() << LOFAR::ParameterSet(itsParsetFileName);
     ASKAPLOG_INFO_STR(logger, "Successfully read calibration solution from a parset file "<<itsParsetFileName);
  }
  catch (const LOFAR::APSException &) {
     // nothing read, this is probably a write case
     ASKAPLOG_INFO_STR(logger, "Set up ParsetCalSolutionAccessor to write results into "<<itsParsetFileName);
  } 
}        
  
/// @brief destructor, stores the cache
/// @details Actual write operation is performed here. All values stored in the cache are written
/// to disk, if the appropriate flag is set (there was at least one write).
ParsetCalSolutionAccessor::~ParsetCalSolutionAccessor()
{
  if (itsWriteRequired) {
      ASKAPLOG_INFO_STR(logger, "Writing out calibration results into a parset file "<<itsParsetFileName);
      const std::vector<std::string> parlist = cache().names();
      std::ofstream os(itsParsetFileName.c_str());
      for (std::vector<std::string>::const_iterator it = parlist.begin(); 
           it != parlist.end(); ++it) {
           const casa::Complex val = cache().complexValue(*it);
           os<<*it<<" = ["<<real(val)<<","<<imag(val)<<"]"<<std::endl;
      }     
  }
}
    
/// @brief helper method executed on every write
/// @details This method manages flags associated with the write operation
void ParsetCalSolutionAccessor::prepareToWrite()
{
  itsWriteRequired = true;
  if (itsFirstWrite) {
      itsFirstWrite = false;
      const std::vector<std::string> names = cache().names();
      if (names.size()) {
          ASKAPLOG_WARN_STR(logger, "Overwriting existing parset "<<itsParsetFileName<<
              " with calibration parameters ("<<names[0]<<",...)");
      }
      cache().reset();
  }
}

/// @brief set gains (J-Jones)
/// @details This method writes parallel-hand gains for both 
/// polarisations (corresponding to XX and YY)
/// @param[in] index ant/beam index 
/// @param[in] gains JonesJTerm object with gains and validity flags
void ParsetCalSolutionAccessor::setGain(const JonesIndex &index, const JonesJTerm &gains)
{
  prepareToWrite();
  CachedCalSolutionAccessor::setGain(index, gains);
}
   
/// @brief set leakages (D-Jones)
/// @details This method writes cross-pol leakages  
/// (corresponding to XY and YX)
/// @param[in] index ant/beam index 
/// @param[in] leakages JonesDTerm object with leakages and validity flags
void ParsetCalSolutionAccessor::setLeakage(const JonesIndex &index, const JonesDTerm &leakages)
{
  prepareToWrite();
  CachedCalSolutionAccessor::setLeakage(index,leakages);
}
  

} // namespace accessors

} // namespace askap

