/// @file LibraryWrapper.h
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

#ifndef ASKAP_CP_EVENTCHANNEL_LIBRARY_WRAPPER_H
#define ASKAP_CP_EVENTCHANNEL_LIBRARY_WRAPPER_H

// ASKAPsoft includes
#include "boost/thread/mutex.hpp"

namespace askap {
namespace cp {
namespace channels {

    /// Instantiate this before using the ActiveMQ C++ library. The constructor
    /// initialises the library if the reference count is zero, and increments
    /// the reference count.
    /// Conversly, the destructor decrements the reference count and shuts down
    /// the library if the reference count is zero.
    class LibraryWrapper {
            public:

                /// Constructor.
                LibraryWrapper();

                /// Destructor.
                ~LibraryWrapper();

            private:
                // Reference count
                static unsigned int theirReferenceCount;

                // Mutex used to serialise access to this library
                static boost::mutex theirMutex;
        };

};
};
};

#endif
