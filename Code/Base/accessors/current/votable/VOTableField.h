/// @file VOTableField.h
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

#ifndef ASKAP_CP_ACCESSORS_VOTABLE_VOTABLEFIELD_H
#define ASKAP_CP_ACCESSORS_VOTABLE_VOTABLEFIELD_H

// System includes
#include <string>

// ASKAPsoft includes
#include "xercesc/dom/DOM.hpp" // Includes all DOM

// Local package includes

namespace askap {
    namespace accessors {

        /// @brief TODO: Write documentation...
        class VOTableField {
            public:

                /// @brief Constructor
                VOTableField();

                void setDescription(const std::string& description);

                std::string getDescription() const;

                void setName(const std::string& name);

                std::string getName() const;

                void setID(const std::string& id);

                std::string getID() const;

                void setDatatype(const std::string& datatype);

                std::string getDatatype() const;

                void setArraysize(const std::string& arraysize);

                std::string getArraysize() const;

                void setUnit(const std::string& unit);

                std::string getUnit() const;

                void setUCD(const std::string& ucd);

                std::string getUCD() const;

                void setUType(const std::string& utype);

                std::string getUType() const;

                void setRef(const std::string& ref);

                std::string getRef() const;

                xercesc::DOMElement* toXmlElement(xercesc::DOMDocument& doc) const;

            private:

                std::string itsDescription;
                std::string itsName;
                std::string itsID;
                std::string itsDatatype;
                std::string itsArraysize;
                std::string itsUnit;
                std::string itsUCD;
                std::string itsUType;
                std::string itsRef;
        };

    }
}

#endif
