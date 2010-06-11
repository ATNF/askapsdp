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
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHDESIGNMATRIX_H
#define SCIMATHDESIGNMATRIX_H

#include <map>
#include <vector>
#include <set>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>

#include <fitting/Params.h>
#include <fitting/ComplexDiff.h>
#include <fitting/ComplexDiffMatrix.h>

namespace askap {
  
namespace scimath {



/// Format of A matrix
typedef std::vector<casa::Matrix<casa::Double> > DMAMatrix;

/// Format of B vector
typedef std::vector<casa::Vector<casa::Double> > DMBVector;
    
/// Format of weights
typedef std::vector<casa::Vector<casa::Double> > DMWeight;


/// @brief Hold the design matrix
class DesignMatrix {
public:

  /// Define a design matrix
  DesignMatrix();
  //// @param ip Parameters
  //explicit DesignMatrix(const Params& ip);

  /// @brief Copy constructor
  /// @param[in] dm another design matrix
  DesignMatrix(const DesignMatrix& dm);

  /// @brief Assignment operator
  /// @param[in] dm another design matrix
  DesignMatrix& operator=(const DesignMatrix& dm);

  /// @brief destructor
  ~DesignMatrix();

  /// @brief Merge this design matrix with another
  ///
  /// Merging means that we just need to append on the data axis
  /// @param[in] other Other design matrix
  void merge(const DesignMatrix& other);

  /// @brief Add the derivative of the data with respect to dof of the named parameter
  /// @param[in] name Name of parameter
  /// @param[in] deriv Derivative
  void addDerivative(const string& name, const casa::Matrix<casa::Double>& deriv);

  /// @brief Add the residual constraint
  /// @param[in] residual Residual vector
  /// @param[in] weight Weight vector
  void addResidual(const casa::Vector<casa::Double>& residual, const casa::Vector<double>& weight);

  /// @brief add derivatives and residual constraint
  /// @details This method extracts all information about derivatives and
  /// model values from ComplexDiffMatrix as well as the name of all parameters
  /// involved. Other arguments of the method are the data matrix conforming
  /// with the ComplexDiffMatrix and a matrix of weights.
  /// @param[in] cdm a ComplexDiffMatrix defining derivatives and model values
  /// @param[in] measured a matrix with measured data
  /// @param[in] weights a matrix with weights
  void addModel(const ComplexDiffMatrix &cdm, const casa::Matrix<casa::Complex> &measured, 
                const casa::Matrix<double> &weights);
     

  /// @brief Reset to empty
  void reset();

  /// @brief obtain all parameter names
  /// @details This method builds on-demand and returns a set with parameter
  /// names this design matrix knows about. 
  /// @return a const reference to std::set
  const std::set<std::string> parameterNames() const;
                
  /// Return the list of named design matrix terms
  const DMAMatrix& derivative(const string& name) const;

  /// Return list of the residual vectors
  const DMBVector& residual() const;

  /// Return lists of the weight vector
  const DMWeight& weight() const;

  /// Return value of fit
  double fit() const;

  /// Return number of data constraints
  casa::uInt nData() const;

  /// Return number of parameters
  int nParameters() const;

  /// Shared pointer definition
  typedef boost::shared_ptr<DesignMatrix> ShPtr;

  /// Clone this into a shared pointer
  DesignMatrix::ShPtr clone() const;

  

private:
  
  /// Design Matrix = number of parameters x number of dof/parameter x number of data points
  /// The number of dof of parameters can vary from parameter to parameter
  mutable std::map<string, DMAMatrix > itsAMatrix;
  
  /// Residual matrix = number of data points
  DMBVector itsBVector;
  
  /// Weight = number of data points        
  DMWeight itsWeight;
  
  /// cache of parameter names
  mutable std::set<std::string> itsParameterNames;
  
  /// invalidity flag for the cache of parameter names
  mutable bool itsParameterNamesInvalid;
};

} // namespace scimath
} // namespace askap
#endif
