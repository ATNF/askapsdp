/// @file
/// @brief Generic interface to normal equations
/// @details There are two kinds of normal equations currently supported. The
/// first one is a generic case, where the full normal matrix is retained. It
/// is used for calibration. The second one is intended for imaging, where we
/// can't afford to keep the whole normal matrix. In this approach, the matrix
/// is approximated by a sum of diagonal and shift invariant matrices. This
/// interface is directly applicable to the generic case. However, it seems 
/// worth while to implement the appropriate generic methods in the approximated 
/// case as well. This will allow to do some tests with the full matrix for
/// small images, where we can afford such calculations.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_NORMAL_EQUATIONS_H
#define I_NORMAL_EQUATIONS_H

// own includes
#include <fitting/ISerializable.h>

// boost includes
#include <boost/shared_ptr.hpp>

namespace conrad {

namespace scimath {

/// @brief Generic interface to normal equations
/// @details There are two kinds of normal equations currently supported. The
/// first one is a generic case, where the full normal matrix is retained. It
/// is used for calibration. The second one is intended for imaging, where we
/// can't afford to keep the whole normal matrix. In this approach, the matrix
/// is approximated by a sum of diagonal and shift invariant matrices. This
/// interface is directly applicable to the generic case. However, it seems 
/// worth while to implement the appropriate generic methods in the approximated 
/// case as well. This will allow to do some tests with the full matrix for
/// small images, where we can afford such calculations.
struct INormalEquations : public ISerializable {

  /// @brief Shared pointer definition
  typedef boost::shared_ptr<INormalEquations> ShPtr;
      
  /// @brief Clone this into a shared pointer
  /// @details "Virtual constructor" - creates a copy of this object. Derived
  /// classes must override this method to instantiate the object of a proper 
  /// type.
  virtual INormalEquations::ShPtr clone() const = 0;

  /// @brief reset the normal equation object
  /// @details After a call to this method the object has the same pristine
  /// state as immediately after creation with the default constructor
  virtual void reset() = 0;

  /// @brief Merge these normal equations with another
  /// @details Combining two normal equations depends on the actual class type
  /// (different work is required for a full matrix and for an approximation).
  /// This method must be overriden in the derived classes for correct 
  /// implementation. 
  /// This means that we just add
  /// @param[in] src an object to get the normal equations from
  virtual void merge(const INormalEquations& src) = 0;
  
  /// @brief normal equations for given parameters
  /// @details In the current framework, parameters are essentially 
  /// vectors, not scalars. Each element of such vector is treated
  /// independently (but only vector as a whole can be fixed). As a 
  /// result the element of the normal matrix is another matrix for
  /// all non-scalar parameters. For scalar parameters each such
  /// matrix has a shape of [1,1].
  /// @param[in] par1 the name of the first parameter
  /// @param[in] par2 the name of the second parameter
  /// @return one element of the sparse normal matrix (a dense matrix)
  virtual const casa::Matrix<double>& normalMatrix(const std::string &par1, 
                        const std::string &par2) const = 0;
  
  /// @brief data vector for a given parameter
  /// @details In the current framework, parameters are essentially 
  /// vectors, not scalars. Each element of such vector is treated
  /// independently (but only vector as a whole can be fixed). As a 
  /// result any element of the normal matrix as well as an element of the
  /// data vector are, in general, matrices, not scalar. For the scalar 
  /// parameter each element of data vector is a vector of unit length.
  /// @param[in] par the name of the parameter of interest
  /// @return one element of the sparse data vector (a dense vector)     
  virtual const casa::Vector<double>& dataVector(const std::string &par) const = 0;
  
};

} // namespace scimath

} // namespace conrad

#endif // #ifndef I_NORMAL_EQUATIONS_H
