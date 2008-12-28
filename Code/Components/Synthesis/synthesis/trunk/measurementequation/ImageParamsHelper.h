/// @copyright (c) 2007 CSIRO
/// @brief Helper class for dealing with Params representing images
/// @details Working on the faceting, it was found that a parser for
/// image parameter name was required. It should return a number of values, so a 
/// separate class seems to be a better alternative than a static member of the
/// existing SynthesisParamsHelper class. Some methods from the latter will probably
/// migrate eventually into this class.
///  
///
///
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
///
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef IMAGE_PARAMS_HELPER_H
#define IMAGE_PARAMS_HELPER_H

#include <string>

namespace askap {

namespace synthesis {

/// @brief Helper class for dealing with Params representing images
/// @details Working on the faceting, it was found that a parser for
/// image parameter name was required. It should return a number of values, so a 
/// separate class seems to be a better alternative than a static member of the
/// existing SynthesisParamsHelper class. Some methods from the latter will probably
/// migrate eventually into this class.
/// @todo improve parsing to add polarisation/Urvashi's decomposition into Taylor series
/// @ingroup measurementequation
class ImageParamsHelper {
public:
   /// @brief empty constructor
   /// @details Full name to be specified later. This method of construction doesn't produce
   /// a valid object until parse method is called.
   ImageParamsHelper();
   
   /// @brief constructor with immediate parsing of a full name
   /// @details This version construct an object and populates all fields with the parse
   /// results.
   /// @param[in] name full name to parse
   ImageParamsHelper(const std::string &name);
   
   /// @brief parse the given string
   /// @param[in] name full name to parse
   void parse(const std::string &name);
  
   /// @brief obtain the actual name of the parameter without all suffixes
   /// @return the name without suffixes
   inline const std::string& name() const { return itsName;}
   
   /// @brief obtain the facet number along the first axis
   /// @return the facet number along the first axis
   int facetX() const;  

   /// @brief obtain the facet number along the second axis
   /// @return the facet number along the second axis
   int facetY() const;  
   
private:
   /// @brief name of the current parameter (cut before all suffixes)
   std::string itsName;

   /// @brief facet number along the first axis
   /// @note The value is negative for non-faceted image
   int itsFacetX;

   /// @brief facet number along the second axis
   /// @note The value is negative for non-faceted image
   int itsFacetY;   
};

} // namespace synthesis

} // namespace askap

#endif // #ifndef IMAGE_PARAMS_HELPER_H
