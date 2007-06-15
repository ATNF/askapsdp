/// @file
///
/// NormalEquations: Hold the normal equations for parameters.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHNORMALEQUATIONS_H_
#define SCIMATHNORMALEQUATIONS_H_

#include <fitting/Params.h>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>

#include <boost/shared_ptr.hpp>

namespace conrad
{
  namespace scimath
  {

    class DesignMatrix;

    /// @brief Hold the normal equations 
    class NormalEquations
/// Fully general normal equations
    {
      public:

        NormalEquations() {};

/// Define the normal equations
/// @param ip Parameters
        NormalEquations(const Params& ip);

/// Copy constructor
        NormalEquations(const NormalEquations& normeq);

/// Assignment operator
        NormalEquations& operator=(const NormalEquations& normeq);

        virtual ~NormalEquations();

/// Construct the normal equations from the design matrix
/// @param dm Design matrix
        NormalEquations(const DesignMatrix& dm);

/// Add the design matrix to the normal equations
/// @param dm Design matrix
        void add(const DesignMatrix& dm);

/// Return the specified parameters (const)
        const Params& parameters() const;
        
/// Return the specified parameters (non-const)
        Params& parameters();

/// @param Store full normal matrix for a given parameter. 

/// This means
/// that the cross terms between parameters are excluded. However
/// the terms inside a parameter are retained.
/// @param name Name of parameter
/// @param normalmatrix Normal Matrix for this parameter
/// @param datavector Data vector for this parameter
        void add(const string& name, const casa::Matrix<double>& normalmatrix,
          const casa::Vector<double>& datavector);

/// @brief Store full normal matrix for a given parameter.
/// 
/// This means
/// that the cross terms between parameters are excluded. However
/// the terms inside a parameter are retained.
/// @param name Name of parameter
/// @param normalmatrix Normal Matrix for this parameter
/// @param datavector Data vector for this parameter
/// @param shape Shape of this parameter
        void add(const string& name, const casa::Matrix<double>& normalmatrix,
          const casa::Vector<double>& datavector,
          const casa::IPosition& shape);

/// @brief Store slice of the normal matrix for a given parameter. 
/// 
/// This means
/// that the cross terms between parameters are excluded and only
/// a slice of the normal matrix is retained.
/// @param name Name of parameter
/// @param normalmatrixslice Slice of normal matrix for this parameter
/// @param normalmatrixdiagonal Diagonal of normal matrix for this parameter
/// @param datavector Data vector for this parameter
/// @param shape Shape of this parameter
/// @param reference Reference point for the slice
        void addSlice(const string& name,
          const casa::Vector<double>& normalmatrixslice,
          const casa::Vector<double>& normalmatrixdiagonal,
          const casa::Vector<double>& datavector,
          const casa::IPosition& shape,
          const casa::IPosition& reference);

/// @brief Store slice of the normal matrix for a given parameter. 
///
/// This means
/// that the cross terms between parameters are excluded and only
/// a slice of the normal matrix is retained.
/// @param name Name of parameter
/// @param normalmatrixslice Slice of normal matrix for this parameter
/// @param normalmatrixdiagonal Diagonal of normal matrix for this parameter
/// @param datavector Data vector for this parameter
/// @param reference Reference point for the slice
        void addSlice(const string& name,
          const casa::Vector<double>& normalmatrixslice,
          const casa::Vector<double>& normalmatrixdiagonal,
          const casa::Vector<double>& datavector,
          const casa::IPosition& reference);

/// @brief Store diagonal of the normal matrix for a given parameter. 
///
/// This means
/// that the cross terms between parameters are excluded and only
/// the diagonal inside a parameter is kept.
/// @param name Name of parameter
/// @param normalmatrix Normal Matrix for this parameter
/// @param datavector Data vector for this parameter
/// @param shape Shape of this parameter
        void addDiagonal(const string& name, 
          const casa::Vector<double>& normalmatrix,
          const casa::Vector<double>& datavector,
          const casa::IPosition& shape);

/// @brief Store diagonal of the normal matrix for a given parameter. 
///
/// This means
/// that the cross terms between parameters are excluded and only
/// the diagonal inside a parameter is kept.
/// @param name Name of parameter
/// @param normalmatrix Normal Matrix for this parameter
/// @param datavector Data vector for this parameter
        void addDiagonal(const string& name, const casa::Vector<double>& normalmatrix,
          const casa::Vector<double>& datavector);

/// @brief Merge these normal equations with another
///
/// This means that we just add
/// @param other Other normal equations
        void merge(const NormalEquations& other);

/// Return normal equations
        std::map<string, std::map<string, casa::Matrix<double> > >& normalMatrix() const;

/// Return normal equations slice
        std::map<string, casa::Vector<double> >& normalMatrixSlice() const;

/// Return normal equations diagonal
        std::map<string, casa::Vector<double> >& normalMatrixDiagonal() const;

/// Return data vector
        std::map<string, casa::Vector<double> >& dataVector() const;

/// Return shape
        std::map<string, casa::IPosition>& shape() const;

/// Return references
        std::map<string, casa::IPosition >& reference() const;

/// Reset to empty
        void reset();

/// Shared pointer definition
        typedef boost::shared_ptr<NormalEquations> ShPtr;

      protected:
      /// Parameters
        Params itsParams;
// Note that this is a very flexible format - it allows any of the
// approximations to be used
/// Normal matrices stored as a map or maps of Matrixes - it's really just a big
/// matrix.
        mutable std::map<string, std::map<string, casa::Matrix<double> > > itsNormalMatrix;
/// A slice through a specified plane
        mutable std::map<string, casa::Vector<double> > itsNormalMatrixSlice;
        /// The diagonal 
        mutable std::map<string, casa::Vector<double> > itsNormalMatrixDiagonal;
        /// The shape
        mutable std::map<string, casa::IPosition> itsShape;
        /// The Reference point for the slice
        mutable std::map<string, casa::IPosition> itsReference;
        /// The data vectors
        mutable std::map<string, casa::Vector<double> > itsDataVector;
    };

  }
}
#endif                                            /*NORMALEQUATIONS_H_*/
