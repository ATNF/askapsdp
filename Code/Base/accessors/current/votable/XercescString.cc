/// @file XercescString.cc
///
/// @copyright (c) 2012 CSIRO
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
#include "XercescString.h"

// Include package level header file
#include "askap_accessors.h"

// ASKAPsoft includes
#include "xercesc/util/XMLString.hpp"

using namespace askap::accessors;

XercescString::XercescString(const char* str) : itsXMLCh(0)
{
    itsXMLCh = xercesc::XMLString::transcode(str);
}

XercescString::XercescString(const std::string& str) : itsXMLCh(0)
{
    itsXMLCh = xercesc::XMLString::transcode(str.c_str());
}

XercescString::~XercescString()
{
    if (itsXMLCh) {
        xercesc::XMLString::release(&itsXMLCh);
    }
}
