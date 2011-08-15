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
#include <calibaccess/ICalSolutionAccessor.h>
#include <fitting/Params.h>

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
class ParsetCalSolutionAccessor : virtual public ICalSolutionAccessor {
public:
  /// @brief constructor 
  /// @details It reads the given parset file, if it exists, and caches the values. Write
  /// operations are performed via this cache which is stored into file in the destructor.
  /// @param[in] parset parset file name
  explicit ParsetCalSolutionAccessor(const std::string &parset);
  
  /// @brief destructor, stores the cache
  /// @details Actual write operation is performed here. All values stored in the cache are written
  /// to disk, if the appropriate flag is set (there was at least one write).
  virtual ~ParsetCalSolutionAccessor();
  
  // implementation of abstract methods of the interface
  
  /// @brief obtain gains (J-Jones)
  /// @details This method retrieves parallel-hand gains for both 
  /// polarisations (corresponding to XX and YY). If no gains are defined
  /// for a particular index, gains of 1. with invalid flags set are
  /// returned.
  /// @param[in] index ant/beam index 
  /// @return JonesJTerm object with gains and validity flags
  virtual JonesJTerm gain(const JonesIndex &index) const;
   
  /// @brief obtain leakage (D-Jones)
  /// @details This method retrieves cross-hand elements of the 
  /// Jones matrix (polarisation leakages). There are two values
  /// (corresponding to XY and YX) returned (as members of JonesDTerm 
  /// class). If no leakages are defined for a particular index,
  /// zero leakages are returned with invalid flags set. 
  /// @param[in] index ant/beam index
  /// @return JonesDTerm object with leakages and validity flags
  virtual JonesDTerm leakage(const JonesIndex &index) const;
   
  /// @brief obtain bandpass (frequency dependent J-Jones)
  /// @details This method retrieves parallel-hand spectral
  /// channel-dependent gain (also known as bandpass) for a
  /// given channel and antenna/beam. The actual implementation
  /// does not necessarily store these channel-dependent gains
  /// in an array. It could also implement interpolation or 
  /// sample a polynomial fit at the given channel (and 
  /// parameters of the polynomial could be in the database). If
  /// no bandpass is defined (at all or for this particular channel),
  /// gains of 1.0 are returned (with invalid flag is set).
  /// @param[in] index ant/beam index
  /// @param[in] chan spectral channel of interest
  /// @return JonesJTerm object with gains and validity flags
  virtual JonesJTerm bandpass(const JonesIndex &index, const casa::uInt chan) const;
  
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
   
  /// @brief set gains for a single bandpass channel
  /// @details This method writes parallel-hand gains corresponding to a single
  /// spectral channel (i.e. one bandpass element).
  /// @param[in] index ant/beam index 
  /// @param[in] bp JonesJTerm object with gains for the given channel and validity flags
  /// @param[in] chan spectral channel
  /// @note We may add later variants of this method assuming that the bandpass is
  /// approximated somehow, e.g. by a polynomial. For simplicity, for now we deal with 
  /// gains set explicitly for each channel.
  virtual void setBandpass(const JonesIndex &index, const JonesJTerm &bp, const casa::uInt chan);
  
private:
  /// @brief parset file name for reading or writing
  std::string itsParsetFileName;
  
  /// @brief true, if write is required at the end
  bool itsWriteRequired;
  
  /// @brief cache of parameters
  scimath::Params itsCache;
};

} // namespace accessors

} // namespace askap

#endif // #ifndef PARSET_CAL_SOLUTION_ACCESSOR_H

