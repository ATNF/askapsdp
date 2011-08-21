/// @file
/// @brief Naming convention for calibratable parameters
/// @details It is handy to use the same names of the calibratable parameters
/// in different parts of the code, e.g. when they're written to a parset file or
/// added as a model parameter. This class holds methods forming the name out of
/// antenna/beam/polarisation indices and parsing the string name to get these
/// indices back. 
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

#ifndef ASKAP_ACCESSORS_CAL_PARAM_NAME_HELPER_H
#define ASKAP_ACCESSORS_CAL_PARAM_NAME_HELPER_H

#include <calibaccess/JonesIndex.h>

// casa includes
#include <measures/Measures/Stokes.h>

// std includes
#include <utility>

namespace askap {

namespace accessors {

/// @brief Naming convention for calibratable parameters
/// @details It is handy to use the same names of the calibratable parameters
/// in different parts of the code, e.g. when they're written to a parset file or
/// added as a model parameter. This class holds methods forming the name out of
/// antenna/beam/polarisation indices and parsing the string name to get these
/// indices back. 
/// @ingroup calibaccess
struct CalParamNameHelper {
  /// @brief form the name of the parameter
  /// @details This method converts index and polarisation descriptor into a string name
  /// @param[in] index antenna/beam index
  /// @param[in] par parameter to get the name for as StokesTypes. XX,YY,XY and YX correspond to 
  /// parallel-hand gains g11 and g22 and cross-pol leakages d12 and d21, respectively
  /// @return string name of the parameter
  static std::string paramName(const JonesIndex &index, casa::Stokes::StokesTypes par);
  
  /// @brief form the name of the parameter
  /// @details This version works with explicit antenna and beam indices
  /// @param[in] ant antenna index
  /// @param[in] beam beam index
  /// @param[in] par parameter to get the name for as StokesTypes. XX,YY,XY and YX correspond to 
  /// parallel-hand gains g11 and g22 and cross-pol leakages d12 and d21, respectively
  /// @return string name of the parameter
  inline static std::string paramName(const casa::uInt ant, const casa::uInt beam, casa::Stokes::StokesTypes par)
      { return paramName(JonesIndex(ant,beam), par); }  

  /// @brief parse the name of the parameter
  /// @details This method is a reverse of paramName. It receive the string with the parameter
  /// name and parses it to extract antenna/beam indices and polarisation descriptor 
  /// (XX,YY,XY and YX correspond to parallel-hand gains g11 and g22 and cross-pol leakages d12 and d21,
  /// respectively).
  /// @param[in] name full name of the parameter (e.g. gain.g11.1.3)
  /// @return a pair with antenna/beam index as the first field and polarisation descriptor as the second
  /// @note An exception is thrown if parameter name is malformed
  static std::pair<JonesIndex, casa::Stokes::StokesTypes> parseParam(const std::string &name);  
};

} // namespace accessors

} // namespace askap

#endif // #ifndef ASKAP_ACCESSORS_CAL_PARAM_NAME_HELPER_H

