/// @file
/// 
/// @brief A measurement equation describing antenna gains.
/// @details This measurement equation just multiplies by a gain matrix
/// visibilities produced by another measurement equation. It also generates
/// normal equations, which allow to solve for unknowns in the gain matrix.
/// 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

// casa includes
#include <casa/complex.h>

// own includes
#include <measurementequation/GainCalibrationEquation.h>
#include <conrad_synthesis.h>
#include <conrad/ConradLogging.h>
CONRAD_LOGGER(logger, "");

#include <conrad/ConradError.h>
#include <conrad/ConradUtil.h>
#include <dataaccess/MemBufferDataAccessor.h>
#include <measurementequation/VectorOperations.h>
#include <fitting/GenericNormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <fitting/Params.h>
#include <fitting/ComplexDiff.h>
#include <fitting/ComplexDiffMatrix.h>


// std includes
#include <algorithm>
#include <functional>
#include <string>
#include <exception>
#include <sstream>
#include <set>


using casa::IPosition;
using conrad::scimath::DesignMatrix;


using namespace conrad;
using namespace conrad::synthesis;

/// @brief Standard constructor using the parameters and the
/// data iterator.
/// @param[in] ip Parameters
/// @param[in] idi data iterator
/// @param[in] ime measurement equation describing perfect visibilities
/// @note In the future, measurement equations will work with accessors
/// only, and, therefore, the dependency on iterator will be removed
GainCalibrationEquation::GainCalibrationEquation(const conrad::scimath::Params& ip,
          const IDataSharedIter& idi, const IMeasurementEquation &ime) :
            MultiChunkEquation(idi), conrad::scimath::GenericEquation(ip),
            itsPerfectVisME(ime) {}
  
/// @brief Predict model visibilities for one accessor (chunk).
/// @details This version of the predict method works with
/// a single chunk of data only. It seems that all measurement
/// equations should work with accessors rather than iterators
/// (i.e. the iteration over chunks should be moved to the higher
/// level, outside this class). In the future, I expect that
/// predict() without parameters will be deprecated.
/// @param[in] chunk a read-write accessor to work with
void GainCalibrationEquation::predict(IDataAccessor &chunk) const
{
  casa::Cube<casa::Complex> &rwVis = chunk.rwVisibility();
  CONRADDEBUGASSERT(rwVis.nplane());
  
  // we don't do cross-pols at the moment. Maximum allowed number of 
  // polarisation products is 2.
  const casa::uInt nPol = chunk.nPol()>1? 2: 1;
  
  itsPerfectVisME.predict(chunk);
  for (casa::uInt row = 0; row < chunk.nRow(); ++row) {
       const casa::uInt ant1 = chunk.antenna1()[row];
       const casa::uInt ant2 = chunk.antenna2()[row];
       CONRADASSERT(ant1!=ant2); // not yet implemented
       
       for (casa::uInt pol=0; pol<nPol; ++pol) {
            CONRADDEBUGASSERT(pol<2);
            
            const casa::Complex factor = parameters().complexValue(paramName(ant1,pol))*
                   conj(parameters().complexValue(paramName(ant2,pol)));
                   
            casa::Vector<casa::Complex> slice = rwVis.xyPlane(pol).row(row);
            slice *= factor;
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
void GainCalibrationEquation::calcEquations(const IConstDataAccessor &chunk,
                              conrad::scimath::GenericNormalEquations& ne) const
{
  MemBufferDataAccessor  buffChunk(chunk);
  itsPerfectVisME.predict(buffChunk);
  const casa::Cube<casa::Complex> &modelVis = buffChunk.visibility();
  const casa::Cube<casa::Complex> &measuredVis = chunk.visibility();
  
  CONRADDEBUGASSERT(buffChunk.nPol());
  CONRADDEBUGASSERT(buffChunk.nChannel());
  
  // we don't do cross-pols at the moment. Maximum allowed number of 
  // polarisation products is 2.
  const casa::uInt nPol = buffChunk.nPol()>1? 2: 1;
  
  const casa::uInt nDataPerPol = 2*buffChunk.nChannel();
  
  
  for (casa::uInt row = 0; row < buffChunk.nRow(); ++row) { 
       casa::uInt ant1 = chunk.antenna1()[row];
       casa::uInt ant2 = chunk.antenna2()[row];
   
       CONRADASSERT(ant1!=ant2); // not yet implemented
   
       casa::Matrix<casa::Complex> modelSlice = modelVis.yzPlane(row);
       // it will probably go away in future, when we will work with
       // all polarisation data
       const casa::Matrix<casa::Complex> modelPolSlice = 
            modelSlice(casa::IPosition(2,0,0), casa::IPosition(2,
                     buffChunk.nChannel()-1,nPol-1));
                     
       // matrix of autodifferentiation objects describing this row
       scimath::ComplexDiffMatrix cdm = modelPolSlice;
       
       //
       //
       //
       casa::Vector<double> residual(nDataPerPol*nPol);
       
       // there is probably an unnecessary copying, but it can't be
       // fixed while we have to convert each complex number to a pair
       // of real numbers
            
       // second axis distinguishes between derivatives by real part of gains and
       // that by imaginary part, the first axis has double the size of elements 
       // because each pair of adjacent elements corresponds to real and imaginary
       // parts of the value of derivative. The last axis of the cube is the 
       // parameter. 
       casa::Cube<double> derivatives(nDataPerPol*nPol,2,nPol*2,0.);
       
       // a vector with parameter names in the same order as they are present
       // in the cube of derivatives (last axis)
       std::vector<std::string> names;
       names.reserve(nPol*2);
        
       // effectively a Jones matrix at this stage
       scimath::ComplexDiffMatrix calFactor(nPol,nPol, casa::Complex(0.,0.));
       
       for (casa::uInt pol=0; pol<nPol; ++pol) {
            CONRADDEBUGASSERT(pol<2);
             
            // gains for antenna 1, polarisation pol
            const std::string g1name = paramName(ant1,pol);
            names.push_back(g1name);
            const casa::Complex g1 = parameters().complexValue(g1name);
            
            // gains for antenna 2, polarisation pol
            const std::string g2name = paramName(ant2,pol);
            names.push_back(g2name);
            const casa::Complex g2 = parameters().complexValue(g2name);
            
            calFactor(pol,pol) = scimath::ComplexDiff(g1name,g1)*
                 conj(scimath::ComplexDiff(g2name,g2));
            
            const casa::uInt offset=pol*nDataPerPol;
            casa::Vector<scimath::ComplexDiff> diffBuf =
                 modelVis.xyPlane(pol).row(row) * scimath::ComplexDiff(g1name,g1)*
                 conj(scimath::ComplexDiff(g2name,g2));
            
            copyReDerivativeVector(g1name, diffBuf, 
                    derivatives(IPosition(3,offset,0,pol*2), 
                               IPosition(3,offset+nDataPerPol-1,0,pol*2)));
            
            copyImDerivativeVector(g1name, diffBuf, 
                    derivatives(IPosition(3,offset,1,pol*2), 
                               IPosition(3,offset+nDataPerPol-1,1,pol*2)));

            copyReDerivativeVector(g2name, diffBuf, 
                    derivatives(IPosition(3,offset,0,pol*2+1), 
                               IPosition(3,offset+nDataPerPol-1,0,pol*2+1)));
            
            copyImDerivativeVector(g2name, diffBuf, 
                    derivatives(IPosition(3,offset,1,pol*2+1), 
                               IPosition(3,offset+nDataPerPol-1,1,pol*2+1)));

            copyVector(measuredVis.xyPlane(pol).row(row),
                 residual(IPosition(1,offset),IPosition(1,offset+nDataPerPol-1)));
            subtractVector(diffBuf, residual(IPosition(1,offset),
                           IPosition(1,offset+nDataPerPol-1)));           
                                                              
                       
       }
       
       DesignMatrix designmatrix;// old parameters: thisRowParams;
       
       for (casa::uInt par=0; par<names.size(); ++par) {
            CONRADDEBUGASSERT(par<derivatives.nplane());
            designmatrix.addDerivative(names[par],derivatives.xyPlane(par));
       }
       designmatrix.addResidual(residual,
                    casa::Vector<double>(residual.size(),1.));
       
       
       /*
       
       // new way of building design matrix
       casa::Matrix<casa::Complex> measuredSlice = measuredVis.yzPlane(row);
       
       // it will probably go away in future, when we will work with
       // all polarisation data
       const casa::Matrix<casa::Complex> measuredPolSlice = 
            measuredSlice(casa::IPosition(2,0,0), casa::IPosition(2,
                     buffChunk.nChannel()-1,nPol-1));
       
       designmatrix.addModel(cdm * calFactor, measuredPolSlice, 
                 casa::Matrix<double>(measuredPolSlice.nrow(),
                 measuredPolSlice.ncolumn(),1.));
       */
       ne.add(designmatrix);
  }
}                                   

/// @brief obtain a name of the parameter
/// @details This method returns the parameter name for a gain of the
/// given antenna and polarisation. In the future, we may add time and/or
/// feed number as well.
/// @param[in] ant antenna number (0-based)
/// @param[in] pol index of the polarisation product
std::string GainCalibrationEquation::paramName(casa::uInt ant, 
                                               casa::uInt pol)
{ 
  std::string res("gain.");
  if (!pol) {
      res+="g11.";
  } else if (pol == 1) {
      res+="g22.";
  } else {
      CONRADTHROW(ConradError, "Only parallel hand polarisation products are supported at the moment, you have pol="<<pol);
  }

  return res+utility::toString<casa::uInt>(ant);
}                                               

/// @brief helper method to split parameter string
/// @details Parameters have a form similar to "gain.g11.dt0.25",
/// one needs to have a way to extract this information from the string.
/// This method splits a string on each dot symbol and appends the all
/// substring to a given vector.
/// @param[in] str input string.
/// @param[out] parts non-const reference to a vector where substrings will
///                   be added to. 
void GainCalibrationEquation::splitParameterString(const std::string &str,
                   std::vector<std::string> &parts) throw()
{
  if (!str.size()) {
      return;
  }
  const size_t expectedSize = parts.size() + std::count_if(str.begin(),str.end(),
                           std::bind2nd(std::equal_to<char>(),'.')) + 1;        
  parts.reserve(expectedSize);
  
  // current starting position
  size_t pos=0; 
  while (pos!=std::string::npos && pos<str.size()) {
         size_t newPos = str.find(".",pos);
         if (newPos == std::string::npos) {
             parts.push_back(str.substr(pos));
             pos=newPos;
         } else {
           parts.push_back(str.substr(pos,newPos-pos));
           pos=newPos+1;
         }
  }
}

/// @brief Calculate the normal equations for the iterator
/// @details This version iterates through all chunks of data and
/// calls an abstract method declared in IMeasurementEquation for each 
/// individual accessor (each iteration of the iterator)
/// @note there is probably a problem with constness here. Hope this method is
/// only temporary here.
/// @param[in] ne Normal equations
void GainCalibrationEquation::calcGenericEquations(conrad::scimath::GenericNormalEquations& ne)
{
  MultiChunkEquation::calcGenericEquations(ne);
}

/// Clone this into a shared pointer
/// @return shared pointer to a copy
GainCalibrationEquation::ShPtr GainCalibrationEquation::clone() const
{
  return GainCalibrationEquation::ShPtr(new GainCalibrationEquation(*this));
}

