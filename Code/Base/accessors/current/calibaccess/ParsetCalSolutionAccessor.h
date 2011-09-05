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

#ifndef PARSET_CAL_SOLUTION_ACCESSOR_H
#define PARSET_CAL_SOLUTION_ACCESSOR_H

// own includes
#include <calibaccess/CachedCalSolutionAccessor.h>

// std includes
#include <string>

namespace askap {

namespace accessors {

/// @brief Parset file-based implementation of the calibration solution accessor
/// @details This implementation is to be used with pre-existing code writing/reading
/// the parset directly and with a number of tests. It doesn't implement anything related
/// to bandpass table (and always returns 1. for bandpass and throws an exception if one
/// attempts to write a bandpass). This is because none of the code written so far deals with
/// bandpass tables (and any future code will used in conjunction with more a flexible 
/// implementation, e.g. table-based). This implementation is just to convert the legacy code.
/// There is only one implementation of this class which is used for both reading and writing.
/// @ingroup calibaccess
class ParsetCalSolutionAccessor : public CachedCalSolutionAccessor  {
public:
  /// @brief constructor 
  /// @details It reads the given parset file, if it exists, and caches the values. Write
  /// operations are performed via this cache which is stored into file in the destructor.
  /// @param[in] parset parset file name
  /// @param[in] readonly if true, additional checks are done that file exists, otherwise
  /// it is assumed that we may write a new file
  explicit ParsetCalSolutionAccessor(const std::string &parset, bool readonly = false);
  
  /// @brief destructor, stores the cache
  /// @details Actual write operation is performed here. All values stored in the cache are written
  /// to disk, if the appropriate flag is set (there was at least one write).
  virtual ~ParsetCalSolutionAccessor();
  
  
  // override write methods to handle caching properly
    
  /// @brief set gains (J-Jones)
  /// @details This method writes parallel-hand gains for both 
  /// polarisations (corresponding to XX and YY)
  /// @param[in] index ant/beam index 
  /// @param[in] gains JonesJTerm object with gains and validity flags
  virtual void setGain(const JonesIndex &index, const JonesJTerm &gains);
   
  /// @brief set leakages (D-Jones)
  /// @details This method writes cross-pol leakages  
  /// (corresponding to XY and YX)
  /// @param[in] index ant/beam index 
  /// @param[in] leakages JonesDTerm object with leakages and validity flags
  virtual void setLeakage(const JonesIndex &index, const JonesDTerm &leakages);
   
protected:
    
  /// @brief helper method executed on every write
  /// @details This method manages flags associated with the write operation and should be called prior
  /// to adding of any new values into cache. 
  /// @note It cleans the cache on the first write.
  void prepareToWrite();
  
private:
  /// @brief parset file name for reading or writing
  std::string itsParsetFileName;
  
  /// @brief true, if write is required at the end
  bool itsWriteRequired;
  
  /// @brief true, if no write operations took place so far
  /// @details We need to combine reading and writing API in the same class. It is not known in
  /// advance how each particular instance is going to be used. This flag ensures that the output
  /// is overwritten completely after the first write operation and the old content of the disk
  /// file does not interfere with the new result
  bool itsFirstWrite;
};

} // namespace accessors

} // namespace askap

#endif // #ifndef PARSET_CAL_SOLUTION_ACCESSOR_H

