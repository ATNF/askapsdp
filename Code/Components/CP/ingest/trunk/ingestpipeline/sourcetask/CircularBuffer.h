/// @file CircularBuffer.h
///
/// @copyright (c) 2010 CSIRO
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

#ifndef ASKAP_CP_CIRCULARBUFFER_H
#define ASKAP_CP_CIRCULARBUFFER_H

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/thread/condition.hpp"
#include "boost/circular_buffer.hpp"

namespace askap {
    namespace cp {

        template<class T>
        class CircularBuffer
        {
            public:
                CircularBuffer(const unsigned int bufSize)
                {
                    if (bufSize > 0) {
                        itsBuffer.set_capacity(bufSize);
                    } else {
                        itsBuffer.set_capacity(1);
                    }
                };

                ~CircularBuffer() {};

                void add(boost::shared_ptr<T> obj)
                {
                    // Add a pointer to the message to the back of the circular burrer
                    boost::mutex::scoped_lock lock(itsMutex);
                    itsBuffer.push_back(obj);

                    // Notify any waiters
                    lock.unlock();
                    itsCondVar.notify_all();
                };

                boost::shared_ptr<T> next(void)
                {
                    boost::mutex::scoped_lock lock(itsMutex);
                    while (itsBuffer.empty()) {
                        // While this call sleeps/blocks the mutex is released
                        itsCondVar.wait(lock);
                    }

                    // Get the pointer on the front of the circular buffer
                    boost::shared_ptr<T> obj(itsBuffer[0]);
                    itsBuffer.pop_front();

                    // No need to notify producer. The producer doesn't block because the
                    // buffer is a circular buffer.

                    return obj;
                };

            private:
                boost::circular_buffer< boost::shared_ptr<T> > itsBuffer;
                boost::mutex itsMutex;
                boost::condition itsCondVar;
        };

    };
};

#endif
