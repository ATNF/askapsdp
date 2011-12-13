/// @file 
///
/// @brief Actual MS writer class doing the low-level dirty job
/// @details This class is heavily based on Ben's MSSink in the CP/ingestpipeline package.
/// I just copied the appropriate code from there. The basic approach is to set up as
/// much of the metadata as we can via the parset file. It is envisaged that we may
/// use this class also for the conversion of the DiFX output into MS. 
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


#ifndef ASKAP_SWCORRELATOR_FILLER_MS_SINK
#define ASKAP_SWCORRELATOR_FILLER_MS_SINK

// own includes
#include <swcorrelator/CorrProducts.h>
#include <swcorrelator/ISink.h>

// casa includes
#include "ms/MeasurementSets/MeasurementSet.h"
#include <casa/BasicSL.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Quanta.h>
#include <measures/Measures/Stokes.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>


// other 3rd party
#include <Common/ParameterSet.h>

// boost includes
#include "boost/scoped_ptr.hpp"
#include <boost/utility.hpp>

// std includes
#include <string>

namespace askap {

namespace swcorrelator {

/// @brief Actual MS writer class doing the low-level dirty job
/// @details This class is heavily based on Ben's MSSink in the CP/ingestpipeline package.
/// I just copied the appropriate code from there. The basic approach is to set up as
/// much of the metadata as we can via the parset file. It is envisaged that we may
/// use this class also for the conversion of the DiFX output into MS. 
/// @ingroup swcorrelator
class FillerMSSink : public ISink {
public:
  /// @brief constructor, sets up  MS writer
  /// @details Configuration is done via the parset, a lot of the metadata are just filled
  /// via the parset.
  /// @param[in] parset parset file with configuration info
  FillerMSSink(const LOFAR::ParameterSet &parset);

  /// @brief calculate uvw for the given buffer
  /// @param[in] buf products buffer
  /// @note The calculation is bypassed if itsUVWValid flag is already set in the buffer
  /// @return time epoch corresponding to the BAT of the buffer
  virtual casa::MEpoch calculateUVW(CorrProducts &buf) const;
  
  /// @brief write one buffer to the measurement set
  /// @details Current fieldID and dataDescID are assumed
  /// @param[in] buf products buffer
  /// @note This method could've received a const reference to the buffer. However, more
  /// workarounds would be required with casa arrays, so we don't bother doing this at the moment.
  /// In addition, we could call calculateUVW inside this method (but we still need an option to
  /// calculate uvw's ahead of writing the buffer if we implement some form of delay tracking).
  virtual void write(CorrProducts &buf) const;
  
  
  /// @brief obtain the number of channels in the current setup
  /// @details This method throws an exception if the number of channels has not been
  /// set up (normally it takes place when MS is initialised)
  /// @return the number of channels in the active configuration
  int nChan() const;
  
  /// @brief obtain number of defined data descriptors
  /// @return number of data descriptors
  int numDataDescIDs() const;
  
  /// @brief set new default data descriptor
  /// @details This will be used for all future write operations
  /// @param[in] desc new data descriptor
  void setDataDescID(const int desc);  
  
protected:
  /// @brief helper method to make a string out of an integer
  /// @param[in] in unsigned integer number
  /// @return a string padded with zero on the left size, if necessary
  static std::string makeString(const casa::uInt in);
    
  /// @brief Initialises ANTENNA and FEED tables
  /// @details This method extracts configuration from the parset and fills in the 
  /// compulsory ANTENNA and FEED tables. It also caches antenna positions and beam offsets 
  /// in the form suitable for calculation of uvw's.
  void initAntennasAndBeams();
  
  
  /// @brief read beam information, populate itsBeamOffsets
  void readBeamInfo();
  
  /// @brief initialises field information
  void initFields();
  
  /// @brief initialises spectral and polarisation info (data descriptor)
  void initDataDesc();
    
  /// @brief Create the measurement set
  void create();

  // methods borrowed from Ben's MSSink class (see CP/ingest)

  // Add observation table row
  casa::Int addObs(const casa::String& telescope,
             const casa::String& observer,
             const double obsStartTime,
             const double obsEndTime);

  // Add field table row
  casa::Int addField(const casa::String& fieldName,
             const casa::MDirection& fieldDirection,
             const casa::String& calCode);

  // Add feeds table rows
  void addFeeds(const casa::Int antennaID,
             const casa::Vector<double>& x,
             const casa::Vector<double>& y,
             const casa::Vector<casa::String>& polType);
  
  // Add antenna table row
  casa::Int addAntenna(const casa::String& station,
             const casa::Vector<double>& antXYZ,
             const casa::String& name,
             const casa::String& mount,
             const casa::Double& dishDiameter);

  // Add data description table row
  casa::Int addDataDesc(const casa::Int spwId, const casa::Int polId);
  
  // Add spectral window table row
  casa::Int addSpectralWindow(const casa::String& name,
            const int nChan,
            const casa::Quantity& startFreq,
            const casa::Quantity& freqInc);

  // Add polarisation table row
  casa::Int addPolarisation(const casa::Vector<casa::Stokes::StokesTypes>& stokesTypes);
  
private:
  /// @brief parameters
  LOFAR::ParameterSet itsParset;
  
  /// @brief data descriptor ID used for all added rows
  casa::uInt itsDataDescID;
  
  /// @brief field ID used for all added rows
  casa::uInt itsFieldID;
  
  /// @brief dish pointing centre corresponding to itsFieldID
  casa::MDirection itsDishPointing;
  
  /// @brief true if uvw's are calculated for the centre of each beam (default)
  bool itsBeamOffsetUVW;
  
  /// @brief global (ITRF) coordinates of all antennas
  /// @details row is antenna number, column is X,Y and Z
  casa::Matrix<double> itsAntXYZ;
  
  /// @brief beam offsets in radians
  /// @details assumed the same for all antennas, row is antenna numbers, column is the coordinate
  casa::Matrix<double> itsBeamOffsets;
  
  /// @brief Measurement set
  boost::scoped_ptr<casa::MeasurementSet> itsMs;
  
  /// @brief antenna indicies for all 3 baselines in our standard order
  static const int theirAntIDs[3][2];
  
  /// @brief cached number of channels
  /// @details We don't use it for the real-time correlator, but it is handy for the converter
  int itsNumberOfChannels;
  
  /// @brief number of data descriptor IDs
  /// @details It is equivalent to the number of rows in the appropriate table
  /// (indices go from 0 to itsNumberOfDataDesc)
  int itsNumberOfDataDesc;
};

} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_FILLER_MS_SINK


