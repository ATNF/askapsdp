/// @file MonitorPoint.h
///
/// @copyright (c) 2013 CSIRO
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

#ifndef ASKAP_CP_INGEST_MONITORPOINT_H
#define ASKAP_CP_INGEST_MONITORPOINT_H

// System includes
#include <string>
#include <stdint.h>

// Local package includes
#include "monitoring/AbstractMonitorPoint.h"

namespace askap {
namespace cp {
namespace ingest {

/// @brief Monitoring point template with specialisations for each of the
/// "value" types supported. There is no "generic" implementation, only the
/// specialised implementations.
template <typename T>
class MonitorPoint : public AbstractMonitorPoint<T> {
};

template <>
class MonitorPoint<bool> : public AbstractMonitorPoint<bool> {
    public:

        MonitorPoint(const std::string& name) : AbstractMonitorPoint<bool>(name) {
        }

    private:
        void send(const std::string& name, const bool& value, bool alarm = false) {
            itsDestination->sendBool(name, value);
        }
};

template <>
class MonitorPoint<float> : public AbstractMonitorPoint<float> {
    public:

        MonitorPoint(const std::string& name) : AbstractMonitorPoint<float>(name) {
        }

    private:
        void send(const std::string& name, const float& value, bool alarm = false) {
            itsDestination->sendFloat(name, value);
        }
};

template <>
class MonitorPoint<double> : public AbstractMonitorPoint<double> {
    public:

        MonitorPoint(const std::string& name) : AbstractMonitorPoint<double>(name) {
        }

    private:
        void send(const std::string& name, const double& value, bool alarm = false) {
            itsDestination->sendDouble(name, value);
        }
};

template <>
class MonitorPoint<int32_t> : public AbstractMonitorPoint<int32_t> {
    public:

        MonitorPoint(const std::string& name) : AbstractMonitorPoint<int32_t>(name) {
        }

    private:
        void send(const std::string& name, const int32_t& value, bool alarm = false) {
            itsDestination->sendInt32(name, value);
        }
};

template <>
class MonitorPoint<int64_t> : public AbstractMonitorPoint<int64_t> {
    public:

        MonitorPoint(const std::string& name) : AbstractMonitorPoint<int64_t>(name) {
        }

    private:
        void send(const std::string& name, const int64_t& value, bool alarm = false) {
            itsDestination->sendInt64(name, value);
        }
};

template <>
class MonitorPoint<std::string> : public AbstractMonitorPoint<std::string> {
    public:

        MonitorPoint(const std::string& name) : AbstractMonitorPoint<std::string>(name) {
        }

    private:
        void send(const std::string& name, const std::string& value, bool alarm = false) {
            itsDestination->sendString(name, value);
        }
};

} // End namespace ingest
} // End namespace cp
} // End namespace askap

#endif
