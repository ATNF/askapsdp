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
#include <fitting/INormalEquations.h>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>

#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>

#include <boost/shared_ptr.hpp>

namespace conrad
{
  namespace scimath
  {
    
    class DesignMatrix;
    
    /// @brief Hold the normal equations 
    class NormalEquations : public INormalEquations
    /// Fully general normal equations
    {
    public:
      
      NormalEquations();
      
      /// @brief Construct for the specified parameters
      ///
      /// Initialisation does not allocate much memory.
      /// @param ip Parameters
      NormalEquations(const Params& ip);
      
      /// Copy constructor
      NormalEquations(const NormalEquations& normeq);
      
      /// Assignment operator
      NormalEquations& operator=(const NormalEquations& normeq);
      
      virtual ~NormalEquations();
      
      /// Construct the normal equations from the design matrix
      /// @param ip parameters (design matrix no longer holds them)
      /// @param dm Design matrix
      NormalEquations(const Params &ip, const DesignMatrix& dm);
      
      /// Add the design matrix to the normal equations
      /// @param dm Design matrix
      void add(const DesignMatrix& dm);
      
      /// Return the specified parameters (const)
      const Params& parameters() const;
            
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
      /// @param normalmatrixdiagonal Diagonal of normal matrix for
      ///        this parameter
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
      /// @param normalmatrixdiagonal Diagonal of normal matrix for
      ///        this parameter
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
      void addDiagonal(const string& name, 
                       const casa::Vector<double>& normalmatrix,
                       const casa::Vector<double>& datavector);
      
      /// @brief Merge these normal equations with another
      /// @details Combining two normal equations depends on the actual class type
      /// (different work is required for a full matrix and for an approximation).
      /// This method must be overriden in the derived classes for correct 
      /// implementation. 
      /// This means that we just add
      /// @param[in] src an object to get the normal equations from
      virtual void merge(const INormalEquations& src);
      
      
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
                        const std::string &par2) const;
  
       /// @brief data vector for a given parameter
       /// @details In the current framework, parameters are essentially 
       /// vectors, not scalars. Each element of such vector is treated
       /// independently (but only vector as a whole can be fixed). As a 
       /// result any element of the normal matrix as well as an element of the
       /// data vector are, in general, matrices, not scalar. For the scalar 
       /// parameter each element of data vector is a vector of unit length.
       /// @param[in] par the name of the parameter of interest
       /// @return one element of the sparse data vector (a dense vector)
       virtual const casa::Vector<double>& dataVector(const std::string &par) const;
      
      
      /// Return normal equations slice
      const std::map<string, casa::Vector<double> >& normalMatrixSlice() const;
      
      /// Return normal equations diagonal
      const std::map<string, casa::Vector<double> >& normalMatrixDiagonal() const;
      
      /// Return data vector
      const std::map<string, casa::Vector<double> >& dataVector() const;
      
      /// Return shape
      const std::map<string, casa::IPosition>& shape() const;
      
      /// Return references
      const std::map<string, casa::IPosition >& reference() const;
      
      /// Reset to empty
      virtual void reset();
      
      /// Shared pointer definition
      typedef boost::shared_ptr<NormalEquations> ShPtr;
      
      /// Clone this into a shared pointer
      virtual INormalEquations::ShPtr clone() const;
      
      /// @brief write the object to a blob stream
      /// @param[in] os the output stream
      virtual void writeToBlob(LOFAR::BlobOStream& os) const;

      /// @brief read the object from a blob stream
      /// @param[in] is the input stream
      /// @note Not sure whether the parameter should be made const or not 
      virtual void readFromBlob(LOFAR::BlobIStream& is); 
              
    protected:
      /// Parameters
      Params::ShPtr itsParams;
      // Note that this is a very flexible format - it allows any of the
      // approximations to be used
      /// Normal matrices stored as a map or maps of Matrixes - 
      /// it's really just a big matrix.
      std::map<string, std::map<string, casa::Matrix<double> > > itsNormalMatrix;
      /// A slice through a specified plane
      std::map<string, casa::Vector<double> > itsNormalMatrixSlice;
      /// The diagonal 
      std::map<string, casa::Vector<double> > itsNormalMatrixDiagonal;
      /// The shape
      std::map<string, casa::IPosition> itsShape;
      /// The Reference point for the slice
      std::map<string, casa::IPosition> itsReference;
      /// The data vectors
      std::map<string, casa::Vector<double> > itsDataVector;
    };
    
  }
}
#endif
