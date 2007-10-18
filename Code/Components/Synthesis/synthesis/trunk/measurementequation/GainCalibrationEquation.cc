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
#include <casa/Complex.h>

// own includes
#include <measurementequation/GainCalibrationEquation.h>
#include <conrad/ConradError.h>
#include <conrad/ConradUtil.h>
#include <dataaccess/MemBufferDataAccessor.h>
#include <measurementequation/VectorOperations.h>
#include <fitting/NormalEquations.h>
#include <fitting/DesignMatrix.h>
#include <fitting/Params.h>


// std includes
#include <algorithm>
#include <functional>
#include <string>
#include <exception>
#include <sstream>

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
            MultiChunkEquation(idi), conrad::scimath::Equation(ip),
            itsPerfectVisME(ime), itsReferenceAntenna(0), 
            itsReferencePhase(casa::Complex(1.,0.)) {}
  
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
  const GainsCacheType &gains = 
         itsGainsCache.value(*this,&GainCalibrationEquation::fillGainsCache);
  casa::Cube<casa::Complex> &rwVis = chunk.rwVisibility();
  CONRADDEBUGASSERT(rwVis.nplane());
  
  itsPerfectVisME.predict(chunk);
  for (casa::uInt row = 0; row < chunk.nRow(); ++row) {
       casa::uInt ant1 = chunk.antenna1()[row];
       casa::uInt ant2 = chunk.antenna2()[row];
       
       if (ant1>=gains.size() || ant2>=gains.size()) {
           CONRADTHROW(ConradError, "Accessor contains data for antenna, which doesn't present in the"
                      " parameter list, ant1="<<ant1<<" ant2="<<ant2);
       }
       const std::pair<casa::Complex,casa::Complex> factor(gains[ant1].first*
            conj(gains[ant2].first), gains[ant1].second*conj(gains[ant2].second));
       
       // we need to have some code here to ensure that we deal with the native
       // polarisation system (i.e. a linear one).
       
       // first polarisation product
       casa::Vector<casa::Complex> slice = rwVis.xyPlane(0).row(row);
       slice *= factor.first;
       // second polarisation product
       if (rwVis.nplane()>=2) {
           slice = rwVis.xyPlane(1).row(row);
           slice *= factor.second;
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
                                   conrad::scimath::NormalEquations& ne) const
{
  const GainsCacheType &gains = 
         itsGainsCache.value(*this,&GainCalibrationEquation::fillGainsCache);
  if (gains.size() <= itsReferenceAntenna) {
      CONRADTHROW(ConradError, "A reference antenna "<<itsReferenceAntenna<<
                  " is outside the range, nAnts = "<<gains.size()<<
                  " or gains for some antennae are not specified"); 
  }       
  MemBufferDataAccessor  buffChunk(chunk);
  itsPerfectVisME.predict(buffChunk);
  
  const casa::Cube<casa::Complex> &modelVis = buffChunk.visibility();
  const casa::Cube<casa::Complex> &measuredVis = chunk.visibility();
  
  CONRADDEBUGASSERT(buffChunk.nPol());
  
  const casa::uInt nPol = buffChunk.nPol()>1? 2: 1;
  // the following assumes that no parameters are missed, i.e. gains.size()
  // is the number of antennae 
  const casa::uInt nDataPerPol = 2*buffChunk.nChannel();
  // second axis distinguishes between derivatives by real part of gains and
  // that by imaginary part, the first axis has double the size of elements 
  // because each pair of adjacent elements corresponds to real and imaginary
  // parts of the value of derivative
  casa::Cube<double> derivatives(nDataPerPol*buffChunk.nRow()*nPol+1+nPol,
                                 2,gains.size(),0.);
  casa::Vector<double> residual(nDataPerPol*buffChunk.nRow()*nPol+1+nPol);
  for (casa::uInt row = 0, offset = 0; row < buffChunk.nRow(); 
                                    ++row) { 
       casa::uInt ant1 = chunk.antenna1()[row];
       casa::uInt ant2 = chunk.antenna2()[row];
       if (ant1>=gains.size() || ant2>=gains.size()) {
           CONRADTHROW(ConradError, "Accessor contains data for antenna, which doesn't present in the"
                      " parameter list, ant1="<<ant1<<" ant2="<<ant2);
       }
       
       for (casa::uInt pol=0; pol<nPol; ++pol,offset+=nDataPerPol) {
            CONRADDEBUGASSERT(pol<2);
            // gains for antenna 1, polarisation pol
            const casa::Complex g1 = pol ? gains[ant1].second : gains[ant1].first;
            // gains for antenna 2, polarisation pol
            const casa::Complex g2 = pol ? gains[ant2].second : gains[ant2].first;
            // there is probably an unnecessary copying in the following code,
            // but we leave it as it is for now. I have a feeling that this
            // part has to be redesigned to have a faster and more readable code
            casa::Vector<casa::Complex> buf = modelVis.xyPlane(pol).row(row)*
                                              conj(g2);
            copyVector(buf, derivatives(IPosition(3,offset,0,ant1), 
                               IPosition(3,offset+nDataPerPol-1,0,ant1)));
            buf = modelVis.xyPlane(pol).row(row)*casa::Complex(0.,1.)*conj(g2);
            copyVector(buf, derivatives(IPosition(3,offset,1,ant1), 
                               IPosition(3,offset+nDataPerPol-1,1,ant1)));
            buf = modelVis.xyPlane(pol).row(row)*g1;
            copyVector(buf, derivatives(IPosition(3,offset,0,ant2), 
                               IPosition(3,offset+nDataPerPol-1,0,ant2)));
            buf = modelVis.xyPlane(pol).row(row)*casa::Complex(0.,-1.)*g1;
            copyVector(buf, derivatives(IPosition(3,offset,1,ant2), 
                               IPosition(3,offset+nDataPerPol-1,1,ant2)));                    
            buf = modelVis.xyPlane(pol).row(row)*conj(g2)*g1;
            copyVector(measuredVis.xyPlane(pol).row(row),
                 residual(IPosition(1,offset),IPosition(1,offset+nDataPerPol-1)));
            subtractVector(buf, residual(IPosition(1,offset),
                           IPosition(1,offset+nDataPerPol-1)));           
       }  
  }
  // add constraint to get an "absolute" phase with a desired origin
  const casa::uInt dataLength = nDataPerPol*buffChunk.nRow()*nPol;
  for (casa::uInt pol=0; pol<nPol; ++pol) {
       CONRADDEBUGASSERT(pol<2);
       
       const casa::Complex refGain = pol ? gains[itsReferenceAntenna].second : 
                                           gains[itsReferenceAntenna].first;
       residual[dataLength+pol]=-imag(itsReferencePhase*refGain);
       derivatives(dataLength+pol,0,itsReferenceAntenna) = imag(itsReferencePhase);
       derivatives(dataLength+pol,1,itsReferenceAntenna) = real(itsReferencePhase);
  }
                        
  
  // we need to remove all unused parameters before creating a design matrix
  scimath::Params::ShPtr tempParams(parameters().clone());
  for (std::vector<std::string>::const_iterator ci = itsUnusedParamNames.begin();
                              ci != itsUnusedParamNames.end(); ++ci) {
       tempParams->remove(*ci);
  }
  // if we have just one polarisation, we need to exclude the second one
  if (nPol == 1) {
      for (std::vector<std::pair<std::string,std::string> >::const_iterator 
                 ci = itsNameCache.begin(); ci != itsNameCache.end(); ++ci) {
           if (ci->second != "") {
               tempParams->remove(ci->second);
           }
      }                              
  }
  //
  //std::cout<<derivatives.xyPlane(0).column(0)<<std::endl;
  DesignMatrix designmatrix(*tempParams);
  tempParams.reset();
  CONRADDEBUGASSERT(itsNameCache.size() >= gains.size());
  for (casa::uInt ant = 0; ant<gains.size(); ++ant) {
       //std::cout<<"ant="<<ant<<" "<<itsNameCache[ant].first<<" "<<sum(derivatives.xyPlane(ant).column(1))<<std::endl;
       designmatrix.addDerivative(itsNameCache[ant].first,derivatives.xyPlane(ant));
       if (nPol>1) {
           designmatrix.addDerivative(itsNameCache[ant].second,derivatives.xyPlane(ant));
       }
  }
  designmatrix.addResidual(residual,casa::Vector<double>(residual.size(),1.));
  ne.add(designmatrix);
}                                   

/// @brief set reference antenna and its phase
/// @details It is impossible to determine absolute phase and some
/// value has to be adopted. This method sets the desired phase for
/// any particular antenna.
/// @param[in] ant a 0-based number of a new reference antenna
/// @param[in] phase a phase to adopt (in radians) 
/// @note This method can probably be made protected and these
/// parameters can be controlled via an additional fixed parameter.
void GainCalibrationEquation::setReferenceAntenna(casa::uInt ant, double phase)
{
  itsReferenceAntenna = ant;
  itsReferencePhase = casa::polar(casa::Float(1.),-casa::Float(phase));
}


// @brief fill the gains cache from parameters
// @param[in] a reference to the container to fill with values
void GainCalibrationEquation::fillGainsCache(GainsCacheType &in) const
{
  // it would be much nicer if we had an iterator over parameters with a
  // possible template match, but it is not currently in the interface of
  // the Param class 
  const std::vector<std::string> allFreeNames(parameters().freeNames());
  for (std::vector<std::string>::const_iterator ci = allFreeNames.begin();
                         ci != allFreeNames.end(); ++ci) {
       if (ci->find("gain") != 0) {
           // word "gain" is not found or the string doesn't start from it
           itsUnusedParamNames.push_back(*ci);
       }
  }

  std::vector<std::string> completions(parameters().completions("gain"));
  in.resize(completions.size()/2);
  itsNameCache.resize(completions.size()/2);
  if(!completions.size()) {
     return;
  } 
  for (std::vector<std::string>::const_iterator it=completions.begin();
                                                it!=completions.end();++it)  {
       if (it->find(".g") == 0) {
           std::vector<std::string> parString;
           splitParameterString(it->substr(1),parString);
           if (parString.size()<2) {
               CONRADTHROW(CheckError, "Parameter gain"<<*it<<" has incomplete name");
           }
           if (parString[0]!="g11" && parString[0]!="g22") {
               CONRADTHROW(CheckError, "Only gain.g11.* and gain.g22.* are recognized, not gain"<<
                           *it);
           }
           // next check is temporary
           if (parString.size()>2) {
               CONRADTHROW(ConradError, "Feed- and time-dependent gains are not yet implemented");
           }
           const int antID = utility::fromString<int>(parString[1]);
           if (antID<0 || antID>=in.size()) {
               CONRADTHROW(ConradError, "Parameter gain"<<*it<<" has incorrect antenna ID ("<<
                          parString[1]<<")");    
           }
           casa::Complex gain = parameters().complexValue("gain"+*it);
           CONRADDEBUGASSERT(antID<in.size());
           CONRADDEBUGASSERT(antID<itsNameCache.size());
           
           if (parString[0] == "g11") {
               in[antID].first = gain; 
               itsNameCache[antID].first = "gain"+*it;    
           } else {
               in[antID].second = gain; 
               itsNameCache[antID].second = "gain"+*it;    
           } 
           
       } else {
           itsUnusedParamNames.push_back("gain"+*it);
       }
  }
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

