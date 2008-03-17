/// @file
/// 
/// @brief Base class for generic measurement equation for calibration.
/// @details This is a base class for a template designed to represent any 
/// possible measurement equation we expect to encounter in calibration. 
/// It is a result of evolution of the former GainCalibrationEquation, which 
/// will probably be completely substituted by this template in the future. 
/// See CalibrationME template for more details. This class contains all
/// functionality, which doesn't depend on the template parameter.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

// casa includes
#include <casa/complex.h>

// own includes
#include <measurementequation/CalibrationMEBase.h>

#include <casa/Arrays/MatrixMath.h>
#include <askap/AskapError.h>
#include <askap/AskapUtil.h>
#include <dataaccess/MemBufferDataAccessor.h>
#include <fitting/GenericNormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <fitting/Params.h>
#include <fitting/ComplexDiff.h>
#include <fitting/ComplexDiffMatrix.h>


// std includes
//#include <algorithm>
//#include <functional>
#include <string>
#include <exception>


using casa::IPosition;
using askap::scimath::DesignMatrix;
using askap::scimath::ComplexDiffMatrix;


using namespace askap;
using namespace askap::synthesis;

/// @brief Standard constructor using the parameters and the
/// data iterator.
/// @param[in] ip Parameters
/// @param[in] idi data iterator
/// @param[in] ime measurement equation describing perfect visibilities
/// @note In the future, measurement equations will work with accessors
/// only, and, therefore, the dependency on iterator will be removed
CalibrationMEBase::CalibrationMEBase(const askap::scimath::Params& ip,
          const IDataSharedIter& idi, 
          const boost::shared_ptr<IMeasurementEquation const> &ime) :
            MultiChunkEquation(idi), askap::scimath::GenericEquation(ip),
            GenericMultiChunkEquation(idi), itsPerfectVisME(ime) {}
  
/// @brief Predict model visibilities for one accessor (chunk).
/// @details This version of the predict method works with
/// a single chunk of data only. It seems that all measurement
/// equations should work with accessors rather than iterators
/// (i.e. the iteration over chunks should be moved to the higher
/// level, outside this class). In the future, I expect that
/// predict() without parameters will be deprecated.
/// @param[in] chunk a read-write accessor to work with
void CalibrationMEBase::predict(IDataAccessor &chunk) const
{ 
  casa::Cube<casa::Complex> &rwVis = chunk.rwVisibility();
  ASKAPDEBUGASSERT(rwVis.nelements());
  ASKAPCHECK(itsPerfectVisME, "Perfect ME should be defined before calling CalibrationMEBase::predict");
 
  itsPerfectVisME->predict(chunk);
  for (casa::uInt row = 0; row < chunk.nRow(); ++row) {
       ComplexDiffMatrix cdm = buildComplexDiffMatrix(chunk, row) * 
            ComplexDiffMatrix(casa::transpose(chunk.visibility().yzPlane(row)));
       
       for (casa::uInt chan = 0; chan < chunk.nChannel(); ++chan) {
            for (casa::uInt pol = 0; pol < chunk.nPol(); ++pol) {
               // cdm is transposed! because we need a vector for
               // each spectral channel for a proper matrix multiplication
               rwVis(row, chan, pol) = cdm(pol, chan).value();
            }
       }
  }
}

/// @brief correct model visibilities for one accessor (chunk).
/// @detals This method corrects the data in the given accessor
/// (accessed via rwVisibility) for the calibration errors 
/// represented by this measurement equation (i.e. an inversion of
/// the matrix has been performed). 
/// @param[in] chunk a read-write accessor to work with
/// @note Need to think what to do in the inversion is unsuccessful
/// e.g. amend flagging information? This is not yet implemented as
/// existing accessors would throw an exception if flagging info is 
/// changed.
void CalibrationMEBase::correct(IDataAccessor &chunk) const
{
  casa::Cube<casa::Complex> &rwVis = chunk.rwVisibility();
  ASKAPDEBUGASSERT(rwVis.nelements());

  for (casa::uInt row = 0; row < chunk.nRow(); ++row) {
       ComplexDiffMatrix cdm = buildComplexDiffMatrix(chunk, row);
                    
       ASKAPASSERT(cdm.nRow()==cdm.nColumn()); // need to fix it in the future
       
       // cdm is transposed! because we need a vector for
       // each spectral channel for a proper matrix multiplication
       casa::Matrix<casa::Complex> effect(cdm.nRow(),cdm.nColumn());
       casa::Matrix<casa::Complex> reciprocal;
       casa::Complex det=0;
       for (casa::uInt i = 0; i <effect.nrow(); ++i) {
            for (casa::uInt j = 0; j < effect.ncolumn(); ++j) {
               effect(i, j) = cdm(j, i).value();
            }
       }
       invertSymPosDef(reciprocal, det, effect);
       if (abs(det)<1e-5) {
           ASKAPTHROW(AskapError, "Unable to apply gains, determinate too close to 0. D="<<abs(det));           
       }
       casa::Matrix<casa::Complex> thisRow = rwVis.yzPlane(row);
       
       casa::Matrix<casa::Complex> temp(thisRow.nrow(),reciprocal.ncolumn(),
                                     casa::Complex(0.,0.)); // = thisRow*reciprocal;
       for (casa::uInt i = 0; i < temp.nrow(); ++i) {
            for (casa::uInt j = 0; j < temp.ncolumn(); ++j) {
                 for (casa::uInt k = 0; k < thisRow.ncolumn(); ++k) {
                      temp(i,j) += thisRow(i,k)*reciprocal(k,j);
                 }  
            }
       }
       
       thisRow = temp;
  }
}

/// @brief Calculate the normal equation for one accessor (chunk).
/// @details This version of the method works on a single chunk of
/// data only (one iteration).It seems that all measurement
/// equations should work with accessors rather than iterators
/// (i.e. the iteration over chunks should be moved to the higher
/// level, outside this class). In the future, I expect that
/// the variant of the method without parameters will be deprecated.
/// @param[in] chunk a read-write accessor to work with
/// @param[in] ne Normal equations
void CalibrationMEBase::calcGenericEquations(const IConstDataAccessor &chunk,
                              askap::scimath::GenericNormalEquations& ne) const
{  
  MemBufferDataAccessor  buffChunk(chunk);
  ASKAPDEBUGASSERT(buffChunk.visibility().nelements());
  ASKAPCHECK(itsPerfectVisME, "Perfect ME should be defined before calling CalibrationMEBase::predict");
  
  itsPerfectVisME->predict(buffChunk);
  const casa::Cube<casa::Complex> &measuredVis = chunk.visibility();
  
  for (casa::uInt row = 0; row < buffChunk.nRow(); ++row) { 
       ComplexDiffMatrix cdm = buildComplexDiffMatrix(buffChunk, row) * 
            ComplexDiffMatrix(casa::transpose(buffChunk.visibility().yzPlane(row)));
       casa::Matrix<casa::Complex> measuredSlice = transpose(measuredVis.yzPlane(row));
       
       DesignMatrix designmatrix;
       // we can probably add below actual weights taken from the data accessor
       designmatrix.addModel(cdm, measuredSlice, 
                 casa::Matrix<double>(measuredSlice.nrow(),
                 measuredSlice.ncolumn(),1.));
      
       ne.add(designmatrix);
  }
}                                   


