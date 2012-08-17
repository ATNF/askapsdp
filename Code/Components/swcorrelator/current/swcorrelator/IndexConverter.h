/// @file 
///
/// @brief Map antenna and/or beam indices into a continuous range
/// @details During BETA3 experiments we plan to use antennas with
/// non-contiguous indices in the data stream (they correspond to 
/// actual antenna/beam numbers given in the data stream; and possibly
/// also one-based). This class simplifies mapping into a continuous 
/// range of indices. Same functionality is likely to be required for
/// beams. 
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

#ifndef ASKAP_SWCORRELATOR_INDEX_CONVERTER
#define ASKAP_SWCORRELATOR_INDEX_CONVERTER

#include <vector>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

namespace askap {

namespace swcorrelator {

/// @brief Map antenna and/or beam indices into a continuous range
/// @details During BETA3 experiments we plan to use antennas with
/// non-contiguous indices in the data stream (they correspond to 
/// actual antenna/beam numbers given in the data stream; and possibly
/// also one-based). This class simplifies mapping into a continuous 
/// range of indices. Same functionality is likely to be required for
/// beams. 
/// @ingroup swcorrelator
class IndexConverter : private boost::noncopyable {
public:
   /// @brief default index converter - no conversion
   IndexConverter();
   
   /// @brief setup conversion from a string
   /// @details Extracts rules like 1:0, 4:1, 5:2
   /// @param[in] indexMap string with the rules
   IndexConverter(const std::string &indexMap);
   
   /// @brief add mapping
   /// @param[in] in input index
   /// @param[in] target output index
   void add(const int in, const int target);
   
   /// @brief add mapping from string
   /// @param[in] indexMap string with the rules
   void add(const std::string &indexMap);
   
   /// @brief do the translation
   /// @param[in] in input index
   /// @return output index
   /// @note a negative index is returned if the input index is out of range
   int operator()(const int in)  const;
 
   /// @brief size of the map
   /// @details returns the number of conversions set up, i.e. zero if there are none
   /// @return number of conversion rules
   size_t nRules() const;
   
private:
   std::vector<int> itsMap; 
};

} // namespace swcorrelator

} // namespace askap


#endif // #ifndef ASKAP_SWCORRELATOR_INDEX_CONVERTER


