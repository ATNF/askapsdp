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

#include <measurementequation/ImageParamsHelper.h>

#include <askap/AskapError.h>
#include <askap/AskapUtil.h>

namespace askap {

namespace synthesis {

/// @brief empty constructor
/// @details Full name to be specified later. This method of construction doesn't produce
/// a valid object until parse method is called.
ImageParamsHelper::ImageParamsHelper() : itsFacetX(-2), itsFacetY(-2) {}
   
/// @brief constructor with immediate parsing of a full name
/// @details This version construct an object and populates all fields with the parse
/// results.
/// @param[in] name full name to parse
ImageParamsHelper::ImageParamsHelper(const std::string &name) : itsFacetX(-2), itsFacetY(-2)
{
  parse(name);
}

/// @brief direct constructor of a facet name from constituents
/// @details This method constructs the object directly from the actual name
/// of the image and facet indices.
/// @param[in] name actual name of the image (without suffixes)
/// @param[in] xFacet facet index along the first axis
/// @param[in] yFacet facet index along the second axis
ImageParamsHelper::ImageParamsHelper(const std::string &name, int xFacet, int yFacet) :
              itsName(name), itsFacetX(xFacet), itsFacetY(yFacet)
{  
}
   
/// @brief parse the given string
/// @param[in] name full name to parse
void ImageParamsHelper::parse(const std::string &name) 
{
  size_t pos = name.rfind(".facet.");
  if (pos == std::string::npos) {
      // this is not a faceted image, just set flags and copy full name
      itsFacetX = -1;
      itsFacetY = -1;
      itsName = name;                      
  } else {
      // this is a single facet, we have to extract indices 
      itsName = name.substr(0,pos);
      pos+=7; // to move it to the start of numbers
      ASKAPCHECK(pos < name.size(), 
         "Name of the faceted parameter should contain facet indices at the end, you have "<<name);
      const size_t pos2 = name.find(".",pos);
      ASKAPCHECK((pos2 != std::string::npos) && (pos2+1<name.size()) && (pos2!=pos), 
          "Two numbers are expected in the parameter name for the faceted image, you have "<<name);
      itsFacetX = utility::fromString<int>(name.substr(pos,pos2-pos));
      itsFacetY = utility::fromString<int>(name.substr(pos2+1));
  }
  // todo: further parsing of the parameter name should go here. 
}

/// @brief obtain the facet number along the first axis
/// @return the facet number along the first axis
int ImageParamsHelper::facetX() const
{
  ASKAPDEBUGASSERT(itsFacetX>=0);
  return itsFacetX;
}

/// @brief obtain the facet number along the second axis
/// @return the facet number along the second axis
int ImageParamsHelper::facetY() const
{
  ASKAPDEBUGASSERT(itsFacetY>=0);
  return itsFacetY;
}


/// @brief obtain the full name of the image parameter
/// @details This method composes the full name of the parameter from 
/// the data stored internally. This returned full name should be the same 
/// as one passed in the parse method or in the constructor. This method can
/// be useful if this object is constructed directly without parsing a 
/// string and effectively represents a reverse operation.
std::string ImageParamsHelper::paramName() const
{
  ASKAPDEBUGASSERT(isValid());
  if (isFacet()) {
      return itsName+".facet."+utility::toString<int>(itsFacetX)+"."+
                               utility::toString<int>(itsFacetY);
  }
  return itsName;                            
}

} // namespace synthesis

} // namespace askap

