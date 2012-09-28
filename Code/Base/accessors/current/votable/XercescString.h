/// @file XMLUtils.h
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

#ifndef ASKAP_ACCESSORS_VOTABLE_XERCESCSTRING_H
#define ASKAP_ACCESSORS_VOTABLE_XERCESCSTRING_H

// System includes
#include <string>

// ASKAPsoft includes
#include "xercesc/util/XMLString.hpp"

namespace askap {
namespace accessors {

class XercescString {
    public:
        XercescString(const char* str);

        XercescString(const std::string& str);

        ~XercescString();

        operator const XMLCh* () const
        {
            return itsXMLCh;
        }

        operator const std::string () const
        {
            char* c = xercesc::XMLString::transcode(itsXMLCh);
            std::string s(c);
            xercesc::XMLString::release(&c);
            return s;
        }

    private:
        XMLCh* itsXMLCh;
};

}
}

#endif
