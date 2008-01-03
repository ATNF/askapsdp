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

#ifndef COMPLEX_DIFF_H
#define COMPLEX_DIFF_H

// casa includes
#include <casa/BasicSL/Complex.h>

// std includes
#include <map>
#include <string>

namespace conrad {

namespace scimath {

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
struct ComplexDiff {
  /// @brief default constructor -  a constant (0.,0.)
  ComplexDiff();
  
  /// @brief construct a complex constant
  /// @param[in] in a reference to input complex value
  ComplexDiff(const casa::Complex &in);
  
  /// @brief construct a real constant
  /// @param[in] in input real value
  ComplexDiff(double in);
  
  /// @brief construct a complex parameter
  /// @details This variant of the constructor fills appropriate 
  /// derivatives with 1, and the value buffer with the given value 
  /// (i.e. the object is the parameter itself). Parameter is assumed to be 
  //// complex (i.e. cross-terms will be tracked).
  /// @param[in] name parameter name
  /// @param[in] in a reference to input complex value
  ComplexDiff(const std::string &name, const casa::Complex &in); 
  
  /// @brief construct a real parameter
  /// @details This variant of the constructor fills appropriate 
  /// derivative with 1, and the value buffer with the given value (i.e. the 
  /// object will represent the parameter itself). Parameter is assumed to 
  /// be real (i.e. no cross-terms will be tracked).
  /// @param[in] name parameter name
  /// @param[in] in input real value
  ComplexDiff(const std::string &name, double in); 
  
  /// @brief obtain value
  /// @return value of the function associated with this object
  casa::Complex value() const throw();
  
  /// @brief obtain derivatives by real part of the parameter
  /// @param[in] name parameter name
  /// @return value of the derivative by real part of the given parameter
  casa::Complex derivRe(const std::string &name) const;
  
  /// @brief obtain derivatives by imaginary part of the parameter
  /// @param[in] name parameter name
  /// @return value of the derivative by imaginary part of the given parameter
  casa::Complex derivIm(const std::string &name) const;
  
  /// @brief add up another autodifferentiator
  /// @param[in] other autodifferentiator to add up
  void operator+=(const ComplexDiff &other);
  
  /// @brief form a sum of two parts 
  //friend ComplexDiff operator+(const ComplexDiff &in1, constComplexDiff &in2);
   
protected:  

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
  template<typename Op> void binaryOperationInSitu(Op &operation,
              std::map<std::string, casa::Complex> &thisDer,
              const std::map<std::string, casa::Complex> &otherDer,
              const casa::Complex &otherVal) const;

  /// @brief helper method to perform in situ addition
  /// @details It is used in conjunction with binaryOperationsInSitu
  /// @param[in] value1 a const reference to the value of the first operand
  /// @param[in] derivative1 a non-const reference to derivative of the 
  ///            first operand
  /// @param[in] value2 a const reference to the value of the second operand
  /// @param[in] derivative2 a const reference to derivative of the second operand
  /// @note parameters value1 and value2 are not used
  void static additionInSitu(const casa::Complex &value1, casa::Complex &derivative1,
                   const casa::Complex &value2, const casa::Complex &derivative2);
  
private:
  
  /// @brief derivatives by real part of the parameters
  std::map<std::string, casa::Complex> itsDerivRe;
  
  /// @brief derivatives by imaginary part of the parameters
  /// @note If some parameter is conceptually real, there may be no
  /// entry in this map for it at all.
  std::map<std::string, casa::Complex> itsDerivIm;
  
  /// @brief the value of the function represented by this differentiator
  casa::Complex itsValue;
};

} // namespace scimath

} // namespace conrad

#endif // #ifndef COMPLEX_DIFF_H