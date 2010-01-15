/// @file Activity.h
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

#ifndef ASKAP_CP_IACTIVITY_H
#define ASKAP_CP_IACTIVITY_H

// System includes
#include <string>

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "boost/thread.hpp"

namespace askap {
    namespace cp {

        class Activity
        {
            public:
                /// @brief Constructor.
                Activity();

                /// @brief Destructor.
                virtual ~Activity();

                virtual void start(void);
                virtual void stop(void);

                virtual std::string getName(void);
                virtual void setName(const std::string& name);

                virtual void attachInputPort(int port, const std::string& topic) = 0;
                virtual void attachOutputPort(int port, const std::string& topic) = 0;

                virtual void detachInputPort(int port) = 0;
                virtual void detachOutputPort(int port) = 0;

                typedef boost::shared_ptr<Activity> ShPtr;

            protected:
                virtual void run(void) = 0;
                bool stopRequested(void);

                boost::shared_ptr<boost::thread> itsThread;
                bool itsStopRequested;

                std::string itsName;
        };

    };
};

#endif
