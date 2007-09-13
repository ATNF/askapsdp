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

#include <measurementequation/GainCalibrationEquation.h>
#include <conrad/ConradError.h>

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
  itsPerfectVisME.predict(chunk);
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
  itsPerfectVisME.calcEquations(chunk,ne);
}                                   

// @brief fill the gains cache from parameters
// @param[in] a reference to the container to fill with values
void GainCalibrationEquation::fillGainsCache(GainsCacheType &in) const
{
  std::vector<std::string> completions(parameters().completions("gains"));
  in.resize(completions.size());
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
  // current starting position
  size_t pos=0; 
  while (pos!=std::string::npos) {
         size_t newPos = str.find(".",pos);
         if (pos == std::string::npos) {
             parts.push_back(str.substr(pos));
         } else {
           parts.push_back(str.substr(pos,newPos-pos));
           pos=newPos+1;
           if (pos >= str.size()) {
               pos = std::string::npos;
           }
         }
  }
}