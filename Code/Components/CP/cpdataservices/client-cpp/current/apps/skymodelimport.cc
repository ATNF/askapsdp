/// @file skymodelimport.cc
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
#include <fstream>

// ASKAPsoft includes
#include "boost/program_options.hpp"
#include "askap/AskapError.h"
#include "casa/aipstype.h"

// Local package includes
#include "skymodelclient/SkyModelServiceClient.h"
#include "skymodelclient/Component.h"

using namespace std;
using namespace askap;
using namespace askap::cp::skymodelservice;
namespace po = boost::program_options;

void processLine(const std::string& line, std::vector<Component>& components)
{
    //////////////////////////////////////////////////////////////////////////////////////
    // The below Matt's SKADS .dat file format
    //////////////////////////////////////////////////////////////////////////////////////
    const casa::uShort totalTokens = 13;
    const casa::uShort raPos = 3;
    const casa::uShort decPos = 4;
    const casa::uShort fluxPos = 10;
    const casa::uShort majorAxisPos = 6;
    const casa::uShort minorAxisPos = 7;
    const casa::uShort positionAnglePos = 5;

    // Tokenize the line
    stringstream iss(line);
    vector<string> tokens;
    copy(istream_iterator<string>(iss),
         istream_iterator<string>(),
         back_inserter<vector<string> >(tokens));

    if (tokens.size() != totalTokens) {
        ASKAPTHROW(AskapError, "Malformed entry - Expected " << totalTokens << " tokens");
    }

    // Extract the values from the tokens
    casa::Double _ra = boost::lexical_cast<casa::Double>(tokens[raPos]);
    casa::Double _dec = boost::lexical_cast<casa::Double>(tokens[decPos]);
    casa::Double _flux = pow(10.0, boost::lexical_cast<casa::Double>(tokens[fluxPos]));
    casa::Double _majorAxis = boost::lexical_cast<casa::Double>(tokens[majorAxisPos]);
    casa::Double _minorAxis = boost::lexical_cast<casa::Double>(tokens[minorAxisPos]);
    casa::Double _positionAngle = boost::lexical_cast<casa::Double>(tokens[positionAnglePos]);

    // Fix some quirks in gaussian sources
    if (_majorAxis > 0.0 || _minorAxis > 0.0) {

        // Ensure major axis is larger than minor axis
        if (_majorAxis < _minorAxis) {
            casa::Double tmp = _minorAxis;
            _minorAxis = _majorAxis;
            _majorAxis = tmp;
        }

        if (_minorAxis == 0.0) {
            _minorAxis = 1.0e-15;
        }
    }

    components.push_back(Component(-1,
            casa::Quantity(_ra, "deg"),
            casa::Quantity(_dec, "deg"),
            casa::Quantity(_positionAngle, "rad"),
            casa::Quantity(_majorAxis, "arcsec"),
            casa::Quantity(_minorAxis, "arcsec"),
            casa::Quantity(_flux, "Jy")));
}

void uploadComponents(SkyModelServiceClient& svc, std::vector<Component>& components)
{
    if(!components.empty()) {
        std::cout << "Sending " << components.size() << " component entries to server" << std::endl;
        svc.addComponents(components);
        components.clear();
    }
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
        ("servicename,s", po::value<string>(&serviceName)->default_value("SkyModelService"),
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

    // Get a handle to the service
    SkyModelServiceClient svc(locatorHost, locatorPort, serviceName);

    // Open the input file
    std::ifstream file(filename.c_str());

    // Create a buffer for the components
    std::vector<Component> components;

    // Process the file
    const casa::uLong batchSize = 50000;
    std::string line;
    while (getline(file, line)) {
        if (line.find_first_of("#") == std::string::npos) {
            processLine(line, components);
            if (components.size() >= batchSize) {
                uploadComponents(svc, components);
            }
        }
    }

    uploadComponents(svc, components);

    return 0;
}
