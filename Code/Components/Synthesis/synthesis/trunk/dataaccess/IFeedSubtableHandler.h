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
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_FEED_SUBTABLE_HANDLER_H
#define I_FEED_SUBTABLE_HANDLER_H

// casa includes
#include <measures/Measures/MEpoch.h>
#include <scimath/Mathematics/RigidVector.h>

// own includes
#include <dataaccess/IHolder.h>

namespace conrad {

namespace synthesis {

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

};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef I_FEED_SUBTABLE_HANDLER_H
