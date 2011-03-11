/// @file
/// @brief An interface to FEED subtable
/// @details A class derived from this interface provides access to
/// the content of the FEED subtable (which provides offsets of each physical
/// feed from the dish pointing centre and its position anlge). 
/// @note The measurement set format specifies offsets for each receptor,
/// rather than feed (i.e. for each polarization separately). We handle possible
/// squints together with other image plane effects and therefore need just
/// a reference position (i.e. an average offset if there is any squint). 
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

#ifndef I_FEED_SUBTABLE_HANDLER_H
#define I_FEED_SUBTABLE_HANDLER_H

// casa includes
#include <measures/Measures/MEpoch.h>
#include <scimath/Mathematics/RigidVector.h>

// own includes
#include <dataaccess/IHolder.h>

namespace askap {

namespace accessors {

/// @brief An interface to FEED subtable
/// @details A class derived from this interface provides access to
/// the content of the FEED subtable (which provides offsets of each physical
/// feed from the dish pointing centre and its position anlge). 
/// @note The measurement set format specifies offsets for each receptor,
/// rather than feed (i.e. for each polarization separately). We handle possible
/// squints together with other image plane effects and therefore need just
/// a reference position (i.e. an average offset if there is any squint). 
/// @ingroup dataaccess_tab
struct IFeedSubtableHandler : virtual public IHolder {

  /// obtain the offsets of each beam with respect to dish pointing
  /// centre.
  /// @param[in] time a full epoch to interest (feed table can be time-
  /// dependent
  /// @param[in] spWinID spectral window ID of interest (feed table can be
  /// spectral window-dependent
  /// @param[in] antID antenna of interest
  /// @param[in] feedID feed of interest
  /// @return a reference to RigidVector<Double,2> with the offsets on each
  /// axes (in radians).
  virtual const casa::RigidVector<casa::Double, 2>& 
        getBeamOffset(const casa::MEpoch &time, casa::uInt spWinID,
                      casa::uInt antID, casa::uInt feedID) const = 0;
  
  /// obtain the position angle of each beam (w.r.t. some coordinate system
  /// fixed with the dish).
  /// @param[in] time a full epoch of interest (feed table can be time-
  /// dependent
  /// @param[in] spWinID spectral window ID of interest (feed table can be
  /// spectral window-dependent
  /// @param[in] antID antenna of interest
  /// @param[in] feedID feed of interest
  /// @return a position angle (in radians).
  virtual casa::Double getBeamPA(const casa::MEpoch &time, 
                                 casa::uInt spWinID,
                                 casa::uInt antID, casa::uInt feedID) const = 0;
  
  /// @brief check whether the given time and spectral window ID is  in cache.
  /// @details The users of this class are expected to do some heavy postprocessing
  /// based on the position angle and beam offsets returned. It is, therefore,
  /// very important to know whether they're still the same or not.
  /// The cache contains the data for all antennae and feeds.
  /// @param[in] time a full epoch of interest (feed table can be time-
  /// dependent
  /// @param[in] spWinID spectral window ID of interest (feed table can be
  /// spectral window-dependent
  /// @return true if the beam parameters are different for the given time and
  /// spectral window ID
  virtual bool newBeamDetails(const casa::MEpoch &time, casa::uInt spWinID) const = 0;

  /// obtain position angles for all beams in the current cache (w.r.t. some
  /// coordinate system fixed with the dish). The correspondence between 
  /// indices in the 1D cache and  antenna/feed pair can be obtained via
  /// the getIndex method
  /// @param[in] time a full epoch of interest (feed table can be time-
  /// dependent
  /// @param[in] spWinID spectral window ID of interest (feed table can be
  /// spectral window-dependent
  /// @return a reference to a vector with angles (in radians)
  virtual const casa::Vector<casa::Double>& getAllBeamPAs(
                        const casa::MEpoch &time, casa::uInt spWinID) const = 0;

  /// obtain the offsets for all beams with respect to dish pointing
  /// centre.
  /// @param[in] time a full epoch of interest (feed table can be time-
  /// dependent
  /// @param[in] spWinID spectral window ID of interest (feed table can be
  /// spectral window-dependent
  /// @return a reference to a vector with offsets (in radians on each axis)
  virtual const casa::Vector<casa::RigidVector<casa::Double, 2> > &
         getAllBeamOffsets(const casa::MEpoch &time, casa::uInt spWinID) const = 0;

  /// obtain feed IDs for the given time and spectral window
  /// @param[in] time a full epoch of interest (feed table can be time-
  /// dependent
  /// @param[in] spWinID spectral window ID of interest (feed table can be
  /// spectral window-dependent
  /// @return a vector of feed IDs, each element corresponds to the appropriate
  /// element of getAllBeamPAs and getAllBeamOffsets
  virtual const casa::Vector<casa::Int>& getFeedIDs(const casa::MEpoch &time, 
                      casa::uInt spWinID) const = 0;
  
  /// obtain antenna IDs for the given time and spectral window
  /// @param[in] time a full epoch of interest (feed table can be time-
  /// dependent
  /// @param[in] spWinID spectral window ID of interest (feed table can be
  /// spectral window-dependent
  /// @return a vector of antenna IDs, each element corresponds to the appropriate
  /// element of getAllBeamPAs and getAllBeamOffsets
  virtual const casa::Vector<casa::Int>& getAntennaIDs(const casa::MEpoch &time, 
                      casa::uInt spWinID) const = 0;

  /// @brief obtain a matrix of indices into beam offset and beam PA arrays
  /// @details getAllBeamOffsets and getAllBeamPAs methods return references
  /// to 1D arrays. This method returns a matrix of nAnt x nFeed indices, which
  /// is required to establish correspondence between the elements of 1D arrays
  /// mentioned above and feed/antenna pairs. Negative values mean that this
  /// feed/antenna pair is undefined.
  /// @note The method returns a valid result after a call to any of the access 
  /// methods (e.g. getAllBeamOffsets). We could have required the time and spWinID
  /// input parameters here to ensure that the cache is up to date as it is done
  /// in all access methods. However, all use cases of this call imply that
  /// the cache is already up to date and passing parameters and doing additional
  /// checks will be a waste of resources. It is probably better to live with the
  /// current interface although this approach is less elegant.
  /// @return a reference to matrix with indicies
  virtual const casa::Matrix<casa::Int>& getIndices() const throw() = 0;

  /// @brief check whether all beam offsets are zero
  /// @details Non-zero beam offsets cause heavy calculations when a pointing
  /// direction is requested for each particular feed. This method allows to
  /// check whether all offsets are zero for the current time and spectral window. 
  /// There is no need to invalidate a cache of pointing directions if we have 
  /// an on-axis feed only. The issue is complicated by the fact that the feed
  /// table could be time- and spectral window-dependent. 
  /// @param[in] time a full epoch of interest (feed table can be time-
  /// dependent
  /// @param[in] spWinID spectral window ID of interest (feed table can be
  /// spectral window-dependent
  /// @return true if all beam offsets are zero for the given time/epoch.
  virtual bool allBeamOffsetsZero(const casa::MEpoch &time, casa::uInt spWinID) const = 0;

};


} // namespace accessors

} // namespace askap

#endif // #ifndef I_FEED_SUBTABLE_HANDLER_H
