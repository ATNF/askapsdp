/// @file 
///
/// @brief Reader of DiFX SWIN format output
/// @details This class allows to access data stored in the SWIN format
/// (produced by DiFX). We use it to convert DiFX output directly into a MS.
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

#ifndef ASKAP_DATAFORMATS_SWIN_READER
#define ASKAP_DATAFORMATS_SWIN_READER

// casa includes
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL.h>
#include <measures/Measures/Stokes.h>
#include <measures/Measures/MEpoch.h>

// std includes
#include <string>
#include <fstream>

// boost includes
#include <boost/shared_ptr.hpp>

namespace askap {

namespace swcorrelator {

/// @brief Reader of DiFX SWIN format output
/// @details This class allows to access data stored in the SWIN format
/// (produced by DiFX). We use it to convert DiFX output directly into a MS.
/// This class acts as an iterator over data stored in the file. It can be
/// rewind to the start of a new or existing file.
/// @ingroup dataformats
class SwinReader {
public:
   /// @brief constructor
   /// @details The DiFX output knows nothing about the beam number.
   /// We will assign some beam id later when the data are written into MS.
   /// This class is beam agnostic. The number of channels has to be set up
   /// externally because it is not present in the file. If it is wrong, 
   /// everything would go out of sync and reading would fail.
   /// @param[in] name file name
   /// @param[in] nchan number of spectral channels
   SwinReader(const std::string &name, const casa::uInt nchan);

   /// @brief constructor
   /// @details The DiFX output knows nothing about the beam number.
   /// We will assign some beam id later when the data are written into MS.
   /// This class is beam agnostic. The number of channels has to be set up
   /// externally because it is not present in the file. If it is wrong, 
   /// everything would go out of sync and reading would fail. This version of
   /// the constructor creates a reader in the detached state. A call to assign
   /// is required before reading can happen.
   /// @param[in] nchan number of spectral channels
   explicit SwinReader(const casa::uInt nchan);
   
   /// @brief start reading the same file again
   void rewind();
   
   /// @brief assign a new file and start iteration from the beginning
   /// @details
   /// @param[in] name file name
   void assign(const std::string &name);
   
   /// @brief check there are more data available
   /// @return true if there are more data in the current file
   bool hasMore() const;
   
   /// @brief advance to the next visibility chunk
   void next();
   
   /// @brief obtain current UVW
   /// @return vector with uvw
   casa::Vector<double> uvw() const;
   
   /// @brief obtain visibility vector
   /// @details Number of elements is the number of spectral channels.
   /// @return visibility vector corresponding to the current record 
   casa::Vector<casa::Complex> visibility() const;
   
   /// @brief get current polarisation
   /// @details stokes descriptor corresponding to the current polarisation
   casa::Stokes::StokesTypes stokes() const;
   
   /// @brief pair of antennas corresponding to the current baseline
   /// @details antenna IDs are zero-based.
   /// @return pair of antenna IDs
   std::pair<casa::uInt, casa::uInt> baseline() const;

   /// @brief time corresponding to the current baseline
   /// @return epoch measure
   casa::MEpoch epoch() const;   
   
   /// @brief get frequency ID of the current record
   /// @return frequency ID
   casa::uInt freqID() const;
   
protected:
   /// @brief helper method to read the header   
   void readHeader();
   
   /// @brief helper method to check sync word
   /// @details We attempt to read the sync word corresponding to
   /// the next record immediately after the previous record has been read.
   /// This allows us to detect the end of file.
   /// @note An exception is thrown if the sync word is not as expected
   void readSyncWord();
   
private:
   /// @brief current file name
   std::string itsFileName;
   
   /// @brief file stream to work with
   /// @details An empty shared pointer indicates that the end of file is reached
   boost::shared_ptr<std::ifstream> itsStream;
   
   /// @brief UVWs
   casa::Vector<double> itsUVW;
   
   /// @brief visibilities
   casa::Vector<casa::Complex> itsVisibility;
   
   /// @brief polarisation descriptor
   casa::Stokes::StokesTypes itsStokes;
   
   /// @brief baseline
   std::pair<casa::uInt, casa::uInt> itsBaseline;
   
   /// @brief epoch
   casa::MEpoch itsEpoch; 
        
   /// @brief frequency ID
   casa::uInt itsFreqID;
};

} // namespace swcorrelator

} // namespace askap


#endif // ASKAP_DATAFORMATS_SWIN_READER

