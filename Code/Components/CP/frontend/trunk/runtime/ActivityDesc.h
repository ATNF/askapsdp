/// @file ActivityDesc.h
///
/// @copyright (c) 2009 CSIRO
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

#ifndef ASKAP_CP_ACTIVITYDESC_H
#define ASKAP_CP_ACTIVITYDESC_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "Common/ParameterSet.h"

namespace askap {
    namespace cp {

        class ActivityDesc
        {
            public:
                /// @brief Constructor.
                ActivityDesc();

                /// @brief Destructor.
                virtual ~ActivityDesc();

                void setRuntime(const std::string& runtime);
                void setType(const std::string& type);
                void setName(const std::string& name);
                unsigned int addInPortMapping(const std::string& stream);
                unsigned int addOutPortMapping(const std::string& stream);
                void setParset(const LOFAR::ParameterSet& parset);

                std::string getRuntime(void) const;
                std::string getType(void) const;
                std::string getName(void) const;
                unsigned int getNumInPorts(void) const;
                unsigned int getNumOutPorts(void) const;
                std::string getPortInPortMapping(unsigned int port) const;
                std::string getPortOutPortMapping(unsigned int port) const;
                LOFAR::ParameterSet getParset(void) const;

            private:
                std::string itsRuntime;
                std::string itsType;
                std::string itsName;
                std::vector<std::string> itsInPorts;
                std::vector<std::string> itsOutPorts;
                LOFAR::ParameterSet itsParset;
        };
    };
};

#endif
