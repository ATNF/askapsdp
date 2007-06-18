/// @file
///
/// DesignMatrix: Hold the design matrix for parameters. If the relationship
/// between data B and model X is B=AX then A is the design matrix.
/// This is usually too large to do much with but it can be used as a
/// convenient way to build up the normal equations. In fact this is
/// currently the only use for DesignMatrix.
///
/// We also store the B vector using this class.
///
/// The parameters are identified by strings and so the storage
/// for the design matrix is a map of strings to a std::vector
/// of casa::Matrix's. There is (currently) not much checking
/// of the consistency of ordering - it is assumed that the
/// std::vector elements march in order. Since the only way
/// to fill them is via the add* functions, this should be ok.
///
/// The parameters are intrinisically casa::Array's but we convert them
/// to casa::Vector's to avoid indexing hell.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHDESIGNMATRIX_H
#define SCIMATHDESIGNMATRIX_H

#include <map>
#include <vector>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>

#include <fitting/Params.h>

namespace conrad
{
  namespace scimath
  {

    class NormalEquations;

/// Format of A matrix
    typedef std::vector<casa::Matrix<casa::Double> > DMAMatrix;
    /// Format of B vector
    typedef std::vector<casa::Vector<casa::Double> > DMBVector;
    /// Format of weights
    typedef std::vector<casa::Vector<casa::Double> > DMWeight;

    /// @brief Hold the design matrix
    class DesignMatrix
    {
      public:

/// Define a design matrix
/// @param ip Parameters
        explicit DesignMatrix(const Params& ip);

/// Copy constructor
        DesignMatrix(const DesignMatrix& dm);

/// Assignment operator
        DesignMatrix& operator=(const DesignMatrix& dm);

        virtual ~DesignMatrix();

/// @brief Merge this design matrix with another
///
/// Merging means that we just need to append on the data axis
/// @param other Other design matrix
        void merge(const DesignMatrix& other);

/// @brief Add the derivative of the data with respect to dof of the named parameter
/// @param name Name of parameter
/// @param deriv Derivative
        void addDerivative(const string& name, const casa::Matrix<casa::Double>& deriv);

/// @brief Add the residual constraint
/// @param residual Residual vector
/// @param weight Weight vector
        void addResidual(const casa::Vector<casa::Double>& residual, const casa::Vector<double>& weight);

/// Reset to empty
        void reset();

/// Return the parameters used by this equation (const)
        const Params& parameters() const;

/// Return the parameters used by this equation (non-const)
        Params& parameters();

/// Return the list of named design matrix terms
        DMAMatrix derivative(const string& name) const;

/// Return list of the residual vectors
        DMBVector residual() const;

/// Return lists of the weight vector
        DMWeight weight() const;

/// Return names of parameters
        vector<string> names() const;

/// Return value of fit
        double fit() const;

/// Return number of data constraints
        uint nData() const;

/// Return number of parameters
        uint nParameters() const;

/// Shared pointer definition
        typedef boost::shared_ptr<DesignMatrix> ShPtr;

/// Clone this into a shared pointer
        virtual DesignMatrix::ShPtr clone();

        friend class NormalEquations;

      private:
/// Parameters used in this design matrix
        Params::ShPtr itsParams;
/// Design Matrix = number of parameters x number of dof/parameter x number of data points
/// The number of dof of parameters can vary from parameter to parameter
        mutable std::map<string, DMAMatrix > itsAMatrix;
/// Residual matrix = number of data points
        mutable DMBVector itsBVector;
/// Weight = number of data points        
        mutable DMWeight itsWeight;
    };

  }
}
#endif
