/// @file 
/// @brief shared pointers to gsl types
/// @details We use gsl for linear algebra routines. However, the interface is C-like and is based on
/// raw pointers. The file contains code to leverage on boost shared pointer and use it with gsl types
/// such as vector and matrix.
///
/// @copyright (c) 2008 CSIRO
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

#ifndef ASKAP_SCIMATH_SHARED_GSL_TYPES_H
#define ASKAP_SCIMATH_SHARED_GSL_TYPES_H

// boost includes
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>


// gsl includes
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

// own includes
#include "askap/AskapError.h"

namespace askap {

namespace utility {

// typedefs for shared pointers, we can add more here when needed

/// @brief shared pointer to gsl vector
/// @ingroup utils
typedef boost::shared_ptr<gsl_vector> SharedGSLVector;

/// @brief shared pointer to gsl matrix
/// @ingroup utils
typedef boost::shared_ptr<gsl_matrix> SharedGSLMatrix;

/// @brief custom deleter for gsl types
/// @details This is a custom deleter class to allow deallocation of
/// a gsl object. Exact operation should be present in specialised types
/// @ingroup utils
template<typename GSLType> struct CustomGSLDeleter {
    /// @brief this method frees the object
    /// @param[in] obj pointer to gsl object
    void operator()(GSLType *obj) const { BOOST_STATIC_ASSERT_MSG(sizeof(GSLType) == 0, 
                            "Generic version is not supposed to be used - specialise it for your type"); }
};

/// @brief specialised deleter for a gsl vector
/// @ingroup utils
template<> struct CustomGSLDeleter<gsl_vector> {
    /// @brief this method frees the object
    /// @param[in] obj pointer to gsl vector
    void operator()(gsl_vector *obj) const;
};

/// @brief specialised deleter for a gsl matrix
/// @ingroup utils
template<> struct CustomGSLDeleter<gsl_matrix> {
    /// @brief this method frees the object
    /// @param[in] obj pointer to gsl matrix
    void operator()(gsl_matrix *obj) const;
};


/// @brief templated helper method to wrap newly allocated gsl object 
/// @details This method processes a pointer to a gsl object and transfers the ownership (i.e. 
/// responsibility for deallocation to the boost shared pointer. Templates are used to automatically
/// deduce the object type and add an appropriate deleter. This method is not supposed to be used directly.
/// @note the pointer passed as a parameter is tested to be non-zero. Otherwise an exception is thrown.
/// @param[in] obj a pointer to gsl object
/// @return boost shared pointer to the object
template<typename GSLType>
inline boost::shared_ptr<GSLType> createGSLObject(GSLType *obj) 
   { ASKAPASSERT(obj); return boost::shared_ptr<GSLType>(obj, CustomGSLDeleter<GSLType>()); }


/// @brief helper method to allocate gsl vector
/// @details This method allocates a gsl vector of requested length and returns a
/// shared pointer. 
/// @param[in] size size of the vector
/// @return shared pointer to gsl vector
inline SharedGSLVector createGSLVector(size_t size) { return createGSLObject(gsl_vector_alloc(size));}


/// @brief helper method to allocate gsl matrix
/// @details This method allocates a gsl matrix of requested shape and returns a
/// shared pointer. 
/// @param[in] nrow number of rows
/// @param[in] ncol number of columns
/// @return shared pointer to gsl matrix
inline SharedGSLMatrix createGSLMatrix(size_t nrow, size_t ncol) { return createGSLObject(gsl_matrix_alloc(nrow,ncol));}

} // namespace utility

} // namespace askap

#endif // #ifndef ASKAP_SCIMATH_SHARED_GSL_TYPES_H

