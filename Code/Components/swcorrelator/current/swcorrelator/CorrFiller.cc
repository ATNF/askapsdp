/// @file 
///
/// @brief MS filler and the result buffer manager
/// @details This class manages a double buffer for the resulting visibilities and
/// flags. When the BAT timestamp changes, it stores the previous buffer.
/// It is intended to be run in a parallel thread.
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

#include <swcorrelator/CorrFiller.h>
#include <askap/AskapError.h>
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>

ASKAP_LOGGER(logger, ".corrfiller");

namespace askap {

namespace swcorrelator {


/// @brief constructor, sets up the filler
/// @details Configuration is done via the parset
/// @param[in] parset parset file with configuration info
CorrFiller::CorrFiller(const LOFAR::ParameterSet &parset) :
   itsNAnt(parset.getInt32("nant",3)), itsNBeam(parset.getInt32("nbeam",1)),
   itsNChan(parset.getInt32("nchan",1)), itsFlushStatus(false,false)
{
  ASKAPCHECK(itsNAnt == 3, "Only 3 antennas are supported at the moment");
  ASKAPCHECK(itsNChan > 0, "Number of channels should be positive");
  ASKAPCHECK(itsNBeam > 0, "Number of beams should be positive");
  
  ASKAPLOG_INFO_STR(logger, "Initialise filler for "<<nAnt()<<" antennas and up to "<<nBeam()<<" beams and "<<nChan()<<" channels(cards)");
  
  itsCorrProducts.resize(2*nBeam());
  for (int buf=0; buf<int(itsCorrProducts.size()); ++buf) {
       itsCorrProducts[buf].reset(new CorrProducts(nChan(), buf % nBeam()));
  }
  itsFillStatus.resize(nBeam(),false);  
  // initialisation of MS will come here
}

/// @brief shutdown the filler
/// @detais This method is effectively a destructor, which can be
/// called more explicitly. It stops the writing thread and
/// closes the MS which is currently being written.
void CorrFiller::shutdown()
{
  // MS should be flushed to disk and closed here
}

} // namespace swcorrelator

} // namespace askap

