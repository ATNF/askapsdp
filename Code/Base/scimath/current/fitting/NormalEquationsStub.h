/// @file
/// @brief Stubbed normal equations
/// @details It is handy to have a stubbed version of normal equations which does nothing, but 
/// can be serialized, shipped around and merged with the same empty normal equations. This allows
/// to reuse the existing framework to parallelize measurement equations based algorithms if no
/// solution to normal equations is required (i.e. continuum subtraction). Receiving Normal Equation
/// acts as a barrier in this case to synchronize the parallel streams.
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
///

#ifndef NORMAL_EQUATIONS_STUB_H
#define NORMAL_EQUATIONS_STUB_H

// own includes
#include <fitting/INormalEquations.h>
#include <casa/Arrays/Matrix.h>

// boost includes
#include <boost/shared_ptr.hpp>

// std includes
#include <vector>
#include <string>

namespace askap {

namespace scimath {

/// @brief Stubbed normal equations
/// @details It is handy to have a stubbed version of normal equations which does nothing, but 
/// can be serialized, shipped around and merged with the same empty normal equations. This allows
/// to reuse the existing framework to parallelize measurement equations based algorithms if no
/// solution to normal equations is required (i.e. continuum subtraction). Receiving Normal Equation
/// acts as a barrier in this case to synchronize the parallel streams.
struct NormalEquationsStub : public INormalEquations {

  /// @brief Shared pointer definition
  typedef boost::shared_ptr<NormalEquationsStub> ShPtr;
      
  /// @brief Clone this into a shared pointer
  /// @details "Virtual constructor" - creates a copy of this object. Derived
  /// classes must override this method to instantiate the object of a proper 
  /// type.
  /// @return shared pointer to the clone
  virtual INormalEquations::ShPtr clone() const 
     { return  INormalEquations::ShPtr(new NormalEquationsStub(*this)); }

  /// @brief reset the normal equation object
  /// @details After a call to this method the object has the same pristine
  /// state as immediately after creation with the default constructor
  virtual void reset() {}

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
  
  
  /// @brief obtain all parameters dealt with by these normal equations
  /// @details Normal equations provide constraints for a number of 
  /// parameters (i.e. unknowns of these equations). This method returns
  /// a vector with the string names of all parameters mentioned in the
  /// normal equations represented by the given object.
  /// @return a vector listing the names of all parameters (unknowns of these equations)
  virtual std::vector<std::string> unknowns() const 
     {  return std::vector<std::string>(); }

  /// @brief write the object to a blob stream
  /// @param[in] os the output stream
  virtual void writeToBlob(LOFAR::BlobOStream& os) const;

  /// @brief read the object from a blob stream
  /// @param[in] is the input stream
  /// @note Not sure whether the parameter should be made const or not 
  virtual void readFromBlob(LOFAR::BlobIStream& is); 

};

} // namespace scimath

} // namespace askap

#endif // #ifndef NORMAL_EQUATIONS_STUB_H
