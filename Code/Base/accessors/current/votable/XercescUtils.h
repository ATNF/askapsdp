/// @file XercescUtils.h
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

#ifndef ASKAP_ACCESSORS_VOTABLE_XERCESCUTILS_H
#define ASKAP_ACCESSORS_VOTABLE_XERCESCUTILS_H

// System includes
#include <string>

// ASKAPsoft includes
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/dom/DOMText.hpp"

namespace askap {
    namespace accessors {

        /// @brief A collection of general XML access utilities
        /// @ingroup votableaccess
        class XercescUtils {
            public:

                /// @brief Returns the string from a DOM Text element.
                static std::string getStringFromDOMText(const xercesc::DOMText& text);

                /// @brief Returns the value of an attibute associated with a
                /// given DOM element.
                static std::string getAttribute(const xercesc::DOMElement& element,
                        const std::string& key);

                /// @brief Returns a pointer to the first element contained by
                /// the "element" parameter, that has the tag matching name.
                static xercesc::DOMElement* getFirstElementByTagName(
                        const xercesc::DOMElement& element,
                        const std::string& name);

                /// @brief Returns the string from an element with the
                ///  tag DESCRIPTION.
                static std::string getDescription(const xercesc::DOMElement& element);

                /// Adds a text element child to a DOMElement
                static xercesc::DOMElement* addTextElement(xercesc::DOMElement& parent,
                        const std::string& tag, const std::string& value);
        };

    }
}

#endif
