/// @file
/// @brief Normal equations without any approximation
/// @details There are two kinds of normal equations currently supported. The
/// first one is a generic case, where the full normal matrix is retained. It
/// is used for calibration. The second one is intended for imaging, where we
/// can't afford to keep the whole normal matrix. In the latter approach, the 
/// matrix is approximated by a sum of diagonal and shift invariant matrices. 
/// This class represents the generic case, where no approximation to the normal
/// matrix is done.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef GENERIC_NORMAL_EQUATIONS_H
#define GENERIC_NORMAL_EQUATIONS_H

// casa includes
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Array.h>

// own includes
#include <fitting/INormalEquations.h>

// std includes
#include <map>
#include <string>

namespace conrad {

namespace scimath {

// forward declaration
class DesignMatrix;

/// @brief Normal equations without any approximation
/// @details There are two kinds of normal equations currently supported. The
/// first one is a generic case, where the full normal matrix is retained. It
/// is used for calibration. The second one is intended for imaging, where we
/// can't afford to keep the whole normal matrix. In this approach, the matrix
/// is approximated by a sum of diagonal and shift invariant matrices. This
/// class represents the generic case, where no approximation to the normal
/// matrix is done.
struct GenericNormalEquations : public INormalEquations {
      
  /// @brief a default constructor
  /// @details It creates an empty normal equations class
  GenericNormalEquations();
  
  /// @brief constructor from a design matrix
  /// @details This version of the constructor is equivalent to an
  /// empty constructor plus a call to add method with the given
  /// design matrix
  /// @param[in] dm Design matrix to use
  explicit GenericNormalEquations(const DesignMatrix& dm);
      
  /// @brief reset the normal equation object
  /// @detail After a call to this method the object has the same pristine
  /// state as immediately after creation with the default constructor
  virtual void reset();
          
  /// @brief Clone this into a shared pointer
  /// @details "Virtual constructor" - creates a copy of this object. Derived
  /// classes must override this method to instantiate the object of a proper 
  /// type.
  virtual GenericNormalEquations::ShPtr clone() const;

  /// @brief Merge these normal equations with another
  /// @details Combining two normal equations depends on the actual class type
  /// (different work is required for a full matrix and for an approximation).
  /// This method must be overriden in the derived classes for correct 
  /// implementation. 
  /// This means that we just add
  /// @param[in] src an object to get the normal equations from
  virtual void merge(const INormalEquations& src);
  
  /// @brief Add a design matrix to the normal equations
  /// @details This method computes the contribution to the normal matrix 
  /// using a given design matrix and adds it.
  /// @param[in] dm Design matrix to use
  void add(const DesignMatrix& dm);
  
  /// @brief add normal matrix for a given parameter
  /// @details This means that the cross terms between parameters 
  /// are excluded. However the terms inside a parameter are retained.
  /// @param[in] name Name of the parameter
  /// @param[in] normalmatrix Normal Matrix for this parameter
  /// @param[in] datavector Data vector for this parameter
  void add(const string& name, const casa::Matrix<double>& normalmatrix,
                               const casa::Vector<double>& datavector);
  
  /// @brief normal equations for given parameters
  /// @details In the current framework, parameters are essentially 
  /// vectors, not scalars. Each element of such vector is treated
  /// independently (but only vector as a whole can be fixed). As a 
  /// result any element of the normal matrix is another matrix for
  /// all non-scalar parameters. For scalar parameters each such
  /// matrix has a shape of [1,1].
  /// @param[in] par1 the name of the first parameter
  /// @param[in] par2 the name of the second parameter
  virtual const casa::Matrix<double>& normalMatrix(const std::string &par1, 
                        const std::string &par2) const;
  
  /// @brief data vector for a given parameter
  /// @details In the current framework, parameters are essentially 
  /// vectors, not scalars. Each element of such vector is treated
  /// independently (but only vector as a whole can be fixed). As a 
  /// result any element of the normal matrix as well as an element of the
  /// data vector are, in general, matrices, not scalar. For the scalar 
  /// parameter each element of data vector is a vector of unit length.
  /// @param[in] par the name of the parameter of interest
  const casa::Vector<double>& dataVector(const std::string &par) const;
  
  /// @brief write the object to a blob stream
  /// @param[in] os the output stream
  virtual LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream& os) const;

  /// @brief read the object from a blob stream
  /// @param[in] is the input stream
  /// @note Not sure whether the parameter should be made const or not 
  virtual LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream& is); 

protected:
  // @brief map of matrices (data element of each row map)
  typedef std::map<std::string, casa::Matrix<double> > MapOfMatrices;
  // @brief map of vectors (data vectors for all parameters)
  typedef std::map<std::string, casa::Vector<double> > MapOfVectors;

  /// @brief Add one parameter from another normal equations class
  /// @details This helper method is used in merging of two normal equations.
  /// It processes just one parameter.
  /// @param[in] par name of the parameter to copy
  /// @param[in] src an object to get the normal equations from
  /// @note This helper method works with instances of this class only (as
  /// only then it knows how the actual normal matrix is handled). One could
  /// have a general code which would work for every possible normal equation,
  /// but in some cases it would be very inefficient. Therefore, the decision
  /// has been made to throw an exception if incompatible operation is requested
  /// and add the code to handle this situation later, if it appears to be 
  /// necessary.
  void mergeParameter(const std::string &par, const GenericNormalEquations& src);
                       
private:
  
  /// @brief normal matrix
  /// @details Normal matrices stored as a map or maps of Matrixes - 
  /// it's really just a big matrix.
  std::map<string, MapOfMatrices> itsNormalMatrix;
  
  /// @brief the data vectors
  /// @details This parameter may eventually go a level up in the class
  /// hierarchy
  MapOfVectors itsDataVector;
  
};

} // namespace scimath

} // namespace conrad

#endif // #ifndef GENERIC_NORMAL_EQUATIONS_H
