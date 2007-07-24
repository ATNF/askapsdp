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
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///


// own includes
#include <dataaccess/FeedSubtableHandler.h>
#include <conrad/ConradError.h>
#include <dataaccess/DataAccessError.h>

// casa includes
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <casa/Arrays/Array.h>
#include <casa/BasicSL/String.h>


using namespace conrad;
using namespace conrad::synthesis;

/// @brief construct the object
/// @details
/// @param[in] ms a table object, which has a feed subtable (main MS table)
/// @note a flag showing the first access to the data similar to itsNeverAccessedFlag
/// in FieldSubtableHandler is not required here because itsCachedSpWindow 
/// initial value of -2 serves as such flag.
FeedSubtableHandler::FeedSubtableHandler(const casa::Table &ms) :
          TableHolder(ms.keywordSet().asTable("FEED")),
          itsCachedSpWindow(-2), itsIntervalFactor(1.)
{
  const casa::Array<casa::String> &intervalUnits=table().tableDesc().
          columnDesc("INTERVAL").keywordSet().asArrayString("QuantumUnits");
  if (intervalUnits.nelements()!=1 || intervalUnits.ndim()!=1) {
      CONRADTHROW(DataAccessError, "Unable to interpret the QuantumUnits keyword for "
                  "the INTERVAL column of the FEED subtable. It should be a 1D Array of "
                  "exactly 1 String element and the table has "<<intervalUnits.nelements()<<
                  " elements and "<<intervalUnits.ndim()<<" dimensions");
  }
  itsIntervalFactor = tableTime(1.).getValue().
            getTime(casa::Unit(intervalUnits(casa::IPosition(1,0)))).getValue();
  CONRADDEBUGASSERT(itsIntervalFactor != 0);
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
  CONRADDEBUGASSERT(spWinID>=0);
  if (newBeamDetails(time,spWinID)) {
      fillCache(time,spWinID);
  }
  const casa::uInt index=getIndex(antID,feedID);
  CONRADDEBUGASSERT(index<=itsBeamOffsets.nelements());
  return itsBeamOffsets[index];
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
      CONRADTHROW(DataAccessError, "Antenna ID requested ("<<antID<<
          ") is outside the range of the FEED table (max. antenna number is "<<
          itsIndices.nrow());
  }
  if (feedID>=itsIndices.ncolumn()) {
      CONRADTHROW(DataAccessError, "Feed ID requested ("<<feedID<<
          ") is outside the range of the FEED table (max. antenna number is "<<
          itsIndices.ncolumn());
  }
  const casa::Int index=itsIndices(antID,feedID);
  if (index<0) {
      CONRADTHROW(DataAccessError, "Requiested Antenna ID="<<antID<<
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
      (spWinID==itsCachedSpWindow || itsCachedSpWindow==-1)) {
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
  casa::TableExprNode expression= ((table().col("SPECTRAL_WINDOW_ID") ==
                static_cast<casa::Int>(spWinID)) || 
                    (table().col("SPECTRAL_WINDOW_ID") == -1)) &&
               (table().col("TIME") <= dTime) &&
                (table().col("TIME") + itsIntervalFactor*table().col("INTERVAL")>=
                 dTime);
  casa::Table selection=table()(expression);
  if (selection.nrow()==0) {
      CONRADTHROW(DataAccessError, "FEED subtable is empty");
  }
  itsBeamOffsets.resize(selection.nrow());
  itsPositionAngles.resize(selection.nrow());
  casa::ROScalarColumn<casa::Int> antIDs(selection,"ANTENNA_ID");
  casa::Int minAntID=-1,maxAntID=-1;
  casa::minMax(minAntID,maxAntID,antIDs.getColumn());
  casa::ROScalarColumn<casa::Int> feedIDs(selection,"FEED_ID");
  casa::Int minFeedID=-1,maxFeedID=-1;
  casa::minMax(minFeedID,maxFeedID,feedIDs.getColumn());
  if (minAntID<0 || maxAntID<0 || minFeedID<0 || maxFeedID<0) {
      CONRADTHROW(DataAccessError,"Negative indices in FEED_ID and ANTENNA_ID "
         "columns of the FEED subtable are not allowed");
  }
  ++maxAntID; ++maxFeedID; // now we have numbers of feeds and antennae
  CONRADDEBUGASSERT(maxAntID*maxFeedID == selection.nrow());
  itsIndices.resize(maxAntID,maxFeedID);
  itsIndices.set(-2); // negative value is a flag, which means an 
                      // uninitialized index
  casa::ROArrayColumn<casa::Double>  rcptrOffsets(selection,"BEAM_OFFSET");
  casa::ROArrayColumn<casa::Double>  rcptrPAs(selection,"RECEPTOR_ANGLE");
  for (casa::uInt row=0; row<selection.nrow(); ++row) {
       computeBeamOffset(rcptrOffsets(row),itsBeamOffsets[row]);
       itsPositionAngles[row]=computePositionAngle(rcptrPAs(row));
       itsIndices(antIDs(row),feedIDs(row))=row;
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
  CONRADASSERT(rcptOffsets.ndim()<3);
  if (rcptOffsets.ndim()==1) {
      // this means that have just one receptor and nothing, but copying
      // of values is required
      CONRADASSERT(rcptOffsets.nelements()==2);
      beamOffsets(0)=rcptOffsets(casa::IPosition(1,0));
      beamOffsets(1)=rcptOffsets(casa::IPosition(1,1));
  } else {
      const casa::IPosition &shape=rcptOffsets.shape();
      CONRADASSERT(shape[0] == 2);
      CONRADASSERT(shape[1] > 0);
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
  CONRADDEBUGASSERT(rcptAngles.ndim()==1);
  CONRADASSERT(rcptAngles.nelements()>=1);
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
  CONRADDEBUGASSERT(spWinID>=0);
  if (newBeamDetails(time,spWinID)) {
      fillCache(time,spWinID);
  }
  const casa::uInt index=getIndex(antID,feedID);
  CONRADDEBUGASSERT(index<=itsPositionAngles.nelements());
  return itsPositionAngles[index];
}                                 
  