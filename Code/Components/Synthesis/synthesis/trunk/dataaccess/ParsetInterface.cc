/// @file
/// @brief A method to set up converters and selectors from parset file
/// @details Parameters are currently passed around using parset files.
/// The methods declared in this file set up converters and selectors
/// from the ParameterSet object. This is probably a temporary solution.
/// This code can eventually become a part of some class (e.g. a DataSource
/// which returns selectors and converters with the defaults alread
/// applied according to the parset file).
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
///

#include <dataaccess/ParsetInterface.h>
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, "");

#include <askap/AskapError.h>
#include <dataaccess/DataAccessError.h>

#include <iostream>

// @brief set selections according to the given parset object
// @details
// @param[in] sel a shared pointer to the converter to be updated
// @param[in] parset a parset object to read the parameters from
void askap::synthesis::operator<<(const boost::shared_ptr<IDataSelector> &sel,
                 const LOFAR::ParameterSet &parset)
{
  ASKAPDEBUGASSERT(sel);
  if (parset.isDefined("Feed")) {
      sel->chooseFeed(static_cast<casa::uInt>(parset.getUint32("Feed")));
  }
  if (parset.isDefined("Baseline")) {
      std::vector<LOFAR::uint32> baseline = parset.getUint32Vector("Baseline");
      if (baseline.size() != 2) {
          ASKAPTHROW(DataAccessError,"The 'Baseline' parameter in the Parset should have "
	          "exactly 2 elements, the current parameter has "<<baseline.size()<<" elements");
      }
      sel->chooseBaseline(static_cast<casa::uInt>(baseline[0]),
                          static_cast<casa::uInt>(baseline[1]));  
  }
  if (parset.isDefined("Channels")) {
      std::vector<LOFAR::uint32> chans = parset.getUint32Vector("Channels");
      if ((chans.size() != 2) && (chans.size() != 3)) {
          ASKAPTHROW(DataAccessError,"The 'Channels' parameter in the Parset should have "
	    "2 or 3 elements, the current parameter has "<<chans.size()<<" elements");
      }
      sel->chooseChannels(static_cast<casa::uInt>(chans[0]),
                          static_cast<casa::uInt>(chans[1]),
	     chans.size() == 3 ? static_cast<casa::uInt>(chans[2]) : 1);
  }
  if (parset.isDefined("SpectralWindow")) {
      sel->chooseSpectralWindow(static_cast<casa::uInt>(
                    parset.getUint32("SpectralWindow")));
  }
  if (parset.isDefined("Polarizations")) {
      sel->choosePolarizations(parset.getString("Polarizations"));
  }
  if (parset.isDefined("Cycles")) {
      std::vector<LOFAR::uint32> cycles = parset.getUint32Vector("Cycles");
      if (cycles.size() != 2) {
          ASKAPTHROW(DataAccessError,"The 'Cycles' parameter in the Parset should have "
	          "exactly 2 elements, the current parameter has "<<cycles.size()<<" elements");
      }
      sel->chooseCycles(static_cast<casa::uInt>(cycles[0]),
                        static_cast<casa::uInt>(cycles[1]));  
  }
  if (parset.isDefined("TimeRange")) {
      std::vector<double> timeRange = parset.getDoubleVector("TimeRange");
      if (timeRange.size() != 2) {
          ASKAPTHROW(DataAccessError,"The 'TimeRange' parameter in the Parset should have "
	          "exactly 2 elements, the current parameter has "<<timeRange.size()<<" elements");
      }
      sel->chooseTimeRange(static_cast<casa::Double>(timeRange[0]),
                        static_cast<casa::Double>(timeRange[1]));  
  } 
  if (parset.isDefined("CorrelationType")) {
      std::string corrType=parset.getString("CorrelationType");
      if (corrType == "auto") {
          sel->chooseAutoCorrelations();
      } else if (corrType == "cross") {
          sel->chooseCrossCorrelations();
      } else if (corrType != "all") {
          ASKAPTHROW(DataAccessError, "CorrelationType can either be cross, auto or all (default)");
      }
  }
  if (parset.isDefined("MinUV")) {
      sel->chooseMinUVDistance(parset.getDouble("MinUV"));
  }
  if (parset.isDefined("MaxUV")) {
      sel->chooseMaxUVDistance(parset.getDouble("MaxUV"));
  }
}
