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


// own includes
#include <fitting/ComplexDiff.h>
#include <askap/AskapError.h>

#include <iostream>

using namespace askap;
using namespace askap::scimath;

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
  return ci!=itsDerivRe.end() ? ci->second : casa::Complex(0.,0.);
}
  
/// @brief obtain derivatives by imaginary part of the parameter
/// @param[in] name parameter name
/// @return value of the derivative by imaginary part of the given parameter
casa::Complex ComplexDiff::derivIm(const std::string &name) const
{
  std::map<std::string, casa::Complex>::const_iterator ci = itsDerivIm.find(name);
  return ci!=itsDerivIm.end()? ci->second : casa::Complex(0.,0.);
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

/// @brief a helper method to perform conformance checks
/// @details Some parameters may be conceptually real. In this case
/// ComplexDiff doesn't track derivatives by imaginary part. These
/// conformance checks are done during binary operations in debug
/// mode to ensure that a parameter doesn't change its real or
/// complex status implicitly.
/// @param[in] other another ComplexDiff object to check conformance of
///            parameters
/// @return true if parameters have the same type (dimension) for this
/// and another class
bool ComplexDiff::isConformant(const ComplexDiff &other) const
{
  // we basically need to check that all real parameters are also real in another
  // ComplexDiff, and all complex parameters are complex there too.
  for (parameter_iterator pit = utility::mapKeyBegin(itsDerivRe);
                     pit != utility::mapKeyEnd(itsDerivRe); ++pit) {
       if (other.itsDerivRe.find(*pit) != other.itsDerivRe.end()) {
           // this parameter is known to another ComplexDiff,
           // check conformance
           if (isReal(*pit) != other.isReal(*pit)) {
               // this parameter is non-conformant, hence check failed
               return false;
           }
       }
  }
  // all parameters are conformant if we reach this part of the code
  return true;
}


/// @brief perform an arbitrary unary operation on derivatives
/// @details This method can be used to implement operations like conjugation.
/// The common point is that the content of this class is updated. Note, 
/// however, that this method accepts a map of derivatives as its parameter.
/// This is done because we need to repeat the same operation for both
/// derivatives by real and imaginary part. Therefore, this method doesn't 
/// change the content of this class directly. It only happens when this method
/// is called, because this class' members are passed as non-const parameter.
/// This is why this method is made const-method.
/// Op is a type, which knows how to do the operation.
/// It should have the operator() accepting 2 parameters: value and derivative. 
/// It doesn't matter at this stage whether the derivative is by real or 
/// imaginary part as the formulae are always the same. 
/// @param[in] operation type performing actual operation
/// @param[inout] der operand's derivatives
template<typename Op> void ComplexDiff::unaryOperationInSitu(Op &operation,
              std::map<std::string, casa::Complex> &der) const
{
   for (std::map<std::string, casa::Complex>::iterator it = der.begin();
       it != der.end(); ++it) {
       operation(itsValue, it->second);
   }
}

  
/// @brief add up another autodifferentiator
/// @param[in] other autodifferentiator to add up
void ComplexDiff::operator+=(const ComplexDiff &other)
{
  // process derivatives
  binaryOperationInSitu(additionInSitu,other);
  // process value
  itsValue+=other.itsValue;
}

/// @brief multiply to another autodifferentiator
/// @param[in] other autodifferentiator to multiply this one to
void ComplexDiff::operator*=(const ComplexDiff &other)
{
  // process derivatives
  binaryOperationInSitu(multiplicationInSitu,other);
  // process value
  itsValue*=other.itsValue;
}

/// a helper class to multiply all derivatives by a given constant
struct ConstantMultiplier {
  /// @brief constructor
  /// @param[in] in a const reference to the constant
  ConstantMultiplier(const casa::Complex &in) : itsConstant(in) {}
  
  /// @brief multiply given derivative by the constant
  /// @param[in] der reference to a value of derivative to multiply
  inline void operator()(const casa::Complex&, casa::Complex& der) 
      { der *= itsConstant; }
private:
  const casa::Complex &itsConstant;
}; 

/// @brief multiply to a constant
/// @details Although this functionality is implemented by the method
/// working with another autodifferentiator (via implicit construction of
/// an autodifferentiator from a constant), having a separate method 
/// working with a constant is good from the performance point of view.
/// Otherwise, a search for matching parmeters will be done on each
/// multiplication.
void ComplexDiff::operator*=(const casa::Complex &other)
{
  ConstantMultiplier cm(other);
  unaryOperationInSitu(cm);
  itsValue *= other;
}

/// @brief perform complex conjugation in situ
void ComplexDiff::conjugate() 
{  
  unaryOperationInSitu(ComplexDiff::conjugationInSitu);
  itsValue = conj(itsValue);
}

