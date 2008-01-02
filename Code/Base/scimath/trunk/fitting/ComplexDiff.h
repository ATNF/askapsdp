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