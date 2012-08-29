/// @file 
///
/// @brief basic on-the-fly monitor dumping data into an ascii file
/// @details This implementation of the data monitor dumps delay and
/// visibility history into ascii files for on-the-fly monitoring along
/// with the latest spectra for each beam
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

#ifndef ASKAP_SWCORRELATOR_BASIC_MONITOR_H
#define ASKAP_SWCORRELATOR_BASIC_MONITOR_H

// own includes
#include <swcorrelator/IMonitor.h>

// casa includes
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Vector.h>

// other 3rd party
#include <Common/ParameterSet.h>

namespace askap {

namespace swcorrelator {

/// @brief basic on-the-fly monitor dumping data into an ascii file
/// @details This implementation of the data monitor dumps delay and
/// visibility history into ascii files for on-the-fly monitoring along
/// with the latest spectra for each beam
/// @ingroup swcorrelator
class BasicMonitor : public IMonitor {
public:
  /// @brief constructor
  BasicMonitor();

  /// @brief create and configure the monitor   
  /// @details
  /// @param[in] parset parset with parameters (without the swcorrelator prefix)
  /// @return shared pointer to the monitor
  static boost::shared_ptr<BasicMonitor> setup(const LOFAR::ParameterSet &parset);
  
  /// @brief name of the monitor
  /// @return the name of the monitor
  static std::string name() { return "basic"; }
  
  /// @brief initialise publishing
  /// @details Technically, this step is not required. But given the
  /// current design of the code it seems better to give a hint on the maximum
  /// possible number of antennas, beams and channels, e.g. to initialise caches.
  /// @param[in] nAnt maximum number of antennas
  /// @param[in] nBeam maximum number of beams
  /// @param[in] nChan maximum number of channels
  /// @note At the moment we envisage that this method would only be called once.
  /// Technically all this information could be extracted from the parset supplied
  /// in the setup method, but it seems handy to have each parameter extracted from
  /// the parset at a single place only.  
  virtual void initialise(const int nAnt, const int nBeam, const int nChan);
   
  /// @brief Publish one buffer of data
  /// @details This method is called as soon as the new chunk of data is written out
  /// @param[in] buf products buffer
  /// @note the buffer is locked for the duration of execution of this method, different
  /// beams are published separately
  virtual void publish(const CorrProducts &buf);
  
  /// @brief finilaise publishing for the current integration
  /// @details This method is called when data corresponding to all beams are published.
  /// It is the place for operations which do not require the lock on the buffers
  /// (i.e. dumping the accumulated history to the file, etc).
  virtual void finalise();

  /// @brief helper method to get delays
  /// @details
  /// @param[in] vis visibility matrix (rows are baselines, columns are channels)
  /// @return delays in seconds for each baseline
  /// @note the routine assumes 1 MHz channel spacing and will not work for a very quick wrap
  static casa::Vector<casa::Float> estimateDelays(const casa::Matrix<casa::Complex> &vis);

protected:
  /// @brief advance history if necessary
  /// @details Advances the cursor in the history list if the new bat is different from the one 
  /// stored during the previous step (unless it is a first step)
  /// @param[in] bat new BAT  
  void advanceHistoryCursor(const uint64_t bat);
private:
  /// @brief history of visibilities
  casa::Cube<casa::Complex> itsHistory;
  
  /// @brief history of delays
  casa::Cube<casa::Float> itsDelayHistory;
  
  /// @brief BATs for the history items
  casa::Vector<uint64_t> itsBATs;
  
  /// @brief last position in the history (circular buffers)
  /// @details negative value for an uninitialised history
  int itsLastHistPosition;
  
  /// @brief true if the history buffers were wrapped
  bool itsWrapped;
};

} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_BASIC_MONITOR_H

