/// @file
/// @brief A method to set up converters and selectors from parset file
/// @details Parameters are currently passed around using parset files.
/// The methods declared in this file set up converters and selectors
/// from the ParameterSet object. This is probably a temporary solution.
/// This code can eventually become a part of some class (e.g. a DataSource
/// which returns selectors and converters with the defaults alread
/// applied according to the parset file).
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include <dataaccess/ParsetInterface.h>
#include <conrad/ConradError.h>
#include <dataaccess/DataAccessError.h>

#include <iostream>

/// @brief set selections according to the given parset object
/// @details
/// @param[in] sel a shared pointer to the converter to be updated
/// @param[in] parset a parset object to read the parameters from
void conrad::synthesis::operator<<(const boost::shared_ptr<IDataSelector> &sel,
                 const LOFAR::ACC::APS::ParameterSet &parset)
{
  CONRADDEBUGASSERT(sel);
  if (parset.isDefined("Feed")) {
      sel->chooseFeed(static_cast<casa::uInt>(parset.getUint32("Feed")));
  }
  if (parset.isDefined("Baseline")) {
      std::vector<LOFAR::uint32> baseline = parset.getUint32Vector("Baseline");
      if (baseline.size() != 2) {
          CONRADTHROW(DataAccessError,"The 'Baseline' parameter in the Parset should have "
	          "exactly 2 elements, the current parameter has "<<baseline.size()<<" elements");
      }
      sel->chooseBaseline(static_cast<casa::uInt>(baseline[0]),
                          static_cast<casa::uInt>(baseline[1]));  
  }
  if (parset.isDefined("Channels")) {
      std::vector<LOFAR::uint32> chans = parset.getUint32Vector("Channels");
      if ((chans.size() != 2) && (chans.size() != 3)) {
          CONRADTHROW(DataAccessError,"The 'Channels' parameter in the Parset should have "
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
          CONRADTHROW(DataAccessError,"The 'Cycles' parameter in the Parset should have "
	          "exactly 2 elements, the current parameter has "<<cycles.size()<<" elements");
      }
      sel->chooseCycles(static_cast<casa::uInt>(cycles[0]),
                        static_cast<casa::uInt>(cycles[1]));  
  }
  if (parset.isDefined("TimeRange")) {
      std::vector<double> timeRange = parset.getDoubleVector("TimeRange");
      if (timeRange.size() != 2) {
          CONRADTHROW(DataAccessError,"The 'TimeRange' parameter in the Parset should have "
	          "exactly 2 elements, the current parameter has "<<timeRange.size()<<" elements");
      }
      sel->chooseTimeRange(static_cast<casa::Double>(timeRange[0]),
                        static_cast<casa::Double>(timeRange[1]));  
  } 
 
}
