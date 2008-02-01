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
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

// casa includes
#include <casa/complex.h>

// own includes
#include <measurementequation/CalibrationMEBase.h>

#include <casa/Arrays/MatrixMath.h>
#include <conrad/ConradError.h>
#include <conrad/ConradUtil.h>
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
using conrad::scimath::DesignMatrix;
using conrad::scimath::ComplexDiffMatrix;


using namespace conrad;
using namespace conrad::synthesis;

/// @brief Standard constructor using the parameters and the
/// data iterator.
/// @param[in] ip Parameters
/// @param[in] idi data iterator
/// @param[in] ime measurement equation describing perfect visibilities
/// @note In the future, measurement equations will work with accessors
/// only, and, therefore, the dependency on iterator will be removed
CalibrationMEBase::CalibrationMEBase(const conrad::scimath::Params& ip,
          const IDataSharedIter& idi, const IMeasurementEquation &ime) :
            MultiChunkEquation(idi), conrad::scimath::GenericEquation(ip),
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
  CONRADDEBUGASSERT(rwVis.nelements());

  itsPerfectVisME.predict(chunk);
    
  for (casa::uInt row = 0; row < chunk.nRow(); ++row) {
       ComplexDiffMatrix cdm = buildComplexDiffMatrix(chunk, row);
       
       for (casa::uInt chan = 0; chan < chunk.nChannel(); ++chan) {
            for (casa::uInt pol = 0; pol < chunk.nPol(); ++pol) {
                 // cdm is transposed! because we need a vector for
                 // each spectral channel for a proper matrix multiplication
                 rwVis(row, chan, pol) = cdm(pol, chan).value();
            }
       }
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
void CalibrationMEBase::calcEquations(const IConstDataAccessor &chunk,
                              conrad::scimath::GenericNormalEquations& ne) const
{  
  MemBufferDataAccessor  buffChunk(chunk);
  CONRADDEBUGASSERT(buffChunk.visibility().nelements());
  
  itsPerfectVisME.predict(buffChunk);
  const casa::Cube<casa::Complex> &measuredVis = chunk.visibility();
  
  for (casa::uInt row = 0; row < buffChunk.nRow(); ++row) { 
       ComplexDiffMatrix cdm = buildComplexDiffMatrix(buffChunk, row);
       casa::Matrix<casa::Complex> measuredSlice = transpose(measuredVis.yzPlane(row));
       
       DesignMatrix designmatrix;
       // we can probably add below actual weights taken from the data accessor
       designmatrix.addModel(cdm, measuredSlice, 
                 casa::Matrix<double>(measuredSlice.nrow(),
                 measuredSlice.ncolumn(),1.));
      
       ne.add(designmatrix);
  }
}                                   


