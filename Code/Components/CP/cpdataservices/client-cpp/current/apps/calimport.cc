/// @file calimport.cc
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

// System includes
#include <string>
#include <iostream>
#include <map>

// ASKAPsoft includes
#include "boost/program_options.hpp"
#include "askap/AskapError.h"
#include "casa/aipstype.h"
#include "Common/ParameterSet.h"
#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"

// Local package includes
#include "calibrationclient/CalibrationDataServiceClient.h"
#include "calibrationclient/JonesJTerm.h"
#include "calibrationclient/JonesIndex.h"
#include "calibrationclient/GenericSolution.h"

using namespace std;
using namespace askap;
using namespace askap::cp::caldataservice;
namespace po = boost::program_options;

void addTo(std::map<JonesIndex, JonesJTerm>& map,
        const casa::Short pol,
        const casa::Short antenna,
        const casa::Short beam,
        const casa::DComplex& gain)
{
    JonesJTerm jterm; // Initialised with both gains (g11 & g22) invalid
    ASKAPDEBUGASSERT(!jterm.g1IsValid());
    ASKAPDEBUGASSERT(!jterm.g2IsValid());

    // First look for the entry. Need to handle the case where the other polarisation
    // has already been added, as well as duplicates.
    std::map<JonesIndex, JonesJTerm>::const_iterator pos = map.find(JonesIndex(antenna, beam));

    if (pos != map.end()) {
        // Update the current one, otherwise use the default instance
        jterm = pos->second;
    }

    if (pol == 1) {
        assert(!jterm.g1IsValid());
        jterm = JonesJTerm(gain, true, jterm.g2(), jterm.g2IsValid());
    } else if (pol == 2) {
        assert(!jterm.g2IsValid());
        jterm = JonesJTerm(jterm.g1(), jterm.g1IsValid(), gain, true);
    } else {
        ASKAPTHROW(AskapError, "Invalid polarisation specification");
    }

    // Add/replace
    map[JonesIndex(antenna, beam)] = jterm;

    // Post-Conditions
    ASKAPCHECK(jterm.g1IsValid() || jterm.g2IsValid(), "");
}

casa::DComplex makeComplex(const std::vector<double>& values)
{
    if (values.size() == 1) {
        return casa::DComplex(values.at(0));
    } else if (values.size() == 2) {
        return casa::DComplex(values.at(0), values.at(1));
    } else {
        ASKAPTHROW(AskapError, "Can't make a complex number from value");
    }
}

GainSolution buildGainSolution(const LOFAR::ParameterSet& parset)
{
    const long timestamp = 0;
    GainSolution sol(timestamp);

    LOFAR::ParameterSet::const_iterator it;
    for (it = parset.begin(); it != parset.end(); ++it) {
        // Tokenise the value. It will look like this g11.1.2, where g11 is
        // polarisation 1, followed by the antenna number then beam number.
        std::vector<string> key;
        boost::split(key, it->first, boost::is_any_of("."));
        if (key.size() != 3) {
            ASKAPTHROW(AskapError, "Malformed key");
        }

        casa::Short pol;
        if (key[0] == "g11") {
            pol = 1;
        } else if (key[0] == "g22") {
            pol = 2;
        } else {
            ASKAPTHROW(AskapError, "Malformed key");
        }

        casa::Short antenna;
        casa::Short beam;
        try {
            antenna = boost::lexical_cast<casa::Short>(key[1]);
            beam = boost::lexical_cast<casa::Short>(key[2]);
        } catch (const boost::bad_lexical_cast&) {
            ASKAPTHROW(AskapError, "Malformed key");
        }

        std::vector<double> values = parset.getDoubleVector(it->first);
        casa::DComplex gain = makeComplex(values);
        addTo(sol.map(), pol, antenna, beam, gain);
    }

    return sol;
}

// main()
int main(int argc, char *argv[])
{
    // Configuration
    string locatorHost;
    string locatorPort;
    string serviceName;
    string filename;

    // Declare the supported options.
    po::options_description desc("Options");
    desc.add_options()
        ("help", "Produce help message")
        ("host,h", po::value<string>(&locatorHost)->default_value("localhost"),
             "IceGrid locator host")
        ("port,p", po::value<string>(&locatorPort)->default_value("4061"),
             "IceGrid locator port number")
        ("servicename,s", po::value<string>(&serviceName)->default_value("CalibrationDataService"),
             "Service name")
        ("filename,f", po::value<string>(&filename),
             "Output filename");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (const po::unknown_option& e) {
        cerr << desc << endl;
        return 1;
    }

    if (vm.count("help") || !vm.count("filename")) {
        cerr << desc << endl;
        return 1;
    }

    LOFAR::ParameterSet parset(filename);
    CalibrationDataServiceClient svc(locatorHost, locatorPort, serviceName);

    try {
        // Process gain solution if exists
        LOFAR::ParameterSet gainSubset = parset.makeSubset("gain.");
        if (gainSubset.size() > 0) {
            const long id = svc.addGainSolution(buildGainSolution(gainSubset));
            std::cout << "ID of new gain solution: " << id << std::endl;
        } else {
            std::cout << "No gains in input file" << std::endl;
        }
    } catch (const AskapError& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
