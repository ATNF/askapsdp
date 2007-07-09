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

/// own includes
#include <dataaccess/TableConstDataIterator.h>
#include <conrad/ConradError.h>
#include <dataaccess/DataAccessError.h>

using namespace casa;
using namespace conrad;
using namespace conrad::synthesis;

/// @param[in] msManager a manager of the measurement set to use
/// @param[in] sel shared pointer to selector
/// @param[in] conv shared pointer to converter
/// @param[in] maxChunkSize maximum number of rows per accessor
TableConstDataIterator::TableConstDataIterator(
            const boost::shared_ptr<ITableManager const> &msManager,
            const boost::shared_ptr<ITableDataSelectorImpl const> &sel,
            const boost::shared_ptr<IDataConverterImpl const> &conv,
	    casa::uInt maxChunkSize) : TableInfoAccessor(msManager),
	    itsAccessor(*this), itsSelector(sel), itsConverter(conv), 
	    itsMaxChunkSize(maxChunkSize)
	   
{ 
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
  }  
}

/// @brief method ensures that the chunk has uniform DATA_DESC_ID
/// @details This method reduces itsNumberOfRows to the achieve
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
  if (itsCurrentDataDescID!=newDataDescID) {      
      itsAccessor.invalidateSpectralCaches();
      itsCurrentDataDescID=newDataDescID;
      
      // determine the shape of the visibility cube
      ROArrayColumn<Complex> visCol(itsCurrentIteration,"DATA");
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
  ROArrayColumn<Complex> visCol(itsCurrentIteration,"DATA");
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

/// populate the buffer with frequencies
/// @param[in] freq a reference to a vector to fill
void TableConstDataIterator::fillFrequency(casa::Vector<casa::Double> &freq) const
{  
  CONRADDEBUGASSERT(itsConverter);
  const ITableSpWindowHolder& spWindowSubtable=subtableInfo().getSpWindow();
  const int spWindowIndex = subtableInfo().getDataDescription().
                            getSpectralWindowID(itsCurrentDataDescID);
  if (spWindowIndex<0) {
      CONRADTHROW(DataAccessError,"A negative spectral window index ("<<
              spWindowIndex<<") is encountered for Data Description ID="<<
	      itsCurrentDataDescID);
  }
  
  if (itsConverter->isVoid(spWindowSubtable.getReferenceFrame(
              static_cast<const uInt>(spWindowIndex)),
	                   spWindowSubtable.getFrequencyUnit())) {
      // the conversion is void, i.e. table units/frame are exactly what
      // we need for output. This simplifies things a lot.
      freq.reference(spWindowSubtable.getFrequencies(
                               static_cast<const uInt>(spWindowIndex)));
      if (itsNumberOfChannels!=freq.nelements()) {
          CONRADTHROW(DataAccessError,"The measurement set has bad or corrupted "<<
	       "SPECTRAL_WINDOW subtable. The number of spectral channels for data "<<
	       itsNumberOfChannels<<" doesn't match the number of channels in the "<<
	       "frequency axis ("<<freq.nelements()<<")");
      }
  } else { 
      // have to process element by element as a conversion is required
      freq.resize(itsNumberOfChannels);
      for (uInt ch=0;ch<itsNumberOfChannels;++ch) {
           freq[ch]=itsConverter->frequency(spWindowSubtable.getFrequency(
	             static_cast<const uInt>(spWindowIndex),ch));
	                            
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
