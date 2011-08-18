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

#include <calibaccess/CalParamNameHelper.h>
#include <askap/AskapError.h>
#include <askap/AskapUtil.h>

namespace askap {

namespace accessors {

/// @brief form the name of the parameter
/// @details This method converts index and polarisation descriptor into a string name
/// @param[in] index antenna/beam index
/// @param[in] par parameter to get the name for as StokesTypes. XX,YY,XY and YX correspond to 
/// parallel-hand gains g11 and g22 and cross-pol leakages d12 and d21, respectively
/// @return string name of the parameter
std::string CalParamNameHelper::paramName(const JonesIndex &index, casa::Stokes::StokesTypes par)
{
   std::string res;
   if ((par == casa::Stokes::XX) || (par == casa::Stokes::YY)) {
       res = "gain."; 
       res += (par == casa::Stokes::XX ? "g11." : "g22.");
   } else if ((par == casa::Stokes::XY) || (par == casa::Stokes::YX)) {
       res = "leakage."; 
       res += (par == casa::Stokes::XY ? "d12." : "d21.");
   } else {
       ASKAPTHROW(AskapError, 
           "Unsupported polarisation descriptor passed to ParsetCalSolutionAccessor::paramName, only XX,XY,YX and YY are allowed");
   }
   
   return res + utility::toString<casa::Short>(index.antenna()) +"."+ utility::toString<casa::Short>(index.beam());
}

/// @brief parse the name of the parameter
/// @details This method is a reverse of paramName. It receive the string with the parameter
/// name and parses it to extract antenna/beam indices and polarisation descriptor 
/// (XX,YY,XY and YX correspond to parallel-hand gains g11 and g22 and cross-pol leakages d12 and d21,
/// respectively).
/// @param[in] name full name of the parameter (e.g. gain.g11.1.3)
/// @return a pair with antenna/beam index as the first field and polarisation descriptor as the second
/// @note An exception is thrown if parameter name is malformed
std::pair<JonesIndex, casa::Stokes::StokesTypes> CalParamNameHelper::parseParam(const std::string &name)
{
  const size_t pos = name.find(".");
  ASKAPCHECK((pos != std::string::npos) && (pos + 1 != name.size()), 
             "Parameter name should be in the form something.something.ant.beam; you have "<<name);
  const std::string what = name.substr(0,pos);
  ASKAPCHECK((what == "gain") || (what == "leakage"), "Only gain and leakage parameters are supported, you have "<<name);
  const size_t pos2 = name.find(".", pos + 1);
  ASKAPCHECK((pos2 != std::string::npos) && (pos2 + 1 != name.size()) && (pos + 1 != pos2), 
             "Parameter name should be in the form something.something.ant.beam; you have "<<name);
  const std::string pol = name.substr(pos + 1, pos2 - pos - 1);
  casa::Stokes::StokesTypes polDescriptor;
  if (what  == "gain") {
      ASKAPCHECK((pol == "g11") || (pol == "g22"), "Unrecognised polarisation product "<<pol<<" in "<<name);
      polDescriptor = (pol == "g11" ? casa::Stokes::XX : casa::Stokes::YY);
  } else if (what == "leakage") {
      ASKAPCHECK((pol == "d12") || (pol == "d21"), "Unrecognised polarisation product "<<pol<<" in "<<name);
      polDescriptor = (pol == "d12" ? casa::Stokes::XY : casa::Stokes::YX);
  } else {
     ASKAPTHROW(AskapError, "This line should never be executed!");
  }
  const size_t pos3 = name.find(".", pos2 + 1);
  ASKAPCHECK((pos3 != std::string::npos) && (pos3 + 1 != name.size()) && (pos2 + 1 != pos3), 
             "Parameter name should be in the form something.something.ant.beam; you have "<<name);
  const casa::Short ant = utility::fromString<casa::Short>(name.substr(pos2 + 1, pos3 - pos2 - 1));
  const casa::Short beam = utility::fromString<casa::Short>(name.substr(pos3 + 1));
  return std::pair<JonesIndex, casa::Stokes::StokesTypes>(JonesIndex(ant,beam),polDescriptor);
}

} // namespace accessors

} // namespace askap
