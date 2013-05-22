/// @file MonitoringSingleton.h
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

#ifndef ASKAP_CP_INGEST_MONITORINGSINGLETON_H
#define ASKAP_CP_INGEST_MONITORINGSINGLETON_H

// System includes

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"

// Local package includes
#include "configuration/Configuration.h" // Includes all configuration attributes too

namespace askap {
    namespace cp {
        namespace ingest {

            class MonitoringSingleton {
                public:
                    /// @brief Obtain the singleton instance of the monitoring
                    /// data interface singleton.
                    ///
                    /// @return the singleton instance.
                    static MonitoringSingleton* instance(void);

                    /// Initialise the singleton instance
                    static void init(const Configuration& config);

                    /// Destroy the singleton instance
                    static void destroy();

                    /// @brief Destructor.
                    ~MonitoringSingleton();

                private:

                    /// @brief Constructor.
                    MonitoringSingleton(const Configuration& config);

                    const Configuration itsConfig;

                    static MonitoringSingleton* itsInstance;

                    // No support for assignment
                    MonitoringSingleton& operator=(const MonitoringSingleton& rhs);

                    // No support for copy constructor
                    MonitoringSingleton(const MonitoringSingleton& src);
            };

        }
    }
}

#endif
