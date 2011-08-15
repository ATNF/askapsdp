/// @file JonesDTerm.h
///
/// @copyright (c) 2011 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

#ifndef ASKAP_CP_CALDATASERVICE_JONESDTERM_H
#define ASKAP_CP_CALDATASERVICE_JONESDTERM_H

// ASKAPsoft includes
#include "casa/aipstype.h"
#include "casa/BasicSL/Complex.h"

namespace askap {
namespace accessors {

/// @brief JonesDTerm (Polarisation leakage)
/// @ingroup calibaccess
class JonesDTerm {

    public:
        /// @brief Constructor
        /// This default (no-args) constructor is needed by various containers,
        /// for instance to populate a vector or matrix with default values.
        JonesDTerm();

        /// @brief Constructor.
        /// @param[in] g12 
        /// @param[in] g21 
        JonesDTerm(const casa::Complex& d12,
                   const casa::Complex& d21);

        casa::Complex d12(void) const;

        casa::Complex d21(void) const;

    private:
        casa::Complex itsD12;
        casa::Complex itsD21;
};

};
};

#endif
