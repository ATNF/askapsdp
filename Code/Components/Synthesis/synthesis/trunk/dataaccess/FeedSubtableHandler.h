/// @file
/// @brief A class to access FEED subtable
/// @details This file contains a class implementing IFeedSubtableHandler interface to
/// the content of the FEED subtable (which provides offsets of each physical
/// feed from the dish pointing centre and its position anlge). Although this 
/// implementation caches the values for the last requested time-range and 
/// the spectral window, it reads the data on-demand. This is the difference
/// from some other subtables which are implemented by Mem... classes reading
/// all the required data in the constructor. If the table is trivial 
/// (no time- and spectral window dependence), it will be fully cached on the
/// first request.
/// @note The measurement set format specifies offsets for each receptor,
/// rather than feed (i.e. for each polarization separately). We handle possible
/// squints together with other image plane effects and therefore need just
/// a reference position (i.e. an average offset if there is any squint). 
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef FEED_SUBTABLE_HANDLER_H
#define FEED_SUBTABLE_HANDLER_H

// casa includes
#include <tables/Tables/Table.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Quanta/Quantum.h>



// own includes
#include <dataaccess/IFeedSubtableHandler.h>
#include <dataaccess/TableHolder.h>
#include <dataaccess/TimeDependentSubtable.h>

namespace askap {

namespace synthesis {

/// @brief A class to access FEED subtable
/// @details This file contains a class implementing IFeedSubtableHandler interface to
/// the content of the FEED subtable (which provides offsets of each physical
/// feed from the dish pointing centre and its position anlge). Although this 
/// implementation caches the values for the last requested time-range and 
/// the spectral window, it reads the data on-demand. This is the difference
/// from some other subtables which are implemented by Mem... classes reading
/// all the required data in the constructor. If the table is trivial 
/// (no time- and spectral window dependence), it will be fully cached on the
/// first request.
/// @note The measurement set format specifies offsets for each receptor,
/// rather than feed (i.e. for each polarization separately). We handle possible
/// squints together with other image plane effects and therefore need just
/// a reference position (i.e. an average offset if there is any squint). 
/// @ingroup dataaccess_tab
struct FeedSubtableHandler : virtual public IFeedSubtableHandler,
                             virtual protected TableHolder,
                             virtual protected TimeDependentSubtable   {
  
  
  /// @brief construct the object
  /// @details
  /// @param[in] ms a table object, which has a feed subtable (main MS table)
  explicit FeedSubtableHandler(const casa::Table &ms);
  

  /// obtain the offsets of a given beam with respect to dish pointing
  /// centre.
  /// @param[in] time a full epoch of interest (feed table can be time-
  /// dependent
  /// @param[in] spWinID spectral window ID of interest (feed table can be
  /// spectral window-dependent
  /// @param[in] antID antenna of interest
  /// @param[in] feedID feed of interest
  /// @return a reference to RigidVector<Double,2> with the offsets on each
  /// axes (in radians).
  virtual const casa::RigidVector<casa::Double, 2>& 
        getBeamOffset(const casa::MEpoch &time, casa::uInt spWinID,
                      casa::uInt antID, casa::uInt feedID) const;
  
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
                                 casa::uInt antID, casa::uInt feedID) const;

  /// obtain position angles for all beams in the current cache (w.r.t. some
  /// coordinate system fixed with the dish). The correspondence between 
  /// indices in the 1D cache and  antenna/feed pair can be obtained via
  /// the getFeedIDs and getAntennaIDs methods
  /// @param[in] time a full epoch of interest (feed table can be time-
  /// dependent
  /// @param[in] spWinID spectral window ID of interest (feed table can be
  /// spectral window-dependent
  /// @return a reference to a vector with angles (in radians)
  virtual const casa::Vector<casa::Double>& getAllBeamPAs(
                        const casa::MEpoch &time, casa::uInt spWinID) const;

  /// obtain the offsets for all beams with respect to dish pointing
  /// centre.
  /// @param[in] time a full epoch of interest (feed table can be time-
  /// dependent
  /// @param[in] spWinID spectral window ID of interest (feed table can be
  /// spectral window-dependent
  /// @return a reference to a vector with offsets (in radians on each axis)
  virtual const casa::Vector<casa::RigidVector<casa::Double, 2> > &
           getAllBeamOffsets(const casa::MEpoch &time, casa::uInt spWinID) const;

  /// obtain feed IDs for the given time and spectral window
  /// @param[in] time a full epoch of interest (feed table can be time-
  /// dependent
  /// @param[in] spWinID spectral window ID of interest (feed table can be
  /// spectral window-dependent
  /// @return a vector of feed IDs, each element corresponds to the appropriate
  /// element of getAllBeamPAs and getAllBeamOffsets
  virtual const casa::Vector<casa::Int>& getFeedIDs(const casa::MEpoch &time, 
                      casa::uInt spWinID) const;
  
  /// obtain antenna IDs for the given time and spectral window
  /// @param[in] time a full epoch of interest (feed table can be time-
  /// dependent
  /// @param[in] spWinID spectral window ID of interest (feed table can be
  /// spectral window-dependent
  /// @return a vector of antenna IDs, each element corresponds to the appropriate
  /// element of getAllBeamPAs and getAllBeamOffsets
  virtual const casa::Vector<casa::Int>& getAntennaIDs(const casa::MEpoch &time, 
                      casa::uInt spWinID) const;
  
  
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
  virtual const casa::Matrix<casa::Int>& getIndices() const throw();
  
  /// @brief check whether the given time and spectral window ID is  in cache.
  /// @details The users of this class are expected to do some heavy postprocessing
  /// based on the position angle and beam offsets returned. It is, therefore,
  /// very important to know whether they're still the same or not.
  /// The cache contains the data for all antennae and feeds.
  /// @param[in] time a full epoch of interest (feed table can be time-
  /// dependent
  /// @param[in] spWinID spectral window ID of interest (feed table can be
  /// spectral window-dependent
  /// @return true if the beam parameters didn't change for the given time and
  /// spectral window ID
  virtual bool newBeamDetails(const casa::MEpoch &time, casa::uInt spWinID) const;
 
protected:
  /// read the data to fill the cache, a call to isCacheValid allows to check
  /// whether reading is necessary
  /// @param[in] time a full epoch of interest (feed table can be time-
  /// dependent
  /// @param[in] spWinID spectral window ID of interest (feed table can be
  /// spectral window-dependent  
  void fillCache(const casa::MEpoch &time, casa::uInt spWinID) const;
  
  /// the same as fillCache, but perform it if newBeamDetails returns true
  /// @param[in] time a full epoch of interest (feed table can be time-
  /// dependent
  /// @param[in] spWinID spectral window ID of interest (feed table can be
  /// spectral window-dependent  
  void fillCacheOnDemand(const casa::MEpoch &time, casa::uInt spWinID) const;
    
  
  /// obtain an index of the given feed/antenna pair via the look-up table
  /// the method throws exceptions if antenna or feed is out of range or
  /// the appropriate record is not defined in the FEED subtable (i.e. absent
  /// in cache).
  /// @param[in] antID antenna of interest
  /// @param[in] feedID feed of interest 
  casa::uInt getIndex(casa::uInt antID, casa::uInt feedID) const; 
  
  /// compute beam offset (squint is taken into acccount by
  /// the voltage pattern model). At this stage we just average over all
  /// receptors
  /// @param[in] rcptOffsets offsets for all receptors corresponding to the 
  /// given feed
  /// @param[out] beamOffsets returned averaged offsets
  static void computeBeamOffset(const casa::Array<casa::Double> &rcptOffsets,
                      casa::RigidVector<casa::Double, 2> &beamOffsets);
  
  /// compute beam position angle. At this stage we just take the
  /// angle corresponding to the first receptor.
  /// @param[in] rcptAngles angles for all receptors corresponding to the given 
  /// feed
  /// @return the angle corresponding to the beam (curretly that of the first 
  /// receptor) 
  static casa::Double computePositionAngle(const casa::Array<casa::Double>
                               &rcptAngles);             
private:
 
  /// the spectral window for which the cache is valid. -1 means for any
  /// spectral window (if the table is spectral window-independent). 
  mutable casa::Int itsCachedSpWindow;
  
  /// start time of the time ranges for which the cache is valid. 
  /// Time-independed table has a very wide time range. The time is stored 
  /// as Double in the native frame/units of the FEED table. 
  mutable casa::Double itsCachedStartTime;
  
  /// stop time of the time range for which the cache is valid. 
  /// See itsCachedStartTimes for more details.
  mutable casa::Double itsCachedStopTime;
  
  /// a cache of beam offsets
  mutable casa::Vector<casa::RigidVector<casa::Double, 2> > itsBeamOffsets;
  
  /// a cache of position angles
  mutable casa::Vector<casa::Double> itsPositionAngles;
  
  /// a look-up table to convert (ant,feed) into an index for all caches
  /// (1D-vectors). We need this look-up table as, in principle, the fields
  /// can be out of order in the FEED subtable, missing or repeated. A simple
  /// sort, as it was done for the VisBuffer in AIPS++ is not sufficient
  /// in the general case.
  mutable casa::Matrix<casa::Int> itsIndices;
  
  /// a factor to multiply the INTERVAL to get the same units as
  /// TIME column
  casa::Double itsIntervalFactor;
  
  /// a cache of antenna IDs
  mutable casa::Vector<casa::Int> itsAntennaIDs;
  
  /// a cache of feed IDs
  mutable casa::Vector<casa::Int> itsFeedIDs;
};


} // namespace synthesis

} // namespace askap

#endif // #ifndef FEED_SUBTABLE_HANDLER_H
