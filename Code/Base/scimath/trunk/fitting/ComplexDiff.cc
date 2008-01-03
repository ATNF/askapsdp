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

#include <iostream>

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

/// @brief perform an arbitrary binary operation on derivatives
/// @details This method can be used to implement operations like +=, *=, etc.
/// The common point is that the result is stored in this class, while its
/// previous value is lost. Op is a type, which knows how to do the operation.
/// It should have the operator() accepting 4 parameters: value1, derivative1,
/// value2 and derivative2 (all parameters are complex). It doesn't matter at
/// this stage whether the derivative is by real or imaginary part as the
/// formulae are always the same. A number of optimizations are possible here,
/// e.g. special handling of the cases where some parameters are undefined
/// instead of always computing the full formula. It can be implemented later,
/// if found necessary. Currently Op::operator() will be called with 
/// the appropriate derivative set to zero. 
/// @param[in] operation type performing actual operation
/// @param[inout] thisDer this operand's derivatives
/// @param[in] otherDer a second operand's derivatives 
/// @param[in] otherVal a second operand's value
template<typename Op> 
void ComplexDiff::binaryOperationInSitu(Op &operation,
            std::map<std::string, casa::Complex> &thisDer, 
            const std::map<std::string, casa::Complex> &otherDer,
            const casa::Complex &otherVal) const
{ 
  for (std::map<std::string, casa::Complex>::const_iterator otherIt = 
       otherDer.begin(); otherIt!=otherDer.end(); ++otherIt) {
       
       const std::map<std::string, casa::Complex>::iterator thisIt = 
                  thisDer.insert(std::pair<std::string, casa::Complex>(
                              otherIt->first, casa::Complex(0.,0.))).first;
                              
       operation(itsValue, thisIt->second, otherVal, otherIt->second);
  }  
  
  // now account for this class parameters which are absent in the second 
  // operand
  for (std::map<std::string, casa::Complex>::iterator thisIt = thisDer.begin();
       thisIt != thisDer.end(); ++thisIt) {
       
       const std::map<std::string, casa::Complex>::const_iterator otherIt = 
                                 otherDer.find(thisIt->first);
       if (otherIt == otherDer.end()) {
           operation(itsValue,thisIt->second, otherVal, casa::Complex(0.,0.));
       }                         
  }
}

/// @brief helper method to perform in situ addition
/// @details It is used in conjunction with binaryOperationsInSitu
/// @param[in] derivative1 a non-const reference to derivative of the 
///            first operand
/// @param[in] derivative2 a const reference to derivative of the second operand
void ComplexDiff::additionInSitu(const casa::Complex &, 
                   casa::Complex &derivative1,
                   const casa::Complex &, const casa::Complex &derivative2)
{  
  derivative1 += derivative2;
}
  
/// @brief add up another autodifferentiator
/// @param[in] other autodifferentiator to add up
void ComplexDiff::operator+=(const ComplexDiff &other)
{
  // process derivatives
  binaryOperationInSitu(additionInSitu,itsDerivRe,other.itsDerivRe,other.itsValue);
  binaryOperationInSitu(additionInSitu,itsDerivIm,other.itsDerivIm,other.itsValue);
  // process value
  itsValue+=other.itsValue;
}
