/// @file JonesJTerm.cc
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

// Include own header file first
#include "JonesJTerm.h"

// Include package level header file
#include "askap_cpdataservices.h"

// System includes

// ASKAPsoft includes
#include "casa/aipstype.h"

// Local package includes

// Using
using namespace askap::cp::caldataservice;

JonesJTerm::JonesJTerm()
        : itsG1(-1.0), itsG1Valid(false),
        itsG2(-1.0), itsG2Valid(false)
{
}

JonesJTerm::JonesJTerm(const casa::DComplex& g1,
                       const casa::Bool g1Valid,
                       const casa::DComplex& g2,
                       const casa::Bool g2Valid)
        : itsG1(g1), itsG1Valid(g1Valid),
        itsG2(g2), itsG2Valid(g2Valid)
{
}

casa::DComplex JonesJTerm::g1(void) const
{
    return itsG1;
}

casa::Bool JonesJTerm::g1IsValid(void) const
{
    return itsG1Valid;
}

casa::DComplex JonesJTerm::g2(void) const
{
    return itsG2;
}

casa::Bool JonesJTerm::g2IsValid(void) const
{
    return itsG2Valid;
}
