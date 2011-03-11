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


// own includes
#include <dataaccess/FeedSubtableHandler.h>
#include <askap_accessors.h>

#include <askap/AskapError.h>
#include <dataaccess/DataAccessError.h>

// casa includes
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <casa/Arrays/Array.h>
#include <casa/BasicSL/String.h>

// enable logger here, when it is used in the code
//#include <askap/AskapLogging.h>
//ASKAP_LOGGER(logger, "");

using namespace askap;
using namespace askap::accessors;

/// @brief construct the object
/// @details
/// @param[in] ms a table object, which has a feed subtable (main MS table)
/// @note a flag showing the first access to the data similar to itsNeverAccessedFlag
/// in FieldSubtableHandler is not required here because itsCachedSpWindow 
/// initial value of -2 serves as such flag.
FeedSubtableHandler::FeedSubtableHandler(const casa::Table &ms) :
          TableHolder(ms.keywordSet().asTable("FEED")),
          itsCachedSpWindow(-2),
          itsCachedStartTime(0.), itsCachedStopTime(0.),
          itsAllCachedOffsetsZero(false), itsIntervalFactor(1.)
{ 
  const casa::Array<casa::String> &intervalUnits=table().tableDesc().
          columnDesc("INTERVAL").keywordSet().asArrayString("QuantumUnits");
  if (intervalUnits.nelements()!=1 || intervalUnits.ndim()!=1) {
      ASKAPTHROW(DataAccessError, "Unable to interpret the QuantumUnits keyword for "
                  "the INTERVAL column of the FEED subtable. It should be a 1D Array of "
                  "exactly 1 String element and the table has "<<intervalUnits.nelements()<<
                  " elements and "<<intervalUnits.ndim()<<" dimensions");
  }
  itsIntervalFactor = tableTime(1.).getValue().
            getTime(casa::Unit(intervalUnits(casa::IPosition(1,0)))).getValue();
  ASKAPDEBUGASSERT(itsIntervalFactor != 0);
  itsIntervalFactor = 1./itsIntervalFactor;
}
 
/// obtain the offsets of each beam with respect to dish pointing
/// centre.
/// @param[in] time a full epoch of interest (feed table can be time-
/// dependent
/// @param[in] spWinID spectral window ID of interest (feed table can be
/// spectral window-dependent
/// @param[in] antID antenna of interest
/// @param[in] feedID feed of interest
/// @return a reference to RigidVector<Double,2> with the offsets on each
/// axes (in radians).
const casa::RigidVector<casa::Double, 2>& 
        FeedSubtableHandler::getBeamOffset(const casa::MEpoch &time, 
                      casa::uInt spWinID,
                      casa::uInt antID, casa::uInt feedID) const
{
  fillCacheOnDemand(time,spWinID);
  const casa::uInt index=getIndex(antID,feedID);
  ASKAPDEBUGASSERT(index<=itsBeamOffsets.nelements());
  return itsBeamOffsets[index];
}    

/// obtain the offsets for all beams with respect to dish pointing
/// centre.
/// @param[in] time a full epoch of interest (feed table can be time-
/// dependent
/// @param[in] spWinID spectral window ID of interest (feed table can be
/// spectral window-dependent
/// @return a reference to a vector with offsets (in radians on each axis)
const casa::Vector<casa::RigidVector<casa::Double, 2> > &
        FeedSubtableHandler::getAllBeamOffsets(const casa::MEpoch &time, 
                                  casa::uInt spWinID) const
{
 fillCacheOnDemand(time,spWinID);
 return itsBeamOffsets;
}

/// obtain position angles for all beams in the current cache (w.r.t. some
/// coordinate system fixed with the dish). The correspondence between 
/// indices in the 1D cache and  antenna/feed pair can be obtained via
/// the getIndex method
/// @param[in] time a full epoch of interest (feed table can be time-
/// dependent
/// @param[in] spWinID spectral window ID of interest (feed table can be
/// spectral window-dependent
const casa::Vector<casa::Double>& FeedSubtableHandler::getAllBeamPAs(
                                 const casa::MEpoch &time, 
                                 casa::uInt spWinID) const
{
  fillCacheOnDemand(time,spWinID);
  return itsPositionAngles;
}

/// obtain an index of the given feed/antenna pair via the look-up table
/// the method throws exceptions if antenna or feed is out of range or
/// the appropriate record is not defined in the FEED subtable (i.e. absent
/// in cache).
/// @param[in] antID antenna of interest
/// @param[in] feedID feed of interest 
casa::uInt FeedSubtableHandler::getIndex(casa::uInt antID, casa::uInt feedID) const
{
 if (antID>=itsIndices.nrow()) {
      ASKAPTHROW(DataAccessError, "Antenna ID requested ("<<antID<<
          ") is outside the range of the FEED table (max. antenna number is "<<
          itsIndices.nrow());
  }
  if (feedID>=itsIndices.ncolumn()) {
      ASKAPTHROW(DataAccessError, "Feed ID requested ("<<feedID<<
          ") is outside the range of the FEED table (max. antenna number is "<<
          itsIndices.ncolumn());
  }
  const casa::Int index=itsIndices(antID,feedID);
  if (index<0) {
      ASKAPTHROW(DataAccessError, "Requested Antenna ID="<<antID<<
           " and Feed ID="<<feedID<<" are not found in the FEED subtable for "
           "the time range from "<<itsCachedStartTime<<" till "<<itsCachedStopTime<<
           " and spectral window "<<itsCachedSpWindow);             
  } 
  return static_cast<casa::uInt>(index);
} 

/// @brief check whether the given time and spectral window ID is  in cache.
/// @details The users of this class are expected to do some heavy postprocessing
/// based on the position angle and beam offsets returned. It is, therefore,
/// very important to know whether they're still the same or not.
/// The cache contains the data for all antennae and feeds.
/// @param[in] time a full epoch of interest (feed table can be time-
/// dependent
/// @param[in] spWinID spectral window ID of interest (feed table can be
/// spectral window-dependent
/// @return true if the beam parameters differ for the given time and
/// spectral window ID
bool FeedSubtableHandler::newBeamDetails(const casa::MEpoch &time, 
                                    casa::uInt spWinID) const
{
  const casa::Double dTime=tableTime(time);
  if (dTime>=itsCachedStartTime && dTime<=itsCachedStopTime &&
      (casa::Int(spWinID)==itsCachedSpWindow || itsCachedSpWindow==-1)) {
      // cache is valid
      return false;
  } 
  return true;
}                                    


/// read the data to fill the cache, a call to isCacheValid allows to check
/// whether reading is necessary
/// @param[in] time a full epoch of interest (feed table can be time-
/// dependent
/// @param[in] spWinID spectral window ID of interest (feed table can be
/// spectral window-dependent  
void FeedSubtableHandler::fillCache(const casa::MEpoch &time, 
                       casa::uInt spWinID) const
{
  // if we really need to optimize the performance, we can cache dTime
  const casa::Double dTime=tableTime(time);
  const casa::TableExprNode halfInterval = table().col("INTERVAL")*itsIntervalFactor/2.;

  // (temporary) work around for zero interval (happens for ATCA data)
  // probably an appropriate filler has to be fixed as it doesn't
  // seem to conform with the measurement set standard

  const casa::TableExprNode expression = ((table().col("SPECTRAL_WINDOW_ID") ==
                static_cast<casa::Int>(spWinID)) || 
                    (table().col("SPECTRAL_WINDOW_ID") == -1)) &&
               (((table().col("TIME") - halfInterval <= dTime) &&
                (table().col("TIME") + halfInterval >= dTime)) ||
                (halfInterval == 0.));
  casa::Table selection=table()(expression);
  if (selection.nrow()==0) {
      ASKAPTHROW(DataAccessError,
                 "FEED subtable is empty or feed data missing for "
                  <<time<<" and spectral window: "<<spWinID);
  }
  itsBeamOffsets.resize(selection.nrow());
  itsPositionAngles.resize(selection.nrow());
  casa::ROScalarColumn<casa::Int> antIDs(selection,"ANTENNA_ID");
  antIDs.getColumn(itsAntennaIDs,casa::True);
  casa::Int minAntID=-1,maxAntID=-1;
  casa::minMax(minAntID,maxAntID,itsAntennaIDs);
  casa::ROScalarColumn<casa::Int> feedIDs(selection,"FEED_ID");
  feedIDs.getColumn(itsFeedIDs,casa::True);
  casa::Int minFeedID=-1,maxFeedID=-1;
  casa::minMax(minFeedID,maxFeedID,itsFeedIDs);
  if (minAntID<0 || maxAntID<0 || minFeedID<0 || maxFeedID<0) {
      ASKAPTHROW(DataAccessError,"Negative indices in FEED_ID and ANTENNA_ID "
         "columns of the FEED subtable are not allowed");
  }
  ++maxAntID; ++maxFeedID; // now we have numbers of feeds and antennae
  ASKAPDEBUGASSERT(maxAntID*maxFeedID == casa::Int(selection.nrow()));
  itsIndices.resize(maxAntID,maxFeedID);
  itsIndices.set(-2); // negative value is a flag, which means an 
                      // uninitialized index
  casa::ROArrayColumn<casa::Double>  rcptrOffsets(selection,"BEAM_OFFSET");
  casa::ROArrayColumn<casa::Double>  rcptrPAs(selection,"RECEPTOR_ANGLE");
  // update start and stop times as well as the spectral window ID 
  // used in the cache management within the same loop
  casa::ROScalarColumn<casa::Double> timeCol(selection,"TIME");
  casa::ROScalarColumn<casa::Double> intervalCol(selection,"INTERVAL");
  casa::ROScalarColumn<casa::Int> spWinCol(selection,"SPECTRAL_WINDOW_ID");
  itsCachedSpWindow = spWinCol(0);
  // we will set this flag to false later, if a non-zero offset is found
  itsAllCachedOffsetsZero = true; 
  for (casa::uInt row=0; row<selection.nrow(); ++row) {
       casa::RigidVector<casa::Double, 2> &cOffset = itsBeamOffsets[row];
       computeBeamOffset(rcptrOffsets(row),cOffset);
       if ((std::abs(cOffset(0)) > 1e-15) || (std::abs(cOffset(1)) > 1e-15)) {
           itsAllCachedOffsetsZero = false;
           //std::cerr<<"non zero offset "<<cOffset(0)<<" "<<cOffset(1)<<std::endl; 
       }
       itsPositionAngles[row]=computePositionAngle(rcptrPAs(row));
       itsIndices(antIDs(row),feedIDs(row))=row;
       
       casa::Double cStartTime = timeCol(row) -
                          intervalCol(row) * itsIntervalFactor/2.;
       casa::Double cStopTime = timeCol(row) +
                          intervalCol(row) * itsIntervalFactor/2.;
       // (temporary) work around for zero interval (happens for ATCA data)
       // probably an appropriate filler has to be fixed as it doesn't
       // seem to conform with the measurement set standard
       if (intervalCol(row) == 0.) {
           // not a very clean way, but
	       // we need large offsets here
           cStartTime = timeCol(row) - 1e30;
	       cStopTime = timeCol(row) + 1e30; 
       }
       if (!row || itsCachedStartTime<cStartTime) {
           itsCachedStartTime=cStartTime;	   
       }
       if (!row || itsCachedStopTime>cStopTime) {
           itsCachedStopTime=cStopTime;
       }
       if (spWinCol(row) != -1) {
           ASKAPDEBUGASSERT((itsCachedSpWindow == -1) || 
                             (spWinCol(row) == itsCachedSpWindow));
           itsCachedSpWindow = spWinCol(row);
       }
  }
            
}
                       

/// compute beam offset (squint is taken into acccount by
/// the voltage pattern model). At this stage we just average over all
/// receptors
/// @param[in] rcptOffsets offsets for all receptors corresponding to the 
/// given feed
/// @param[out] beamOffsets returned averaged offsets
void FeedSubtableHandler::computeBeamOffset(const casa::Array<casa::Double> &rcptOffsets,
                      casa::RigidVector<casa::Double, 2> &beamOffsets)
{
  ASKAPASSERT(rcptOffsets.ndim()<3);
  if (rcptOffsets.ndim()==1) {
      // this means that have just one receptor and nothing, but copying
      // of values is required
      ASKAPASSERT(rcptOffsets.nelements()==2);
      beamOffsets(0)=rcptOffsets(casa::IPosition(1,0));
      beamOffsets(1)=rcptOffsets(casa::IPosition(1,1));
  } else {
      const casa::IPosition &shape=rcptOffsets.shape();
      ASKAPASSERT(shape[0] == 2);
      ASKAPASSERT(shape[1] > 0);
      beamOffsets(0)=beamOffsets(1)=0.;
      const casa::uInt nReceptors=shape[1];
      for (casa::uInt rcpt=0; rcpt<nReceptors; ++rcpt) {
           beamOffsets(0)+=rcptOffsets(casa::IPosition(2,0,rcpt))/
                       static_cast<casa::Double>(nReceptors);
           beamOffsets(1)+=rcptOffsets(casa::IPosition(2,1,rcpt))/
                       static_cast<casa::Double>(nReceptors);           
      }
  }  
}                      
  
/// compute beam position angle. At this stage we just take the
/// angle corresponding to the first receptor.
/// @param[in] rcptAngles angles for all receptors corresponding to the given 
/// feed
/// @return the angle corresponding to the beam (curretly that of the first 
/// receptor) 
casa::Double FeedSubtableHandler::computePositionAngle(const casa::Array<casa::Double>
                               &rcptAngles)
{
  ASKAPDEBUGASSERT(rcptAngles.ndim()==1);
  ASKAPASSERT(rcptAngles.nelements()>=1);
  return rcptAngles(casa::IPosition(1,0));
}                               
                  
  
/// obtain the position angle of each beam (w.r.t. some coordinate system
/// fixed with the dish).
/// @param[in] time a full epoch to of interest (feed table can be time-
/// dependent
/// @param[in] spWinID spectral window ID of interest (feed table can be
/// spectral window-dependent
/// @param[in] antID antenna of interest
/// @param[in] feedID feed of interest 
/// @return a position angle (in radians).
casa::Double FeedSubtableHandler::getBeamPA(const casa::MEpoch &time, 
                                 casa::uInt spWinID, 
                                 casa::uInt antID, casa::uInt feedID) const
{
  fillCacheOnDemand(time,spWinID);
  const casa::uInt index=getIndex(antID,feedID);
  ASKAPDEBUGASSERT(index<=itsPositionAngles.nelements());
  return itsPositionAngles[index];
}                                 
  
/// the same as fillCache, but perform it if newBeamDetails returns true
/// @param[in] time a full epoch of interest (feed table can be time-
/// dependent
/// @param[in] spWinID spectral window ID of interest (feed table can be
/// spectral window-dependent  
void FeedSubtableHandler::fillCacheOnDemand(const casa::MEpoch &time, 
                                            casa::uInt spWinID) const
{
  ASKAPDEBUGASSERT(spWinID>=0);
  if (newBeamDetails(time,spWinID)) {
      fillCache(time,spWinID);
  }
}                                            

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
bool FeedSubtableHandler::allBeamOffsetsZero(const casa::MEpoch &time, casa::uInt spWinID) const
{
  fillCacheOnDemand(time,spWinID);
  return itsAllCachedOffsetsZero;
}

/// obtain feed IDs for the given time and spectral window
/// @param[in] time a full epoch of interest (feed table can be time-
/// dependent
/// @param[in] spWinID spectral window ID of interest (feed table can be
/// spectral window-dependent
/// @return a vector of feed IDs, each element corresponds to the appropriate
/// element of getAllBeamPAs and getAllBeamOffsets
const casa::Vector<casa::Int>& FeedSubtableHandler::getFeedIDs(const casa::MEpoch &time, 
                      casa::uInt spWinID) const
{
  fillCacheOnDemand(time,spWinID);
  return itsFeedIDs;
}                      
  
/// obtain antenna IDs for the given time and spectral window
/// @param[in] time a full epoch of interest (feed table can be time-
/// dependent
/// @param[in] spWinID spectral window ID of interest (feed table can be
/// spectral window-dependent
/// @return a vector of antenna IDs, each element corresponds to the appropriate
/// element of getAllBeamPAs and getAllBeamOffsets
const casa::Vector<casa::Int>& FeedSubtableHandler::getAntennaIDs(const casa::MEpoch &time, 
                      casa::uInt spWinID) const
{
  fillCacheOnDemand(time,spWinID);
  return itsAntennaIDs;  
}  

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
const casa::Matrix<casa::Int>& FeedSubtableHandler::getIndices() const throw()
{
  return itsIndices;
}
