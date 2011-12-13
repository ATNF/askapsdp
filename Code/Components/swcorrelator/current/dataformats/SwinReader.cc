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
#include <inttypes.h>
#include <utils/PolConverter.h>


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

/// @brief constructor
/// @details The DiFX output knows nothing about the beam number.
/// We will assign some beam id later when the data are written into MS.
/// This class is beam agnostic. The number of channels has to be set up
/// externally because it is not present in the file. If it is wrong, 
/// everything would go out of sync and reading would fail. This version of
/// the constructor creates a reader in the detached state. A call to assign
/// is required before reading can happen.
/// @param[in] nchan number of spectral channels
SwinReader::SwinReader(const casa::uInt nchan) : itsUVW(3,0.), 
    itsVisibility(nchan, casa::Complex(0.,0.))
{
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
  if (hasMore()) {
      for (casa::uInt chan = 0; chan<itsVisibility.nelements(); ++chan) {
           float re = 0., im = 0.;
           itsStream->read((char*) &re, sizeof(float));
           itsStream->read((char*) &im, sizeof(float));
           ASKAPCHECK(*itsStream, "Error while reading the stream, channel="<<chan);
           itsVisibility[chan] = casa::Complex(re,im);
      }
  }
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
   ASKAPCHECK(itsStream, "An attempt to read from a stream which is closed"); 
   if (!(*itsStream)) {
       itsStream.reset();
       return;
   }              
   int32_t intBuf;
   itsStream->read((char*)&intBuf, 4);
   ASKAPCHECK(intBuf == int32_t(0xFF00FF00), "Sync word is not as expected ("<<std::hex<<intBuf<<
              ") wrong file format or mismanaged read (i.e. wrong number of channels)");
   if (!(*itsStream)) {
       itsStream.reset();
       return;
   }              
   itsStream->read((char*)&intBuf, 4);
   ASKAPCHECK(intBuf == 1, "Expect header version 1, you have "<<intBuf);
   // baseline
   itsStream->read((char*)&intBuf, 4);
   const int ant1 = intBuf / 256 - 1;
   const int ant2 = intBuf % 256 - 1;
   ASKAPCHECK((ant1 < 256) && (ant1 >=0), "Illegal 1st antenna ID: "<<ant1+1<<" baseline index "<<intBuf);
   ASKAPCHECK((ant2 < 256) && (ant2 >=0), "Illegal 2nd antenna ID: "<<ant2+1<<" baseline index "<<intBuf);
   itsBaseline.first = casa::uInt(ant1);
   itsBaseline.second = casa::uInt(ant2);
   // mjd
   itsStream->read((char*)&intBuf, 4);
   double doubleBuf;
   // seconds
   itsStream->read((char*)&doubleBuf, 8);
   itsEpoch = casa::MEpoch(casa::MVEpoch(double(intBuf), doubleBuf / 86400.), casa::MEpoch::Ref(casa::MEpoch::UTC));
   // this will read config, source and freq indices, which we ignore for the moment
   itsStream->read((char*)&intBuf, 4);
   itsStream->read((char*)&intBuf, 4);
   itsStream->read((char*)&intBuf, 4);
   // stokes descriptor
   char polBuf[3];
   polBuf[2] = 0;
   itsStream->read(polBuf, 2);
   const casa::Vector<casa::Stokes::StokesTypes> stokesBuf = 
           scimath::PolConverter::fromString(polBuf);
   ASKAPCHECK(stokesBuf.nelements() == 1, "Expected only one element in the stokes vector, you have "<<stokesBuf.nelements());
   itsStokes = stokesBuf[0];
   // pulsar bin - ignored
   itsStream->read((char*)&intBuf, 4);
   // weight - ignored for now
   itsStream->read((char*)&doubleBuf, 8);
   ASKAPDEBUGASSERT(itsUVW.nelements() == 3);
   itsStream->read((char*)&doubleBuf, 8);
   itsUVW[0] = doubleBuf;
   itsStream->read((char*)&doubleBuf, 8);
   itsUVW[1] = doubleBuf;
   itsStream->read((char*)&doubleBuf, 8);
   itsUVW[2] = doubleBuf;              
}


} // namespace swcorrelator

} // namespace askap