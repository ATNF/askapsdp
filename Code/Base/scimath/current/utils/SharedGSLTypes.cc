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

// own includes
#include "utils/SharedGSLTypes.h"

namespace askap {

namespace utility {


// CustomGSLDeleter<gsl_vector>

/// @brief this method frees the object
/// @param[in] obj pointer to gsl vector
void CustomGSLDeleter<gsl_vector>::operator()(gsl_vector *obj) const 
{
  gsl_vector_free(obj);
}

// CustomGSLDeleter<gsl_matrix>

/// @brief this method frees the object
/// @param[in] obj pointer to gsl matrix
void CustomGSLDeleter<gsl_matrix>::operator()(gsl_matrix *obj) const 
{
  gsl_matrix_free(obj);
}


} // namespace utility

} // namespace askap

