/// @file UVChannelConfig.cc
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

// Include own header file first
#include "UVChannelConfig.h"

// System includes
#include <stdint.h>
#include <string>
#include <sstream>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "Common/ParameterSet.h"

// Local package includes

// Using
using namespace std;
using namespace askap;
using namespace askap::cp;
using namespace askap::cp::channels;

UVChannelConfig::UVChannelConfig(const LOFAR::ParameterSet& parset) : itsParset(parset.makeSubset("uvchannel."))
{
}

std::string UVChannelConfig::getBrokerId(const std::string& name, const int chan) const
{
    // Confirm the channel name exists
    vector<string> channels = itsParset.getStringVector("channels");
    vector<string>::const_iterator it;
    it = find(channels.begin(), channels.end(), name);
    ASKAPCHECK(it != channels.end(), "Not a vaild channel name");
    
    // Find the number of blocks
    stringstream ss;
    ss << "channel." << name << ".nblocks";
    const uint32_t nBlocks = itsParset.getUint32(ss.str());

    string broker;
    // Find which block the channel resides in
    for (uint32_t i = 1; i <=nBlocks; ++i) {
        ss.str("");
        ss << "channel." << name << ".block_" << i;
        vector<string> block = itsParset.getStringVector(ss.str());
        ASKAPCHECK(block.size() == 3, "Invalid uvchannel block specification");
        const int start = strTo<int>(block[0]);
        const int end = strTo<int>(block[1]);
        if (chan == start || chan == end || ((chan > start) && (chan < end))) {
            broker = block[2];
            break;
        }
    }
    ASKAPCHECK(!broker.empty(), "Could not map channel to broker");

    // Confirm the broker name exists
    vector<string> brokers = itsParset.getStringVector("brokers");
    it = find(brokers.begin(), brokers.end(), broker);
    ASKAPCHECK(it != brokers.end(), "Not a vaild broker");

    return broker;
}

std::string UVChannelConfig::getHost(const std::string& brokerId) const
{
    std::stringstream ss;
    ss << "broker." << brokerId << ".host";
    return itsParset.getString(ss.str());
}

int UVChannelConfig::getPort(const std::string& brokerId) const
{
    std::stringstream ss;
    ss << "broker." << brokerId << ".port";
    return strTo<int>(itsParset.getString(ss.str()));
}

std::string UVChannelConfig::getTopic(const std::string& name, const int chan) const
{
    std::stringstream ss;
    ss << name << "_" << chan;
    return ss.str();
}

int UVChannelConfig::getChannel(const std::string& name, const std::string& topic) const
{
    size_t pos = topic.find("_");
    std::string num = topic.substr(pos+1);
    return strTo<int>(num);    
}

template <class T>
T UVChannelConfig::strTo(const std::string& str) const
{
    T val;
    std::istringstream iss(str);
    bool fail = (iss >> val).fail();
    ASKAPCHECK(!fail, "Failed to convert from String to type T");

    return val;
}
