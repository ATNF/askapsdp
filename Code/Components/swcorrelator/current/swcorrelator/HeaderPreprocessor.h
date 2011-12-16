/// @file 
///
/// @brief low-level operations on the header
/// @details We need some configurable flexibility dealing with the incoming stream. 
/// This class encapsulates low-level hacking operations on the buffer header to
/// allow necessary substutions. Index manipulation is done via IndexConverter.
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

#ifndef ASKAP_SWCORRELATOR_HEADER_PREPROCESSOR
#define ASKAP_SWCORRELATOR_HEADER_PREPROCESSOR

// own includes
#include <swcorrelator/IndexConverter.h>
#include <swcorrelator/BufferHeader.h>

// boost includes
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

// other 3rd party
#include <Common/ParameterSet.h>

namespace askap {

namespace swcorrelator {

/// @brief low-level operations on the header
/// @details We need some configurable flexibility dealing with the incoming stream. 
/// This class encapsulates low-level hacking operations on the buffer header to
/// allow necessary substutions. Index manipulation is done via IndexConverter.
/// @ingroup swcorrelator
class HeaderPreprocessor : private boost::noncopyable {
public:
  /// @brief constructor, extracts parameters from the parset
  /// @details
  /// @param[in] parset parset with parameters
  explicit HeaderPreprocessor(const LOFAR::ParameterSet &parset);  
  
  /// @brief update the header
  /// @param[in] hdr a reference to header
  /// @return true if there is no valid mapping for this header and the data have to be rejected,
  /// false otherwise
  /// @note If the header is rejected, it remains unchanged (to help with the debugging). 
  bool updateHeader(BufferHeader &hdr) const;
  
private:
  /// @brief antenna index converter
  boost::shared_ptr<IndexConverter> itsAntIDConverter;
  /// @brief beam index converter
  boost::shared_ptr<IndexConverter> itsBeamIDConverter;
  /// @brief freq index converter
  boost::shared_ptr<IndexConverter> itsFreqIDConverter;  
  /// @brief if true beams become antennas and antennas become beams
  /// @details If done, swap happens early on before any index conversion takes place
  bool itsSwapBeamAndAnt;
};


} // namespace swcorrelator

} // namespace askap


#endif // #ifndef ASKAP_SWCORRELATOR_HEADER_PREPROCESSOR

