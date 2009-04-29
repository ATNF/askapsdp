/// @file TableConstDataIterator.cc
///
/// @brief Implementation of IConstDataIterator in the table-based case
/// @details
/// TableConstDataIterator: Allow read-only iteration across preselected data. Each 
/// iteration step is represented by the IConstDataAccessor interface.
/// This is an implementation in the table-based case.
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

/// casa includes
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <scimath/Mathematics/SquareMatrix.h>
#include <measures/Measures/MeasFrame.h>


/// own includes
#include <dataaccess/TableConstDataIterator.h>
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, "");

#include <askap/AskapError.h>
#include <dataaccess/DataAccessError.h>
#include <dataaccess/DirectionConverter.h>

using namespace casa;
using namespace askap;
using namespace askap::synthesis;

namespace askap {

namespace synthesis {

/// @brief a helper class to flag the whole row on the basis of FLAG_ROW
/// @details The method to read a cube (i.e. visibility or flag info) from
/// the table has been templated to allow the same code to work for both
/// casa::Complex visibilities and casa::Bool flags. There is, however, an
/// important difference. For flagging information, there is an extra
/// column, FLAG_ROW. If the appropriate element is True, all row should be
/// flagged. This is a helper template, which does nothing in the Complex
/// case, but performs required checks for casa::Bool. 
/// @ingroup dataaccess_tab
template<typename T>
struct WholeRowFlagger
{
  /// @brief constructor
  /// @details in the default version input parameter is not used
  inline WholeRowFlagger(const casa::Table &) {}
  
  /// @brief determine whether element by element copy is needed
  /// @details This method analyses other columns of the table specific
  /// for a particular type and fills the cube with appropriate data.
  /// If it can't do this, it returns true, which forces an element by element 
  /// processing. By default parameters are not used
  inline bool copyRequired(casa::uInt, casa::Cube<T> &) { return true;}
};


/// @brief a helper class to flag the whole row on the basis of FLAG_ROW
/// @details This is a specialization for casa::Bool (i.e. flagging information)
/// see the main template for details
/// @ingroup dataaccess_tab
template<>
struct WholeRowFlagger<casa::Bool>
{
  /// @brief constructor
  /// @details in the default version input parameter is not used
  /// @param[in] iteration current iteration (table returned by the iterator)
  inline WholeRowFlagger(const casa::Table &iteration);
  
  /// @brief determine whether element by element copy is needed
  /// @details This method analyses other columns of the table specific
  /// for a particular type and fills the cube with appropriate data.
  /// If it can't do this, it returns true, which forces an element by element 
  /// processing. By default parameters are not used
  /// @param[in] row a row to work with 
  /// @param[in] cube cube to work with
  inline bool copyRequired(casa::uInt row, casa::Cube<casa::Bool> &cube);
private:
  /// @brief accessor to the FLAG_ROW column
  ROScalarColumn<casa::Bool> itsFlagRowCol;
  /// @brief true if the dataset has FLAG_ROW column
  bool itsHasFlagRow;
};

WholeRowFlagger<casa::Bool>::WholeRowFlagger(const casa::Table &iteration) :
    itsHasFlagRow(iteration.tableDesc().isColumn("FLAG_ROW")) 
{
  if (itsHasFlagRow) {
      itsFlagRowCol.attach(iteration, "FLAG_ROW"); 
  }
}

bool WholeRowFlagger<casa::Bool>::copyRequired(casa::uInt row, 
                 casa::Cube<casa::Bool> &cube)
{
  ASKAPDEBUGASSERT(!itsFlagRowCol.isNull());
  if (itsHasFlagRow) {
      if (itsFlagRowCol.asBool(row)) {
          cube.yzPlane(row) = true;
          return false;
      } 
  }
  return true;
}


} // namespace synthesis

} // namespace askap

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
#ifndef ASKAP_DEBUG	    
        itsSelector(sel->clone()), 
	    itsConverter(conv->clone()),
#endif 
	    itsMaxChunkSize(maxChunkSize)
	   
{ 
  ASKAPDEBUGASSERT(conv);
  ASKAPDEBUGASSERT(sel);
  #ifdef ASKAP_DEBUG
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
  itsCurrentFieldID = -100; // this value can't be in the table,
                            // therefore it is a flag of a new field ID
  // by default use FIELD_ID column if it exists, otherwise use time to select
  // pointings
  itsUseFieldID = table().actualTableDesc().isColumn("FIELD_ID");  
  
  const casa::TableExprNode &exprNode =
              itsSelector->getTableSelector(itsConverter);
  if (exprNode.isNull()) {
      itsTabIterator=casa::TableIterator(table(),"TIME",
	     casa::TableIterator::Ascending,casa::TableIterator::NoSort);
  } else {
      itsTabIterator=casa::TableIterator(table()(itsSelector->
                               getTableSelector(itsConverter)),"TIME",
	     casa::TableIterator::Ascending,casa::TableIterator::NoSort);
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
      ASKAPDEBUGASSERT(!itsTabIterator.pastEnd());
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
      
      // determine whether FIELD_ID is uniform in the whole chunk
      // and reduce itsNumberOfRows if necessary
      // invalidate direction cache if necessary.
      // do nothing if itsUseFieldID is false
      makeUniformFieldID();
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
  
  if ((itsDirectionCache.isValid() || itsParallacticAngleCache.isValid()) 
       && itsCurrentDataDescID>=0) {
      // extra checks make sense if the cache is valid (and this means it 
      // has been used before)
      const casa::MEpoch epoch = currentEpoch();
      const casa::uInt spWindow = currentSpWindowID();
      const bool newField = itsUseFieldID ? false : subtableInfo().getField().newField(epoch);
      // a case where fieldID changes is dealt with separately.
      const IFeedSubtableHandler &feedSubtable = subtableInfo().getFeed();
      if ( newField || !subtableInfo().getAntenna().allEquatorial()) {
          itsParallacticAngleCache.invalidate();
      }
      if ( newField || ((!subtableInfo().getAntenna().allEquatorial() ||
           feedSubtable.newBeamDetails(epoch,spWindow)) &&
           !feedSubtable.allBeamOffsetsZero(epoch,spWindow))) {
              itsDirectionCache.invalidate();
              // rotated uvw depend on the direction (phase centres)
              itsAccessor.invalidateRotatedUVW();
              // itsDishPointingCache doesn't depend on feeds
              if (newField) {
                  itsDishPointingCache.invalidate();
              }
      }
  }
  // retreive the number of channels and polarizations from the table
  if (itsNumberOfRows) {
      // determine whether DATA_DESC_ID is uniform in the whole chunk
      // and reduce itsNumberOfRows if necessary
      // set up visibility cube shape if necessary
      makeUniformDataDescID();
      
      // determine whether FIELD_ID is uniform in the whole chink
      // and reduce itsNumberOfRows if necessary
      // invalidate direction cache if necessary.
      // do nothing if itsUseFieldID is false
      makeUniformFieldID();
  } else {
      itsNumberOfChannels = 0;
      itsNumberOfPols = 0;
      itsCurrentDataDescID = -100;
      itsCurrentFieldID = -100;
      itsDirectionCache.invalidate();
      // rotated uvw depends on the direction (phase centres)
      itsAccessor.invalidateRotatedUVW();
      itsParallacticAngleCache.invalidate();
      itsDishPointingCache.invalidate();
  }  
}

/// @brief method ensures that the chunk has uniform DATA_DESC_ID
/// @details This method reduces itsNumberOfRows to achieve a
/// uniform DATA_DESC_ID reading for all rows in the current chunk.
/// The resulting itsNumberOfRows will be 1 or more.
/// theirAccessor's spectral axis cache is reset if new DATA_DESC_ID is
/// different from itsCurrentDataDescID
/// This method also sets up itsNumberOfPols and itsNumberOfChannels
/// when DATA_DESC_ID changes (and therefore at the first run as well)
void TableConstDataIterator::makeUniformDataDescID()
{
  ASKAPDEBUGASSERT(itsNumberOfRows);  
  ASKAPDEBUGASSERT(itsCurrentTopRow+itsNumberOfRows<=
                    itsCurrentIteration.nrow());

  ROScalarColumn<Int> dataDescCol(itsCurrentIteration,"DATA_DESC_ID");
  const Int newDataDescID=dataDescCol(itsCurrentTopRow);
  ASKAPDEBUGASSERT(newDataDescID>=0);
  if (itsCurrentDataDescID!=newDataDescID) {      
      itsAccessor.invalidateSpectralCaches();
      itsCurrentDataDescID=newDataDescID;
      if (itsDirectionCache.isValid()) {
          // if-statement, because it is pointless to do further checks in the 
          // case when the cache is already invalid due to 
          // the time change. In addition, checks require an access to the table,
          // which we want to avoid if, e.g., we don't need pointing direction 
          // at all
          const casa::uInt spWindow = currentSpWindowID();
          const casa::MEpoch epoch = currentEpoch();
          const IFeedSubtableHandler &feedSubtable = subtableInfo().getFeed();
          if (!feedSubtable.allBeamOffsetsZero(epoch,spWindow)) {
              if (feedSubtable.newBeamDetails(epoch,spWindow)) {
                  itsDirectionCache.invalidate();
                  // rotated uvw depend on the direction (phase centres)
                  itsAccessor.invalidateRotatedUVW();
              }
          }
      }
      
      // determine the shape of the visibility cube
      ROArrayColumn<Complex> visCol(itsCurrentIteration,getDataColumnName());
      const casa::IPosition &shape=visCol.shape(itsCurrentTopRow);
      ASKAPASSERT(shape.size() && (shape.size()<3));
      itsNumberOfPols=shape[0];
      itsNumberOfChannels=shape.size()>1?shape[1]:1;
      ASKAPDEBUGASSERT(itsSelector);
      if (itsSelector->channelsSelected()) {
          // validity checks that selection doesn't extend beyond the channels available
          const std::pair<int,int> chanSelection = itsSelector->getChannelSelection();          
          ASKAPCHECK(itsNumberOfChannels >= casa::uInt(chanSelection.first + chanSelection.second),
               "Channel selection from "<<chanSelection.second+1<<" to "<<chanSelection.first+
               chanSelection.second<<" (1-based) extends beyond "<<itsNumberOfChannels<<
               " channel(s) available in  the dataset");
      }      
  }
  for (uInt row=1;row<itsNumberOfRows;++row) {
       if (dataDescCol(row+itsCurrentTopRow)!=itsCurrentDataDescID) {
           itsNumberOfRows=row;
	   break;
       }
  }
}

/// @brief method ensures that the chunk has a uniform FIELD_ID
/// @details This method reduces itsNumberOfRows until FIELD_ID is
/// the same for all rows in the current chunk. The resulting 
/// itsNumberOfRows will be 1 or more. If itsUseFieldID is false,
/// the method returns without doing anything. itsAccessor's direction
/// cache is reset if new FIELD_ID is different from itsCurrentFieldID
/// (and it sets it up at the first run as well)
void TableConstDataIterator::makeUniformFieldID()
{
  if (itsUseFieldID) {
      ASKAPDEBUGASSERT(itsNumberOfRows);  
      ASKAPDEBUGASSERT(itsCurrentTopRow+itsNumberOfRows<=
                       itsCurrentIteration.nrow());
                       
      ROScalarColumn<Int> fieldIDCol(itsCurrentIteration,"FIELD_ID");
      const Int newFieldID=fieldIDCol(itsCurrentTopRow);
      ASKAPDEBUGASSERT(newFieldID>=0);
      if (newFieldID != itsCurrentFieldID) {
          itsCurrentFieldID = newFieldID;
          itsDirectionCache.invalidate();
          // rotated uvw depend on the direction (phase centres)
          itsAccessor.invalidateRotatedUVW();
          itsParallacticAngleCache.invalidate();          
          itsDishPointingCache.invalidate();
      }
      // break the iteration if necessary
      for (uInt row=1;row<itsNumberOfRows;++row) {
           if (fieldIDCol(row+itsCurrentTopRow)!=itsCurrentFieldID) {
               itsNumberOfRows=row;
	           break;
           }
      }
  }
}

/// @brief read an array column of the table into a cube
/// @details populate the buffer provided with the information
/// read in the current iteration. This method is templated and can be
/// used for both visibility and flag data fillers.
/// @param[in] cube a reference to the nRow x nChannel x nPol buffer
///            cube to fill with the information from table
/// @param[in] columnName a name of the column to read
template<typename T>
void TableConstDataIterator::fillCube(casa::Cube<T> &cube, 
               const std::string &columnName) const
{
  cube.resize(itsNumberOfRows,itsNumberOfChannels,itsNumberOfPols);
  ROArrayColumn<T> tableCol(itsCurrentIteration,columnName);
  
  // helper class, which does nothing for visibility cube, but checks
  // FLAG_ROW for flagging
  WholeRowFlagger<T> wrFlagger(itsCurrentIteration);
  
  // temporary buffer and position in this buffer, declared outside the loop
  IPosition curPos(2,itsNumberOfPols,itsNumberOfChannels);
  Array<T> buf(curPos);
  for (uInt row=0;row<itsNumberOfRows;++row) {
       const casa::IPosition &shape=tableCol.shape(row);
       ASKAPASSERT(shape.size() && (shape.size()<3));
       const casa::uInt thisRowNumberOfPols=shape[0];
       const casa::uInt thisRowNumberOfChannels=shape.size()>1?shape[1]:1;
       if (thisRowNumberOfPols!=itsNumberOfPols) {
           ASKAPTHROW(DataAccessError,"Number of polarizations is not "
	               "conformant for row "<<row<<" of the "<<columnName<<
	               "column");           	       
       }
       if (thisRowNumberOfChannels!=itsNumberOfChannels) {
           ASKAPTHROW(DataAccessError,"Number of channels is not "
	               "conformant for row "<<row<<" of the "<<columnName<<
	               "column");           	       
       }
       // for now just copy. In the future we will pass this array through
       // the transformation which will do averaging, selection,
       // polarization conversion

       if (wrFlagger.copyRequired(row+itsCurrentTopRow,cube)) {
           // extract data record for this row, no resizing
           tableCol.get(row+itsCurrentTopRow,buf,False); 
       
           for (uInt chan=0;chan<itsNumberOfChannels;++chan) {
                curPos[1]=chan;
                for (uInt pol=0;pol<itsNumberOfPols;++pol) {
	                 curPos[0]=pol;
	                 cube(row,chan,pol)=buf(curPos);
	            }
           }
       }
  }
}               

/// populate the buffer of visibilities with the values of current
/// iteration
/// @param[inout] vis a reference to the nRow x nChannel x nPol buffer
///            cube to fill with the complex visibility data
void TableConstDataIterator::fillVisibility(casa::Cube<casa::Complex> &vis) const
{
  fillCube(vis, getDataColumnName());
}

/// @brief read flagging information
/// @details populate the buffer of flags with the information
/// read in the current iteration
/// @param[in] flag a reference to the nRow x nChannel x nPol buffer
///            cube to fill with the flag information (each element has
///            bool type)
void TableConstDataIterator::fillFlag(casa::Cube<casa::Bool> &flag) const
{
  fillCube(flag,"FLAG");
}

/// populate the buffer of noise figures with the values of current
/// iteration
/// @param[in] noise a reference to the nRow x nChannel x nPol buffer
///            cube to be filled with the noise figures
void TableConstDataIterator::fillNoise(casa::Cube<casa::Complex> &noise) const
{
  // default action first - just resize the cube and assign 1.
  noise.resize(itsNumberOfRows, itsNumberOfChannels, itsNumberOfPols);
  noise.set(casa::Complex(1.,0.));
  if (table().actualTableDesc().isColumn("SIGMA")) {
      ROArrayColumn<Float> sigmaCol(itsCurrentIteration,"SIGMA");
      for (uInt row = 0; row<itsNumberOfRows; ++row) {
           const casa::IPosition shape = sigmaCol.shape(row);
           ASKAPASSERT((shape.size()<=2) && (shape.size()!=0));
           if (shape.size() == 1) {
               // noise is given per polarisation, same for all spectral channels
               ASKAPASSERT(shape[0] == casa::Int(itsNumberOfPols));
               casa::Array<Float> buf(casa::IPosition(1,itsNumberOfPols));
               sigmaCol.get(row+itsCurrentTopRow,buf,False);
               casa::Matrix<casa::Complex> slice = noise.yzPlane(row);
               for (uInt chan = 0; chan<noise.ncolumn(); ++chan) {
                    ASKAPDEBUGASSERT(chan<slice.nrow());
                    casa::Vector<casa::Complex> polNoise = slice.row(chan);
                    convertArray(polNoise,buf);
               }
           } else {
               // noise is given per channel and polarisation
               ASKAPASSERT((shape[0] == casa::Int(itsNumberOfChannels)) && 
                           (shape[1] == casa::Int(itsNumberOfPols)));
               
               casa::Array<Float> buf(casa::IPosition(2,itsNumberOfChannels,itsNumberOfPols));
               sigmaCol.get(row+itsCurrentTopRow,buf,False);
                           
               // not clear whether we need a transpose of the matrix. This
               // case is not present in any available measurement set            
               casa::Matrix<casa::Complex> rowNoise = noise.yzPlane(row);
               convertArray(rowNoise, buf);            
           }
      } // loop over rows
  } // if-statement checking that SIGMA column is present
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
       const casa::IPosition shape=uvwCol.shape(row);
       ASKAPASSERT(shape.size()==1);
       ASKAPASSERT(shape[0]==3);
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
  ASKAPDEBUGASSERT(itsCurrentDataDescID>=0);
  const int spWindowIndex = subtableInfo().getDataDescription().
                            getSpectralWindowID(itsCurrentDataDescID);
  if (spWindowIndex<0) {
      ASKAPTHROW(DataAccessError,"A negative spectral window index ("<<
              spWindowIndex<<") is encountered for Data Description ID="<<
	      itsCurrentDataDescID);
  }
  return static_cast<const uInt>(spWindowIndex);
}  

/// @brief obtain a reference direction for the current iteration
/// @details Currently we assume that the dish pointing centre stays
/// fixed for the whole chunk. We break the iteration, if necessary
/// to achieve this. This helper method extracts the reference direction
/// from the FIELD subtable using either FIELD_ID, or current time if
/// the former is not supported by the main table.
/// @return a reference to direction measure
const casa::MDirection& TableConstDataIterator::getCurrentReferenceDir() const
{
  const IFieldSubtableHandler &fieldSubtable = subtableInfo().getField();
  if (itsUseFieldID) {
      ASKAPCHECK(itsCurrentFieldID>=0, "Elements of FIELD_ID column should be 0 or positive. You have "<<
                 itsCurrentFieldID);
      return fieldSubtable.getReferenceDir(itsCurrentFieldID);      
  }
  const casa::MEpoch epoch = currentEpoch();
  return fieldSubtable.getReferenceDir(epoch);
}

/// populate the buffer with frequencies
/// @param[in] freq a reference to a vector to fill
void TableConstDataIterator::fillFrequency(casa::Vector<casa::Double> &freq) const
{  
  ASKAPDEBUGASSERT(itsConverter);
  const ITableSpWindowHolder& spWindowSubtable=subtableInfo().getSpWindow();
  ASKAPDEBUGASSERT(itsCurrentDataDescID>=0);
  const casa::uInt spWindowID = currentSpWindowID();
  
  if (itsConverter->isVoid(spWindowSubtable.getReferenceFrame(spWindowID),
	                   spWindowSubtable.getFrequencyUnit())) {
      // the conversion is void, i.e. table units/frame are exactly what
      // we need for output. This simplifies things a lot.
      freq.reference(spWindowSubtable.getFrequencies(spWindowID));
      if (itsNumberOfChannels!=freq.nelements()) {
          ASKAPTHROW(DataAccessError,"The measurement set has bad or corrupted "<<
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
      const casa::MDirection& antReferenceDir = getCurrentReferenceDir();
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
  #ifdef ASKAP_DEBUG
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
  ASKAPDEBUGASSERT(buf.nelements()==ids.nelements());
  // need a copy because the type is different. There are no
  // appropriate cast operators for casa::Vectors
  Vector<Int>::const_iterator ci=buf.begin();
  Vector<uInt>::iterator it=ids.begin();
  for (; ci!=buf.end() && it!=ids.end() ; ++ci,++it) {
       ASKAPDEBUGASSERT(*ci>=0);
	   *it=static_cast<uInt>(*ci);
  }
}

/// @brief an alternative way to get the time stamp
/// @details This method uses the accessor to get cached time stamp. It
/// is returned as an epoch measure.
casa::MEpoch TableConstDataIterator::currentEpoch() const 
{ 
  ASKAPDEBUGASSERT(itsConverter);
  return itsConverter->epochMeasure(itsAccessor.time());
}  

/// @brief Fill internal buffer with parallactic angles
/// @details This buffer holds parallactic angles for all antennae. The buffer
/// is invalidated when the time changes for an alt-az array, for an equatorial
/// array it happens only if the pointing changes.
/// @param[in] angles a reference to a vector to be filled
void TableConstDataIterator::fillParallacticAngleCache(casa::Vector<casa::Double> &angles) const
{ 
  angles.resize(subtableInfo().getAntenna().getNumberOfAntennae());
  ASKAPDEBUGASSERT(angles.size());
  if (subtableInfo().getAntenna().allEquatorial()) {
      angles.set(0.);
  } else {
  
  const casa::MEpoch epoch=currentEpoch();
  
  // we need a separate converter for parallactic angle calculations
  DirectionConverter dirConv((casa::MDirection::Ref(casa::MDirection::AZEL)));
  dirConv.setMeasFrame(epoch);                  

  // we currently use FIELD table to get the pointing direction. This table
  // does not depend on the antenna.
  const casa::MDirection& antReferenceDir = getCurrentReferenceDir();


  for (casa::uInt ant = 0; ant<angles.size(); ++ant) {
       const casa::String &antMount=subtableInfo().getAntenna().
                                          getMount(ant);

       if (antMount == "ALT-AZ" || antMount == "alt-az")  {
           casa::MDirection celestialPole;
           celestialPole.set(MDirection::Ref(MDirection::HADEC));
           dirConv.setMeasFrame(casa::MeasFrame(subtableInfo().getAntenna().
                              getPosition(ant),epoch));
           const casa::Double pAngle=dirConv(antReferenceDir).
                               positionAngle(dirConv(celestialPole).getValue());
           angles[ant] = pAngle;
                                                    
       } else if (antMount != "EQUATORIAL" && antMount != "equatorial") {
           ASKAPTHROW(DataAccessError,"Unknown mount type "<<antMount<<
              " for antenna "<<ant);
       } else {
           // just in case we ever have a mixed array with different mounts
           angles[ant] = 0.;
       }
                  
    }   
  } // if all equatorial
}


/// @brief Fill internal buffer with the pointing directions
/// @details  The layout of this buffer is the same as the layout of
/// the FEED subtable for current time and spectral window. 
/// getAntennaIDs and getFeedIDs methods of the 
/// subtable handler can be used to unwrap this 1D array. 
/// The buffer can be invalidated if the time changes (i.e. for an alt-az array),
/// for an equatorial array this happends only if the FEED or FIELD subtable
/// are time-dependent or if FIELD_ID changes
/// @param[in] dirs a reference to a vector to fill
void TableConstDataIterator::fillDirectionCache(casa::Vector<casa::MVDirection> &dirs) const
{
  // the code fills both pointing directions and position angles. For ASKAP, it would
  // probably be a bit faster if we split these two operations between two methods, as
  // position angle will be fixed and will not need as much updating as the pointing.
  
  // threshold (in radians) beyond which a proper parallactic angle rotation is done
  // it is hard coded at the moment 
  const double parallacticAngleThreshold = 1e-9;
  
  const casa::Vector<casa::Double> &parallacticAngles = itsParallacticAngleCache.value(*this,
                 &TableConstDataIterator::fillParallacticAngleCache);  
  
  const IFeedSubtableHandler &feedSubtable = subtableInfo().getFeed();
  
  const casa::MEpoch epoch=currentEpoch();
  ASKAPDEBUGASSERT(itsCurrentDataDescID>=0);
  const casa::uInt spWindowID = currentSpWindowID();
  // antenna and feed IDs here are those in the FEED subtable, rather than
  // in the current accessor
  const casa::Vector<casa::Int> &antIDs = feedSubtable.getAntennaIDs(epoch,spWindowID);
  
  dirs.resize(antIDs.nelements());
  
  // we currently use FIELD table to get the pointing direction. This table
  // does not depend on the antenna.
  const casa::MDirection& antReferenceDir = getCurrentReferenceDir();
                                     
  const casa::Vector<casa::RigidVector<casa::Double, 2> > &offsets =
               feedSubtable.getAllBeamOffsets(epoch,spWindowID);
               
  for (casa::uInt element=0;element<antIDs.nelements();++element) {
       const casa::uInt ant=antIDs[element];

       // if we decide to be paranoid about performance, we can add a method
       // to the converter to test whether antenna position and/or epoch are
       // really required to the requested convertion. Because the antenna 
       // position are cached, the overhead of the present straightforward
       // approach should be relatively minor.                                                             
       itsConverter->setMeasFrame(casa::MeasFrame(epoch,subtableInfo().
                     getAntenna().getPosition(ant)));
       
       casa::RigidVector<casa::Double, 2> offset = offsets[element];
       ASKAPDEBUGASSERT(ant<parallacticAngles.nelements());
       const casa::Double posAngle = parallacticAngles[ant];

       if (std::abs(posAngle)>parallacticAngleThreshold)  {
           // need to do a proper parallactic angle rotation
           casa::SquareMatrix<casa::Double, 2> 
                       rotMatrix(casa::SquareMatrix<casa::Double, 2>::General);
           const casa::Double cpa=cos(posAngle);
           const casa::Double spa=sin(posAngle);            
           rotMatrix(0,0)=cpa;
           rotMatrix(0,1)=-spa;
           rotMatrix(1,0)=spa;
           rotMatrix(1,1)=cpa;
           offset*=rotMatrix;                                        
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

/// fill the buffer with the position angles of the first antenna/feed
/// @param[in] angles a reference to vector to be filled
void TableConstDataIterator::fillFeed1PA(casa::Vector<casa::Float> &angles) const
{
  const casa::Vector<casa::uInt> &feedIDs=itsAccessor.feed1();
  const casa::Vector<casa::uInt> &antIDs=itsAccessor.antenna1();
  fillVectorOfPositionAngles(angles,antIDs,feedIDs);  
}

/// fill the buffer with the position angles of the second antenna/feed
/// @param[in] angles a reference to vector to be filled
void TableConstDataIterator::fillFeed2PA(casa::Vector<casa::Float> &angles) const
{
  const casa::Vector<casa::uInt> &feedIDs=itsAccessor.feed2();
  const casa::Vector<casa::uInt> &antIDs=itsAccessor.antenna2();
  fillVectorOfPositionAngles(angles,antIDs,feedIDs);
}


/// @brief a helper method to get dish pointings 
/// @details fillDishPointing1 and fillDishPointing2 methods do very
/// similar operations, which differ only by the antennaIDs used.
/// This method encapsulated these common operations.
/// @note fillVectorOfPointings computes pointing directions for 
/// individual feeds, not for the centre of the dish as this method
/// does
/// @param[in] dirs a reference to a vector to fill
/// @param[in] antIDs a vector with antenna IDs  
void TableConstDataIterator::fillVectorOfDishPointings(casa::Vector<casa::MVDirection> &dirs,
               const casa::Vector<casa::uInt> &antIDs) const
{
  ASKAPDEBUGASSERT(itsNumberOfRows == antIDs.nelements());
  const casa::Vector<casa::MVDirection> &dishPointingCache = itsDishPointingCache.
                      value(*this,&TableConstDataIterator::fillDishPointingCache);
  dirs.resize(itsNumberOfRows);
  for (casa::uInt row=0; row<itsNumberOfRows; ++row) {
       ASKAPDEBUGASSERT(antIDs[row] < dishPointingCache.nelements());
       dirs[row] = dishPointingCache[antIDs[row]];
  }
}

/// @brief fill the buffer with the pointing directions for the first antenna centre
/// @details The difference from fillPointingDir1 is that no feed offset is applied.
/// @param[in] dirs a reference to a vector to fill
void TableConstDataIterator::fillDishPointing1(casa::Vector<casa::MVDirection> &dirs) const
{
  const casa::Vector<casa::uInt> &antIDs=itsAccessor.antenna1();
  fillVectorOfDishPointings(dirs,antIDs);
}
  
/// @brief fill the buffer with the pointing directions for the second antenna centre
/// @details The difference from fillPointingDir2 is that no feed offset is applied.
/// @param[in] dirs a reference to a vector to fill
void TableConstDataIterator::fillDishPointing2(casa::Vector<casa::MVDirection> &dirs) const
{
  const casa::Vector<casa::uInt> &antIDs=itsAccessor.antenna2();
  fillVectorOfDishPointings(dirs,antIDs);
}

/// @brief fill the buffer with the dish pointing directions
/// @details The difference from fillDirectionCache is that
/// this method computes the pointing directions for the dish centre, not for
/// individual feeds (or synthetic beams, strictly speaking). The number of elements
/// in the buffer equals to the number of antennae. This is also different from
/// fillDirectionCache, which projects feeds to the same 1D array as well. 
/// @note At this stage we use FIELD subtable to get the pointing directions.
/// Therefore, these directions do not depend on antenna/feed. This method writes
/// the same value for all elements of the array. It will be used for both antennae
/// in the pair. 
/// @param[in] dirs a reference to a vector to fill
void TableConstDataIterator::fillDishPointingCache(casa::Vector<casa::MVDirection> &dirs) const
{
  ASKAPDEBUGASSERT(itsConverter);
  const casa::MEpoch epoch = currentEpoch();
  
  dirs.resize(subtableInfo().getAntenna().getNumberOfAntennae());
  
  // we currently use FIELD table to get the pointing direction. This table
  // does not depend on the antenna. However, the reference frame can introduce such
  // a dependence (i.e. a large array and AZEL frame requested)
  const casa::MDirection& antReferenceDir = getCurrentReferenceDir();
  
  for (casa::uInt ant = 0; ant<dirs.nelements(); ++ant) {     
       // if we decide to be paranoid about performance, we can add a method
       // to the converter to test whether antenna position and/or epoch are
       // really required to the requested convertion. Because the antenna 
       // position are cached, the overhead of the present straightforward
       // approach should be relatively minor.                                                             
       itsConverter->setMeasFrame(casa::MeasFrame(epoch,subtableInfo().
                  getAntenna().getPosition(ant)));
       itsConverter->direction(antReferenceDir,dirs[ant]);           
  }
}

/// @brief A helper method to fill a given vector with pointing directions.
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
  ASKAPDEBUGASSERT(antIDs.nelements() == feedIDs.nelements());
  const casa::Vector<casa::MVDirection> &directionCache = 
      itsDirectionCache.value(*this,&TableConstDataIterator::fillDirectionCache);
  const casa::Matrix<casa::Int> &directionCacheIndices = 
                 subtableInfo().getFeed().getIndices();
  dirs.resize(itsNumberOfRows);
  
  for (casa::uInt row=0; row<itsNumberOfRows; ++row) {
       if ((feedIDs[row]>=directionCacheIndices.ncolumn()) ||
           (antIDs[row]>=directionCacheIndices.nrow())) {
              ASKAPTHROW(DataAccessError, "antID="<<antIDs[row]<<
                   " and/or feedID="<<feedIDs[row]<<
                   " are beyond the range of the FEED table");
           }
       if (directionCacheIndices(antIDs[row],feedIDs[row])<0) {
           ASKAPTHROW(DataAccessError, "The pair andID="<<antIDs[row]<<
                   " feedID="<<feedIDs[row]<<" doesn't have beam parameters defined"); 
       }    
       const casa::uInt index = static_cast<casa::uInt>(
                    directionCacheIndices(antIDs[row],feedIDs[row]));
       ASKAPDEBUGASSERT(index < directionCache.nelements());             
       dirs[row]=directionCache[index];             
  }                    
}

/// @brief A helper method to fill a given vector with position angles.
/// @details fillFeedPA1 and fillFeedPA2 method do very similar operations,
/// which differ only by the feedIDs and antennaIDs used.
/// This method encapsulated these common operations
/// @param[in] angles a reference to vector to fill
/// @param[in] antIDs a vector with antenna IDs
/// @param[in] feedIDs a vector with feed IDs
/// @note There are some similarities between the code of this method and that
/// of fillVectorOfPointings. They are different with just a command called within
/// the loop. Theoretically, we can combine these two methods together, it would just
/// invovle some coding to make it look nice and probably some minor performance penalty.
void TableConstDataIterator::fillVectorOfPositionAngles(casa::Vector<casa::Float> &angles,
              const casa::Vector<casa::uInt> &antIDs,
              const casa::Vector<casa::uInt> &feedIDs) const
{
  ASKAPDEBUGASSERT(antIDs.nelements() == feedIDs.nelements());
  const casa::Vector<casa::Double> &parallacticAngles = itsParallacticAngleCache.value(*this,
                 &TableConstDataIterator::fillParallacticAngleCache);  
  
  const casa::MEpoch epoch=currentEpoch();
  ASKAPDEBUGASSERT(itsCurrentDataDescID>=0);
  const casa::uInt spWindowID = currentSpWindowID();
  
  const IFeedSubtableHandler& feedSubtable = subtableInfo().getFeed();                                             
                 
  angles.resize(itsNumberOfRows);
  
  for (casa::uInt row=0; row<itsNumberOfRows; ++row) {
       ASKAPDEBUGASSERT(antIDs[row] < parallacticAngles.nelements());
       angles[row] = casa::Float(feedSubtable.getBeamPA(epoch, spWindowID, antIDs[row],
                               feedIDs[row]) + parallacticAngles[antIDs[row]]);
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
  ASKAPDEBUGASSERT(itsSelector);
  return itsSelector->getDataColumnName();
}
