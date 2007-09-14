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

// own includes
#include <measurementequation/GainCalibrationEquation.h>
#include <conrad/ConradError.h>
#include <dataaccess/MemBufferDataAccessor.h>
#include <measurementequation/VectorOperations.h>
#include <fitting/NormalEquations.h>
#include <fitting/DesignMatrix.h>


// std includes
#include <algorithm>
#include <functional>
#include <string>
#include <exception>
#include <sstream>

using casa::IPosition;
using conrad::scimath::DesignMatrix;

namespace conrad {

namespace synthesis {

/// a number of helper functions are gathered in this namespace
namespace utility {

// probably something like this exists somewhere in the Blob support.
// I grabbed this code from one of my old programs to speed up the development.
// If the appropriate code existing somewhere else in conrad is structured out
// we can move to use it instead. 

/// @brief helper method to interpret string
/// @details any type supported by the input stream can be converted
/// using this method (e.g. string to numbers)
/// @param[in] str input string
/// @return result of the conversion
/// @exception ConradError is thrown if the conversion failed
template<class T> T fromString(const std::string &str) throw(std::bad_cast) {
         std::istringstream is(str);
         T buf;
         is>>buf;
         if (!is) {
             CONRADTHROW(ConradError, "Unable to convert "<<str);
         } 
         return buf;
}

/// @brief helper method to convert any type (e.g. numbers) to a string
/// @details any type supported by the input stream can be converted
/// using this method.
/// @param[in] a const reference to the value to convert
/// @return resulting string
/// @exception ConradError is thrown if the conversion failed
template<class T> std::string toString(const T &in) throw(std::bad_cast) {
         std::ostringstream os;
         os<<in;
         if (!os) {
             CONRADTHROW(ConradError, "Unable to convert "<<in<<" to string");
         }
         return os.str();
}

} // namespace utility

} // namespace synthesis

} // namespace conrad

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
  MemBufferDataAccessor  buffChunk(chunk);
  itsPerfectVisME.predict(buffChunk);
  
  const casa::Cube<casa::Complex> &modelVis = buffChunk.visibility();
  const casa::Cube<casa::Complex> &measuredVis = chunk.visibility();
  
  CONRADDEBUGASSERT(buffChunk.nPol());
  
  const casa::uInt nPol = buffChunk.nPol()>1? 2: 1;
  // the following assumes that no parameters are missed, i.e. gains.size()
  // is the number of antennae 
  const casa::uInt nDataPerPol = 2*buffChunk.nChannel();
  casa::Cube<double> derivatives(buffChunk.nRow()*buffChunk.nChannel()*nPol,2,
                                 gains.size(),0.);
  casa::Vector<double> residual(nDataPerPol*buffChunk.nRow()*nPol);
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
                casa::Complex(0.,imag(g1))*conj(g2);
            copyVector(buf, derivatives(IPosition(3,offset,0,ant1), 
                               IPosition(3,offset+nDataPerPol-1,0,ant1)));
            buf = modelVis.xyPlane(pol).row(row)*casa::Complex(real(g1))*conj(g2);
            copyVector(buf, derivatives(IPosition(3,offset,1,ant1), 
                               IPosition(3,offset+nDataPerPol-1,1,ant1)));
            buf = modelVis.xyPlane(pol).row(row)*casa::Complex(0.,-imag(g2))*g1;
            copyVector(buf, derivatives(IPosition(3,offset,0,ant2), 
                               IPosition(3,offset+nDataPerPol-1,0,ant2)));
            buf = modelVis.xyPlane(pol).row(row)*casa::Complex(real(g2))*g1;
            copyVector(buf, derivatives(IPosition(3,offset,1,ant2), 
                               IPosition(3,offset+nDataPerPol-1,1,ant2)));                    
            buf = modelVis.xyPlane(pol).row(row)*conj(g2)*g1;
            copyVector(measuredVis.xyPlane(pol).row(row),
                 residual(IPosition(1,offset),IPosition(1,offset+nDataPerPol-1)));
            subtractVector(buf, residual(IPosition(1,offset),
                           IPosition(1,offset+nDataPerPol-1)));           
       }   
  }
  
  DesignMatrix designmatrix(parameters());
  CONRADDEBUGASSERT(itsNameCache.size() >= gains.size());
  for (casa::uInt ant = 0; ant<gains.size(); ++ant) {
       designmatrix.addDerivative(itsNameCache[ant],derivatives.xyPlane(ant));
  }
  designmatrix.addResidual(residual,casa::Vector<double>(residual.size(),1.));
  ne.add(designmatrix);
}                                   

// @brief fill the gains cache from parameters
// @param[in] a reference to the container to fill with values
void GainCalibrationEquation::fillGainsCache(GainsCacheType &in) const
{
  std::vector<std::string> completions(parameters().completions("gain"));
  in.resize(completions.size());
  itsNameCache.resize(completions.size());
  if(!completions.size()) {
     return;
  }
  for (std::vector<std::string>::const_iterator it=completions.begin();
                                                it!=completions.end();++it)  {
       if (it->find("g") == 0) {
           std::vector<std::string> parString;
           splitParameterString(*it,parString);
           if (parString.size()<2) {
               CONRADTHROW(CheckError, "Parameter "<<*it<<" has incomplete name");
           }
           if (parString[0]!="g11" && parString[1]!="g22") {
               CONRADTHROW(CheckError, "Only gains.g11.* and gains.g22.* are recognized, not "<<
                           *it);
           }
           // next check is temporary
           if (parString.size()>2) {
               CONRADTHROW(ConradError, "Feed- and time-dependent gains are not yet implemented");
           }
           const int antID = utility::fromString<int>(parString[1]);
           if (antID<0 || antID>=in.size()) {
               CONRADTHROW(ConradError, "Parameter "<<*it<<" has incorrect antenna ID ("<<
                          parString[1]<<")");    
           }
           casa::Complex gain(0.,0.);
           if (parameters().isScalar("gain"+*it)) {
               gain = parameters().scalarValue("gain"+*it);
           } else {
               const casa::Array<double> &gains = parameters().value("gain"+*it);
               gain = casa::Complex(gains(casa::IPosition(1,0)),
                                    gains(casa::IPosition(1,1)));
           }
           
           CONRADDEBUGASSERT(antID<in.size());
           CONRADDEBUGASSERT(antID<itsNameCache.size());
           
           if (parString[0] == "g11") {
               in[antID].first = gain; 
           } else {
               in[antID].second = gain; 
           } 
           itsNameCache[antID] = "gain"+*it;
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
         if (pos == std::string::npos) {
             parts.push_back(str.substr(pos));
         } else {
           parts.push_back(str.substr(pos,newPos-pos));
           pos=newPos+1;
         }
  }
}