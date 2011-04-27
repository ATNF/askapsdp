/// @file calexport.cc
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
#include <map>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/aipstype.h"
#include "CommandLineParser.h"
#include "Common/ParameterSet.h"

// Local package includes
#include "calibrationclient/CalibrationDataServiceClient.h"
#include "calibrationclient/JonesJTerm.h"
#include "calibrationclient/JonesIndex.h"
#include "calibrationclient/GenericSolution.h"

using namespace std;
using namespace askap;
using namespace askap::cp::caldataservice;

void usage(void)
{
    std::cout << "usage: calexport [-h hostname] [-p port] [-s servicename] -f <filename>"
        << std::endl;
}

void dumpGainSolution(const GainSolution& sol, std::ofstream& file)
{
    file << "# Gain solution timestamp: " << sol.timestamp() << std::endl;
    const std::map<JonesIndex, JonesJTerm> map = sol.map();
    std::map<JonesIndex, JonesJTerm>::const_iterator it;
    for (it = map.begin(); it != map.end(); ++it) {
        const JonesIndex index = it->first;
        const JonesJTerm jterm = it->second;
        if (jterm.g1IsValid()) {
            file << "gain.g11." << index.antenna() << "." << index.beam() << " = ["
                << jterm.g1().real() << ", " << jterm.g1().imag() << "]" << std::endl;
        }
        if (jterm.g2IsValid()) {
            file << "gain.g22." << index.antenna() << "." << index.beam() << " = ["
                << jterm.g2().real() << ", " << jterm.g2().imag() << "]" << std::endl;
        }
    }
}

// main()
int main(int argc, char *argv[])
{
    // Command line parser
    cmdlineparser::Parser parser;

    // Command line parameter
    cmdlineparser::FlaggedParameter<string> locatorHost("-h", "localhost");
    cmdlineparser::FlaggedParameter<string> locatorPort("-p", "4061");
    cmdlineparser::FlaggedParameter<string> serviceName("-s", "CalibrationDataService");
    cmdlineparser::FlaggedParameter<string> filename("-f");

    // Throw an exception if the parameter is not present
    parser.add(locatorHost, cmdlineparser::Parser::return_default);
    parser.add(locatorPort, cmdlineparser::Parser::return_default);
    parser.add(serviceName, cmdlineparser::Parser::return_default);
    parser.add(filename, cmdlineparser::Parser::throw_exception);
    try {
        parser.process(argc, const_cast<char**> (argv));
    } catch (const cmdlineparser::XParser& e) {
        usage();
        return 1;
    }

    CalibrationDataServiceClient svc(locatorHost, locatorPort, serviceName);

    const casa::Long gainsID = svc.getCurrentGainSolutionID();
    std::cout << "Obtaining gain solution " << gainsID << std::endl;
    GainSolution gainSolution = svc.getGainSolution(gainsID);
    std::ofstream file(filename.getValue().c_str(), ios::out);
    dumpGainSolution(gainSolution, file);
    file.close();

    return 0;
}
