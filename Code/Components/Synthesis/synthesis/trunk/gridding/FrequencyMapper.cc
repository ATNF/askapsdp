/// @file 
/// @brief Mapping between frequency channels and image planes
/// @details This class provides mapping between image (grid) planes and frequency
/// channels. One image plane can correspond to a number of accessor planes 
/// (multi-frequency synthesis). This class is used inside TableVisGridder.
///
/// @copyright (c) 2008 CSIRO
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

#include <gridding/FrequencyMapper.h>
#include <askap/AskapError.h>

using namespace askap;
using namespace synthesis;

/// @brief default constructor
/// @details The class is left in an uninitialised state
FrequencyMapper::FrequencyMapper() : itsImageNChan(-1) {}
   
/// @brief constructor doing initialisation
/// @details
/// @param[in] axes axes object containing spectral axis of the image cube
/// @param[in] nchan number of frequency channels in the image cube
/// @note an exception is thrown if axes object doesn't contain the spectral axis
FrequencyMapper::FrequencyMapper(const scimath::Axes &axes, int nchan) : itsImageNChan(nchan) 
{
  setupImage(axes, nchan);
}
   
/// @brief setup image
/// @details 
/// @param[in] axes axes object containing spectral axis of the image cube
/// @param[in] nchan number of frequency channels in the image cube
/// @note an exception is thrown if axes object doesn't contain the spectral axis
void FrequencyMapper::setupImage(const scimath::Axes &axes, int nchan)
{
   ASKAPCHECK(axes.has("FREQUENCY"), "FREQUENCY axis is missing in axes object passed to FrequencyMapper:setupImage");
   ASKAPASSERT(nchan>0);
   itsStartFreq = axes.start("FREQUENCY");
   itsEndFreq = axes.end("FREQUENCY");
   itsImageNChan = nchan;
}
   
/// @brief setup mapping 
/// @details 
/// This method sets up actual mapping between image and accessor channels. Only 
/// vector returned by accessor's frequency method is required.    
/// @param[in] freqs vector with frequencies
/// @note The current assumption is that no regridding is required. Therefore, it is
/// expected that no fractional channel offset can occur.
void FrequencyMapper::setupMapping(const casa::Vector<casa::Double> &freqs)
{
   ASKAPCHECK(itsImageNChan>0, "An attempt to call setupMapping for uninitialised FrequencyMapper");
   itsMap.resize(freqs.nelements());
   const double increment = (itsEndFreq - itsStartFreq)/double(itsImageNChan);
   for (casa::uInt chan=0; chan<freqs.nelements(); ++chan) {
        const int imgChan = int((freqs[chan]-itsStartFreq)/increment*1000.)/1000;
        if (imgChan<0 || imgChan>=itsImageNChan) {
            itsMap[chan] = -1;
        } else {
            itsMap[chan] = imgChan;
        }
   }
}

/// @brief test whether the given channel is mapped 
/// @details The measurement does not necessarily contribute to the cube which is being imaged.
/// This method allows to check whether some mapping exists. Operator() throws the exception if
/// it is called for a channel without a mapping.
/// @param[in] chan accessor channel
/// @return true, if the given channel has a mapping
bool FrequencyMapper::isMapped(casa::uInt chan)  const
{
  ASKAPDEBUGASSERT(chan<itsMap.size());
  return itsMap[chan]>=0;
}

   
/// @brief map accessor channel to image channel
/// @details
/// @param[in] chan accessor channel
/// @note the output is guaranteed to be from [0,itsImageNChan-1] interval.
casa::uInt FrequencyMapper::operator()(casa::uInt chan) const
{
  ASKAPDEBUGASSERT(chan<itsMap.size());
  ASKAPDEBUGASSERT(itsImageNChan>0);
  ASKAPCHECK(itsMap[chan]>=0, "An attempt to call FrequencyMapper::operator() for unmapped channel "<<chan);
  return itsMap[chan];
}


