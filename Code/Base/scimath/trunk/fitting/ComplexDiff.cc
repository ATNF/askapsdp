/// @file
/// 
/// @brief Autodifferentiation class working for complex parameters
/// @details A creation of this class was inspired by the CASA's 
/// AutoDiff and SparseDiff classes. Its functionality and purpose are 
/// essentially the same as that for casa::SparseDiff. However, it works
/// correctly for complex parameters (i.e. it tracks derivatives by 
/// real and imaginary part of each parameter) and uses string indices.
/// It is quite likely, that in the future we will convert this template
/// to use casa::SparseDiff classes. An extra adapter layer will be required
/// anyway to convert string indices into integer indices and to deal with
/// complex-valued parameters properly. 
/// 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>


// own includes
#include <fitting/ComplexDiff.h>
#include <conrad/ConradError.h>

using namespace conrad;
using namespace conrad::scimath;

/// @brief default constructor -  a constant (0.,0.)
ComplexDiff::ComplexDiff() : itsValue(0.,0.) {}
  
/// @brief construct a complex constant
/// @param[in] in a reference to input complex value
ComplexDiff::ComplexDiff(const casa::Complex &in) : itsValue(in) {}
  
/// @brief construct a real constant
/// @param[in] in input real value
ComplexDiff::ComplexDiff(double in) : itsValue(in,0.) {}
  
/// @brief construct a complex parameter
/// @details This variant of the constructor fills appropriate 
/// derivatives with 1, and the value buffer with the given value 
/// (i.e. the object is the parameter itself). Parameter is assumed to be 
/// complex (i.e. cross-terms will be tracked).
/// @param[in] name parameter name
/// @param[in] in a reference to input complex value
ComplexDiff::ComplexDiff(const std::string &name, const casa::Complex &in) :
              itsValue(in) 
{
  itsDerivRe[name] = casa::Complex(1.,0.);
  itsDerivIm[name] = casa::Complex(0.,1.);
}
  
/// @brief construct a real parameter
/// @details This variant of the constructor fills appropriate 
/// derivative with 1, and the value buffer with the given value (i.e. the 
/// object will represent the parameter itself). Parameter is assumed to 
/// be real (i.e. no cross-terms will be tracked).
/// @param[in] name parameter name
/// @param[in] in input real value
ComplexDiff::ComplexDiff(const std::string &name, double in) : 
              itsValue(in,0.) 
{
  itsDerivRe[name] = casa::Complex(1.,0.);
} 
  
/// @brief obtain value
/// @return value of the function associated with this object
casa::Complex ComplexDiff::value() const throw()
{
  return itsValue;
}
  
/// @brief obtain derivatives by real part of the parameter
/// @param[in] name parameter name
/// @return value of the derivative by real part of the given parameter
casa::Complex ComplexDiff::derivRe(const std::string &name) const
{
  std::map<std::string, casa::Complex>::const_iterator ci = itsDerivRe.find(name);
  CONRADDEBUGASSERT(ci!=itsDerivRe.end());
  return ci->second;
}
  
/// @brief obtain derivatives by imaginary part of the parameter
/// @param[in] name parameter name
/// @return value of the derivative by imaginary part of the given parameter
casa::Complex ComplexDiff::derivIm(const std::string &name) const
{
  std::map<std::string, casa::Complex>::const_iterator ci = itsDerivIm.find(name);
  CONRADDEBUGASSERT(ci!=itsDerivIm.end());
  return ci->second;
}

  
/// @brief add up another autodifferentiator
/// @param[in] other autodifferentiator to add up
void ComplexDiff::operator+=(const ComplexDiff &other)
{
  // process real parts
  for (std::map<std::string, casa::Complex>::const_iterator ci = 
       other.itsDerivRe.begin(); ci!=other.itsDerivRe.end(); ++ci) {

       std::map<std::string, casa::Complex>::iterator it = 
                    itsDerivRe.find(ci->first);
       if (it != itsDerivRe.end()) {
           // this is a known parameter
           it->second += ci->second;
       } else {
           // this is a brand new parameter
           itsDerivRe.insert(*ci);
       }
  }

  // porcess imaginary parts
  for (std::map<std::string, casa::Complex>::const_iterator ci = 
       other.itsDerivIm.begin(); ci!=other.itsDerivIm.end(); ++ci) {

       std::map<std::string, casa::Complex>::iterator it = 
                    itsDerivIm.find(ci->first);
       if (it != itsDerivIm.end()) {
           // this is a known parameter
           it->second += ci->second;
       } else {
           // this is a brand new parameter
           itsDerivIm.insert(*ci);
       }
  }
  // process value
  itsValue+=other.itsValue;
}
