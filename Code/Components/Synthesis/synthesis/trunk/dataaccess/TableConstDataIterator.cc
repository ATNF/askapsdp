/// @file TableConstDataIterator.cc
///
/// @brief Implementation of IConstDataIterator in the table-based case
/// @details
/// TableConstDataIterator: Allow read-only iteration across preselected data. Each 
/// iteration step is represented by the IConstDataAccessor interface.
/// This is an implementation in the table-based case.
/// 
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

/// casa includes
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <scimath/Mathematics/SquareMatrix.h>
#include <measures/Measures/MeasFrame.h>

/// own includes
#include <dataaccess/TableConstDataIterator.h>
#include <conrad_synthesis.h>
#include <conrad/ConradLogging.h>
CONRAD_LOGGER(logger, "");

#include <conrad/ConradError.h>
#include <dataaccess/DataAccessError.h>
#include <dataaccess/DirectionConverter.h>

using namespace casa;
using namespace conrad;
using namespace conrad::synthesis;

/// @param[in] msManager a manager of the measurement set to use
/// @param[in] sel shared pointer to selector
/// @param[in] conv shared pointer to converter
/// @param[in] dataColumn column name, which contains visibility data 
///                       default is DATA, but can be, e.g., CORRECTED_DATA
/// @param[in] maxChunkSize maximum number of rows per accessor
TableConstDataIterator::TableConstDataIterator(
            const boost::shared_ptr<ITableManager const> &msManager,
            const boost::shared_ptr<ITableDataSelectorImpl const> &sel,
            const boost::shared_ptr<IDataConverterImpl const> &conv,
            casa::uInt maxChunkSize) : 
        TableInfoAccessor(msManager),
	    itsAccessor(*this),
#ifndef CONRAD_DEBUG	    
        itsSelector(sel->clone()), 
	    itsConverter(conv->clone()),
#endif 
	    itsMaxChunkSize(maxChunkSize)
	   
{ 
  CONRADDEBUGASSERT(conv);
  CONRADDEBUGASSERT(sel);
  #ifdef CONRAD_DEBUG
    itsConverter = conv->clone();
    itsSelector  = sel->clone();
  #endif
  init();
}

/// Restart the iteration from the beginning
void TableConstDataIterator::init()
{ 
  itsCurrentTopRow=0;
  itsCurrentDataDescID=-100; // this value can't be in the table,
                             // therefore it is a flag of a new data descriptor
  const casa::TableExprNode &exprNode =
              itsSelector->getTableSelector(itsConverter);
  if (exprNode.isNull()) {
      itsTabIterator=casa::TableIterator(table(),"TIME",
	     casa::TableIterator::DontCare,casa::TableIterator::NoSort);
  } else {
      itsTabIterator=casa::TableIterator(table()(itsSelector->
                               getTableSelector(itsConverter)),"TIME",
	     casa::TableIterator::DontCare,casa::TableIterator::NoSort);
  }  
  setUpIteration();
}

/// operator* delivers a reference to data accessor (current chunk)
/// @return a reference to the current chunk
const IConstDataAccessor& TableConstDataIterator::operator*() const
{  
  return itsAccessor;
}
      
/// Checks whether there are more data available.
/// @return True if there are more data available
casa::Bool TableConstDataIterator::hasMore() const throw()
{
  if (!itsTabIterator.pastEnd()) {
      return true;
  }
  if (itsCurrentTopRow+itsNumberOfRows<itsCurrentIteration.nrow()) {
      return true;
  }   
  return false;
}
      
/// advance the iterator one step further 
/// @return True if there are more data (so constructions like 
///         while(it.next()) {} are possible)
casa::Bool TableConstDataIterator::next()
{
  itsCurrentTopRow+=itsNumberOfRows;
  if (itsCurrentTopRow>=itsCurrentIteration.nrow()) {
      CONRADDEBUGASSERT(!itsTabIterator.pastEnd());
      itsCurrentTopRow=0;
      // need to advance table iterator further
      itsTabIterator.next();      
      if (!itsTabIterator.pastEnd()) {
          setUpIteration();
      }      
  } else {
      uInt remainder=itsCurrentIteration.nrow()-itsCurrentTopRow;
      itsNumberOfRows=remainder<=itsMaxChunkSize ?
                      remainder : itsMaxChunkSize;      
      itsAccessor.invalidateIterationCaches();
      // itsDirectionCache don't need invalidation because the time is the same
      // as for the previous iteration
      
      // determine whether DATA_DESC_ID is uniform in the whole chunk
      // and reduce itsNumberOfRows if necessary
      makeUniformDataDescID();      
  }  
  return hasMore();
}

/// setup accessor for a new iteration of the table iterator
void TableConstDataIterator::setUpIteration()
{
  itsCurrentIteration=itsTabIterator.table();  
  itsAccessor.invalidateIterationCaches();
  
  itsNumberOfRows=itsCurrentIteration.nrow()<=itsMaxChunkSize ?
                  itsCurrentIteration.nrow() : itsMaxChunkSize;
  
  if (itsDirectionCache.isValid() && itsCurrentDataDescID>=0) {
      // extra checks make sense if the cache is valid (and this means it 
      // has been used before)
      const casa::MEpoch epoch = currentEpoch();
      if (subtableInfo().getField().newField(epoch) ||
          !subtableInfo().getAntenna().allEquatorial() ||
          subtableInfo().getFeed().newBeamDetails(epoch,currentSpWindowID())) {
              itsDirectionCache.invalidate();
      }
  }
  // retreive the number of channels and polarizations from the table
  if (itsNumberOfRows) {
      // determine whether DATA_DESC_ID is uniform in the whole chunk
      // and reduce itsNumberOfRows if necessary
      // set up visibility cube shape if necessary
      makeUniformDataDescID();
  } else {
      itsNumberOfChannels=0;
      itsNumberOfPols=0;
      itsCurrentDataDescID=-100;
      itsDirectionCache.invalidate();
  }  
}

/// @brief method ensures that the chunk has uniform DATA_DESC_ID
/// @details This method reduces itsNumberOfRows to achieve
/// uniform DATA_DESC_ID reading for all rows in the current chunk.
/// The resulting itsNumberOfRows will be 1 or more.
/// theirAccessor's spectral axis cache is reset if new DATA_DESC_ID is
/// different from itsCurrentDataDescID
/// This method also sets up itsNumberOfPols and itsNumberOfChannels
/// when DATA_DESC_ID changes (and therefore at the first run as well)
void TableConstDataIterator::makeUniformDataDescID()
{
  CONRADDEBUGASSERT(itsNumberOfRows);  
  CONRADDEBUGASSERT(itsCurrentTopRow+itsNumberOfRows<=
                    itsCurrentIteration.nrow());

  ROScalarColumn<Int> dataDescCol(itsCurrentIteration,"DATA_DESC_ID");
  const Int newDataDescID=dataDescCol(itsCurrentTopRow);
  CONRADDEBUGASSERT(newDataDescID>=0);
  if (itsCurrentDataDescID!=newDataDescID) {      
      itsAccessor.invalidateSpectralCaches();
      itsCurrentDataDescID=newDataDescID;
      if (itsDirectionCache.isValid()) {
          // if-statement, because it is pointless to do further checks in the 
          // case when the cache is already invalid due to 
          // the time change. In addition, checks require an access to the table,
          // which we want to avoid if, e.g., we don't need pointing direction 
          // at all
          if (subtableInfo().getFeed().newBeamDetails(currentEpoch(),
                                       currentSpWindowID())) {
              itsDirectionCache.invalidate();
          }
      }
      
      // determine the shape of the visibility cube
      ROArrayColumn<Complex> visCol(itsCurrentIteration,getDataColumnName());
      const casa::IPosition &shape=visCol.shape(itsCurrentTopRow);
      CONRADASSERT(shape.size() && (shape.size()<3));
      itsNumberOfPols=shape[0];
      itsNumberOfChannels=shape.size()>1?shape[1]:1;      
  }
  for (uInt row=1;row<itsNumberOfRows;++row) {
       if (dataDescCol(row+itsCurrentTopRow)!=itsCurrentDataDescID) {
           itsNumberOfRows=row;
	   break;
       }
  }
}


/// populate the buffer of visibilities with the values of current
/// iteration
/// @param[inout] vis a reference to the nRow x nChannel x nPol buffer
///            cube to fill with the complex visibility data
void TableConstDataIterator::fillVisibility(casa::Cube<casa::Complex> &vis) const
{
  vis.resize(itsNumberOfRows,itsNumberOfChannels,itsNumberOfPols);
  ROArrayColumn<Complex> visCol(itsCurrentIteration,getDataColumnName());
  // temporary buffer and position in this buffer, declared outside the loop
  IPosition curPos(2,itsNumberOfPols,itsNumberOfChannels);
  Array<Complex> buf(curPos);
  for (uInt row=0;row<itsNumberOfRows;++row) {
       const casa::IPosition &shape=visCol.shape(row);
       CONRADASSERT(shape.size() && (shape.size()<3));
       const casa::uInt thisRowNumberOfPols=shape[0];
       const casa::uInt thisRowNumberOfChannels=shape.size()>1?shape[1]:1;
       if (thisRowNumberOfPols!=itsNumberOfPols) {
           CONRADTHROW(DataAccessError,"Number of polarizations is not "
	               "conformant for row "<<row);           	       
       }
       if (thisRowNumberOfChannels!=itsNumberOfChannels) {
           CONRADTHROW(DataAccessError,"Number of channels is not "
	               "conformant for row "<<row);           	       
       }
       // for now just copy. In the future we will pass this array through
       // the transformation which will do averaging, selection,
       // polarization conversion

       // extract data record for this row, no resizing
       visCol.get(row+itsCurrentTopRow,buf,False); 
       
       for (uInt chan=0;chan<itsNumberOfChannels;++chan) {
            curPos[1]=chan;
            for (uInt pol=0;pol<itsNumberOfPols;++pol) {
	         curPos[0]=pol;
	         vis(row,chan,pol)=buf(curPos);
	    }
       }
  }
}

/// populate the buffer with uvw
/// @param[in] uvw a reference to vector of rigid vectors (3 elemets,
///            u,v and w for each row) to fill
void TableConstDataIterator::fillUVW(casa::Vector<casa::RigidVector<casa::Double, 3> >&uvw) const
{
  uvw.resize(itsNumberOfRows);

  ROArrayColumn<Double> uvwCol(itsCurrentIteration,"UVW");
  // temporary buffer and position in it
  IPosition curPos(1,3);
  Array<Double> buf(curPos);
  for (uInt row=0;row<itsNumberOfRows;++row) {
       const casa::IPosition &shape=uvwCol.shape(row);
       CONRADASSERT(shape.size()==1);
       CONRADASSERT(shape[0]==3);
       // extract data record for this row, no resizing     
       uvwCol.get(row+itsCurrentTopRow,buf,False);
       RigidVector<Double, 3> &thisRowUVW=uvw(row);
       for (curPos[0]=0;curPos[0]<3;++curPos[0]) {
            thisRowUVW(curPos[0])=buf(curPos);
       }
  }
}

/// @brief obtain a current spectral window ID
/// @details This method obtains a spectral window ID corresponding to the
/// current data description ID and tests its validity
/// @return current spectral window ID
casa::uInt TableConstDataIterator::currentSpWindowID() const
{
  CONRADDEBUGASSERT(itsCurrentDataDescID>=0);
  const int spWindowIndex = subtableInfo().getDataDescription().
                            getSpectralWindowID(itsCurrentDataDescID);
  if (spWindowIndex<0) {
      CONRADTHROW(DataAccessError,"A negative spectral window index ("<<
              spWindowIndex<<") is encountered for Data Description ID="<<
	      itsCurrentDataDescID);
  }
  return static_cast<const uInt>(spWindowIndex);
}  


/// populate the buffer with frequencies
/// @param[in] freq a reference to a vector to fill
void TableConstDataIterator::fillFrequency(casa::Vector<casa::Double> &freq) const
{  
  CONRADDEBUGASSERT(itsConverter);
  const ITableSpWindowHolder& spWindowSubtable=subtableInfo().getSpWindow();
  CONRADDEBUGASSERT(itsCurrentDataDescID>=0);
  const casa::uInt spWindowID = currentSpWindowID();
  
  if (itsConverter->isVoid(spWindowSubtable.getReferenceFrame(spWindowID),
	                   spWindowSubtable.getFrequencyUnit())) {
      // the conversion is void, i.e. table units/frame are exactly what
      // we need for output. This simplifies things a lot.
      freq.reference(spWindowSubtable.getFrequencies(spWindowID));
      if (itsNumberOfChannels!=freq.nelements()) {
          CONRADTHROW(DataAccessError,"The measurement set has bad or corrupted "<<
	       "SPECTRAL_WINDOW subtable. The number of spectral channels for data "<<
	       itsNumberOfChannels<<" doesn't match the number of channels in the "<<
	       "frequency axis ("<<freq.nelements()<<")");
      }
  } else { 
      // have to process element by element as a conversion is required
      const casa::MEpoch epoch=currentEpoch();
      // always use the dish pointing centre, rather than a pointing centre
      // of each individual feed for frequency conversion. The error is not
      // huge. If this code will ever work for SKA, this may need to be changed.
      // Currently use the FIELD table, not the actual pointing. It is probably
      // correct to use the phase centre for conversion as opposed to the
      // pointing centre.
      const casa::MDirection &antReferenceDir=subtableInfo().getField().
                                   getReferenceDir(epoch);
      // currently use the position of the first antenna for convertion.
      // we may need some average position + a check that they are close
      // enough to throw an exception if someone gives a VLBI measurement set.                             
      itsConverter->setMeasFrame(casa::MeasFrame(epoch, subtableInfo().
                     getAntenna().getPosition(0), antReferenceDir));
  
      freq.resize(itsNumberOfChannels);
      for (uInt ch=0;ch<itsNumberOfChannels;++ch) {
           freq[ch]=itsConverter->frequency(spWindowSubtable.getFrequency(
	                                    spWindowID,ch));
	                            
      }
  }
}

/// @return the time stamp  
casa::Double TableConstDataIterator::getTime() const 
{ 
  // add additional checks in debug mode
  #ifdef CONRAD_DEBUG
   ROScalarColumn<Double> timeCol(itsCurrentIteration,"TIME");
   Double time=timeCol(itsCurrentTopRow);
    Vector<Double> allTimes=timeCol.getColumnRange(Slicer(IPosition(1,
                       itsCurrentTopRow),IPosition(1,itsNumberOfRows)));
    for (uInt row=0;row<allTimes.nelements();++row)
         if (allTimes[row]!=time) {
             throw DataAccessLogicError("Time column is not homogeneous for each "
	                            "DataAccessor. This shouldn't happend");
	 }  
  #endif
  // end of additional checks

  ROScalarMeasColumn<MEpoch> timeMeasCol(itsCurrentIteration,"TIME"); 
  return itsConverter->epoch(timeMeasCol(itsCurrentTopRow)); 
}


/// populate the buffer with IDs of the first antenna
/// @param[in] ids a reference to a vector to fill
void TableConstDataIterator::fillAntenna1(casa::Vector<casa::uInt>& ids) const
{
  fillVectorOfIDs(ids,"ANTENNA1");
}

/// populate the buffer with IDs of the second antenna
/// @param[in] ids a reference to a vector to fill
void TableConstDataIterator::fillAntenna2(casa::Vector<casa::uInt> &ids) const
{
  fillVectorOfIDs(ids,"ANTENNA2");
}

/// populate the buffer with IDs of the first feed
/// @param[in] ids a reference to a vector to fill
void TableConstDataIterator::fillFeed1(casa::Vector<casa::uInt> &ids) const
{
  fillVectorOfIDs(ids,"FEED1");
}

/// populate the buffer with IDs of the second feed
/// @param[in] ids a reference to a vector to fill
void TableConstDataIterator::fillFeed2(casa::Vector<casa::uInt> &ids) const
{
  fillVectorOfIDs(ids,"FEED2");
}


/// @brief a helper method to read a column with IDs of some sort
/// @details It reads the column of casa::Int and fills a Vector of
/// casa::uInt. A check to ensure all numbers are non-negative is done
/// in the debug mode.
/// @param[in] ids a reference to a vector to fill
/// @param[in] name a name of the column to read
void TableConstDataIterator::fillVectorOfIDs(casa::Vector<casa::uInt> &ids,
                     const casa::String &name) const
{
  ROScalarColumn<Int> col(itsCurrentIteration,name);
  ids.resize(itsNumberOfRows);
  Vector<Int> buf=col.getColumnRange(Slicer(IPosition(1,
                      itsCurrentTopRow),IPosition(1,itsNumberOfRows)));
  CONRADDEBUGASSERT(buf.nelements()==ids.nelements());
  // need a copy because the type is different. There are no
  // appropriate cast operators for casa::Vectors
  Vector<Int>::const_iterator ci=buf.begin();
  Vector<uInt>::iterator it=ids.begin();
  for (; ci!=buf.end() && it!=ids.end() ; ++ci,++it) {
       CONRADDEBUGASSERT(*ci>=0);
	   *it=static_cast<uInt>(*ci);
  }
}

/// @brief an alternative way to get the time stamp
/// @details This method uses the accessor to get cached time stamp. It
/// is returned as an epoch measure.
casa::MEpoch TableConstDataIterator::currentEpoch() const 
{ 
  CONRADDEBUGASSERT(itsConverter);
  return itsConverter->epochMeasure(itsAccessor.time());
}  

/// @brief Fill internal buffer with the pointing directions.
/// @details  The layout of this buffer is the same as the layout of
/// the FEED subtable for current time and spectral window. 
/// getAntennaIDs and getFeedIDs methods of the 
/// subtable handler can be used to unwrap this 1D array. 
/// The buffer can invalidated if the time changes (i.e. for an alt-az array),
/// for an equatorial array this happends only if the FEED or FIELD subtable
/// are time-dependent
/// @param[in] dirs a reference to a vector to fill
void TableConstDataIterator::fillDirectionCache(casa::Vector<casa::MVDirection> &dirs) const
{
  const casa::MEpoch epoch=currentEpoch();
  CONRADDEBUGASSERT(itsCurrentDataDescID>=0);
  const casa::uInt spWindowID = currentSpWindowID();
  // antenna and feed IDs here are those in the FEED subtable, rather than
  // in the current accessor
  const casa::Vector<casa::Int> &antIDs=subtableInfo().getFeed().
                             getAntennaIDs(epoch,spWindowID);                             
  const casa::Vector<casa::Int> &feedIDs=subtableInfo().getFeed().
                             getFeedIDs(epoch,spWindowID);
  CONRADDEBUGASSERT(antIDs.nelements() == feedIDs.nelements());                           
  dirs.resize(antIDs.nelements());
  // we currently use FIELD table to get the pointing direction. This table
  // does not depend on the antenna.
  casa::MDirection antReferenceDir=subtableInfo().getField().
                                   getReferenceDir(epoch);
                                   
  // we need a separate converter for parallactic angle calculations
  DirectionConverter dirConv((casa::MDirection::Ref(casa::MDirection::AZEL)));
  dirConv.setMeasFrame(epoch);                  
  
  const casa::Vector<casa::RigidVector<casa::Double, 2> > &offsets =
               subtableInfo().getFeed().getAllBeamOffsets(epoch,spWindowID);
  for (casa::uInt element=0;element<antIDs.nelements();++element) {
       const casa::uInt ant=antIDs[element];
       //const casa::uInt feed=feedIDs[element];
       const casa::String &antMount=subtableInfo().getAntenna().
                                            getMount(ant);
       // if we decide to be paranoid about performance, we can add a method
       // to the converter to test whether antenna position and/or epoch are
       // really required to the requested convertion. Because the antenna 
       // position are cached, the overhead of the present straightforward
       // approach should be relatively minor.                                                             
       itsConverter->setMeasFrame(casa::MeasFrame(epoch,subtableInfo().
                     getAntenna().getPosition(ant)));
       
       casa::RigidVector<casa::Double, 2> offset = offsets[element];
       if (antMount == "ALT-AZ" || antMount == "alt-az")  {
           // need to do parallactic angle rotation
           const casa::MVDirection zenith(0.,1.);
           dirConv.setMeasFrame(casa::MeasFrame(subtableInfo().getAntenna().
                                getPosition(ant),epoch));
           const casa::Double posAngle=dirConv(antReferenceDir).
                                        positionAngle(zenith);
           casa::SquareMatrix<casa::Double, 2> 
                       rotMatrix(casa::SquareMatrix<casa::Double, 2>::General);
           const casa::Double cpa=cos(posAngle);
           const casa::Double spa=sin(posAngle);            
           rotMatrix(0,0)=cpa;
           rotMatrix(0,1)=-spa;
           rotMatrix(1,0)=spa;
           rotMatrix(1,1)=cpa;
           offset*=rotMatrix;                                        
       } else if (antMount != "EQUATORIAL" && antMount != "equatorial") {
           CONRADTHROW(DataAccessError,"Unknown mount type "<<antMount<<
                " for antenna "<<ant);
       }
       casa::MDirection feedPointingCentre(antReferenceDir);
       // x direction is fliped to convert az-el type frame to ra-dec           
       feedPointingCentre.shift(casa::MVDirection(-offset(0),
                             offset(1)),casa::True);
       itsConverter->direction(feedPointingCentre,dirs[element]);
  }
}

               

/// fill the buffer with the pointing directions of the first antenna/feed
/// @param[in] dirs a reference to a vector to fill
void TableConstDataIterator::fillPointingDir1(
                          casa::Vector<casa::MVDirection> &dirs) const
{ 
  const casa::Vector<casa::uInt> &feedIDs=itsAccessor.feed1();
  const casa::Vector<casa::uInt> &antIDs=itsAccessor.antenna1();
  fillVectorOfPointings(dirs,antIDs,feedIDs);
}

/// fill the buffer with the pointing directions of the second antenna/feed
/// @param[in] dirs a reference to a vector to fill
void TableConstDataIterator::fillPointingDir2(
                          casa::Vector<casa::MVDirection> &dirs) const
{ 
  const casa::Vector<casa::uInt> &feedIDs=itsAccessor.feed2();
  const casa::Vector<casa::uInt> &antIDs=itsAccessor.antenna2();
  fillVectorOfPointings(dirs,antIDs,feedIDs);
}


/// @brief A helper method to fill a given vector with pointingdirections.
/// @details fillPointingDir1 and fillPointingDir2 methods do very similar
/// operations, which differ only by the feedIDs and antennaIDs used.
/// This method encapsulates these common operations
/// @param[in] dirs a reference to a vector to fill
/// @param[in] antIDs a vector with antenna IDs
/// @param[in] feedIDs a vector with feed IDs
void TableConstDataIterator::fillVectorOfPointings(
               casa::Vector<casa::MVDirection> &dirs,
               const casa::Vector<casa::uInt> &antIDs,
               const casa::Vector<casa::uInt> &feedIDs) const
{
  CONRADDEBUGASSERT(antIDs.nelements() == feedIDs.nelements());
  const casa::Vector<casa::MVDirection> &directionCache = itsDirectionCache.
                      value(*this,&TableConstDataIterator::fillDirectionCache);
  const casa::Matrix<casa::Int> &directionCacheIndices = 
                 subtableInfo().getFeed().getIndices();
  dirs.resize(itsNumberOfRows);
  
  for (casa::uInt row=0; row<itsNumberOfRows; ++row) {
       if ((feedIDs[row]>=directionCacheIndices.ncolumn()) ||
           (antIDs[row]>=directionCacheIndices.nrow())) {
              CONRADTHROW(DataAccessError, "antID="<<antIDs[row]<<
                   " and/or feedID="<<feedIDs[row]<<
                   " are beyond the range of the FEED table");
           }
       if (directionCacheIndices(antIDs[row],feedIDs[row])<0) {
           CONRADTHROW(DataAccessError, "The pair andID="<<antIDs[row]<<
                   " feedID="<<feedIDs[row]<<" doesn't have beam parameters defined"); 
       }    
       const casa::uInt index = static_cast<casa::uInt>(
                    directionCacheIndices(antIDs[row],feedIDs[row]));
       CONRADDEBUGASSERT(index < directionCache.nelements());             
       dirs[row]=directionCache[index];             
  }                    
}

/// @brief obtain the name of the data column
/// @details The visibility data can be taken not only from the DATA column,
/// but from any other appropriate column, e.g. CORRECTED_DATA. This method
/// returns the name of the column used to store such data. We need it in
/// derived classes to perform writing
/// @return name of the table column with visibility data
const std::string& TableConstDataIterator::getDataColumnName() const throw() 
{ 
  CONRADDEBUGASSERT(itsSelector);
  return itsSelector->getDataColumnName();
}
