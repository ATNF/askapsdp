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

#include <dataformats/SwinReader.h>
#include <askap/AskapError.h>
#include <casa/OS/File.h>

namespace askap {

namespace swcorrelator {

/// @brief constructor
/// @details The DiFX output knows nothing about the beam number.
/// We will assign some beam id later when the data are written into MS.
/// This class is beam agnostic. The number of channels has to be set up
/// externally because it is not present in the file. If it is wrong, 
/// everything would go out of sync and reading would fail.
/// @param[in] name file name
/// @param[in] nchan number of spectral channels
SwinReader::SwinReader(const std::string &name, const casa::uInt nchan) :
    itsFileName(name), itsUVW(3,0.), itsVisibility(nchan, casa::Complex(0.,0.))
{
  // this would start the read and create a stream
  rewind();
}
   
/// @brief start reading the same file again
void SwinReader::rewind()
{
  // close the current stream, if it has been opened
  itsStream.reset();
  ASKAPCHECK(itsFileName!="", "Empty file name has been given");    
  ASKAPCHECK(casa::File(itsFileName).exists(), "File "<<itsFileName<<" does not exist!");  
  itsStream.reset(new std::ifstream(itsFileName.c_str()));
  next();
}
   
/// @brief assign a new file and start iteration from the beginning
/// @details
/// @param[in] name file name
void SwinReader::assign(const std::string &name)
{
  itsFileName = name;
  rewind();
}
   
/// @brief check there are more data available
/// @return true if there are more data in the current file
bool SwinReader::hasMore() const
{
  // empty shared pointer is a signature of the end of file
  return itsStream;
}
   
/// @brief advance to the next visibility chunk
void SwinReader::next()
{
  readHeader();
}
   
/// @brief obtain current UVW
/// @return vector with uvw
casa::Vector<double> SwinReader::uvw() const
{ 
  return itsUVW;
}
   
/// @brief obtain visibility vector
/// @details Number of elements is the number of spectral channels.
/// @return visibility vector corresponding to the current record 
casa::Vector<casa::Complex> SwinReader::visibility() const
{
  return itsVisibility;
}
   
/// @brief get current polarisation
/// @details stokes descriptor corresponding to the current polarisation
casa::Stokes::StokesTypes SwinReader::stokes() const
{
  return itsStokes;
}
   
/// @brief pair of antennas corresponding to the current baseline
/// @details antenna IDs are zero-based.
/// @return pair of antenna IDs
std::pair<casa::uInt, casa::uInt> SwinReader::baseline() const
{
  return itsBaseline;
}

/// @brief time corresponding to the current baseline
/// @return epoch measure
casa::MEpoch SwinReader::epoch() const
{
  return itsEpoch;
}
   
/// @brief helper method to read the header   
void SwinReader::readHeader()
{
  
}


} // namespace swcorrelator

} // namespace askap