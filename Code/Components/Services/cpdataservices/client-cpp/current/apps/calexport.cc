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
#include <vector>

// ASKAPsoft includes
#include "boost/program_options.hpp"
#include "askap/AskapError.h"
#include "casa/aipstype.h"
#include "Common/ParameterSet.h"

// Local package includes
#include "calibrationclient/CalibrationDataServiceClient.h"
#include "calibrationclient/JonesJTerm.h"
#include "calibrationclient/JonesIndex.h"
#include "calibrationclient/GenericSolution.h"

using namespace std;
using namespace askap;
using namespace askap::cp::caldataservice;
namespace po = boost::program_options;

void dumpGainSolution(const GainSolution& sol, std::ofstream& file)
{
    file << "# Gain solution timestamp: " << sol.timestamp() << endl;
    typedef std::map<JonesIndex, JonesJTerm> MapType;
    const MapType map = sol.map();
    MapType::const_iterator it;
    for (it = map.begin(); it != map.end(); ++it) {
        const JonesIndex index = it->first;
        const JonesJTerm jterm = it->second;
        if (jterm.g1IsValid()) {
            file << "gain.g11." << index.antenna() << "." << index.beam() << " = ["
                << jterm.g1().real() << ", " << jterm.g1().imag() << "]" << endl;
        }
        if (jterm.g2IsValid()) {
            file << "gain.g22." << index.antenna() << "." << index.beam() << " = ["
                << jterm.g2().real() << ", " << jterm.g2().imag() << "]" << endl;
        }
    }
}

void dumpLeakageSolution(const LeakageSolution& sol, std::ofstream& file)
{
    file << "# Leakage solution timestamp: " << sol.timestamp() << endl;
    typedef std::map<JonesIndex, JonesDTerm> MapType;
    const MapType map = sol.map();
    MapType::const_iterator it;
    for (it = map.begin(); it != map.end(); ++it) {
        const JonesIndex index = it->first;
        const JonesDTerm dterm = it->second;
        file << "leakage.d12." << index.antenna() << "." << index.beam() << " = ["
            << dterm.d12().real() << ", " << dterm.d12().imag() << "]" << endl;
        file << "leakage.d21." << index.antenna() << "." << index.beam() << " = ["
            << dterm.d21().real() << ", " << dterm.d21().imag() << "]" << endl;
    }
}

// NOTE: This dumps out the entie bandpass solution, ignoring the JonesJTerm
// validity flags
void dumpBandpassSolution(const BandpassSolution& sol, std::ofstream& file)
{
    file << "# Bandpass solution timestamp: " << sol.timestamp() << endl;
    typedef std::map<JonesIndex, std::vector<JonesJTerm> > MapType;
    const MapType map = sol.map();
    MapType::const_iterator it;
    for (it = map.begin(); it != map.end(); ++it) {
        const JonesIndex index = it->first;
        const std::vector<JonesJTerm> jterms = it->second;

        // Iterator over the vector for g11 and g22 simultaneously, writing the
        // output into a stringstream
        std::stringstream g11;
        std::stringstream g22;
        g11 << "bandpass.g11." << index.antenna() << "." << index.beam() << " = [";
        g22 << "bandpass.g22." << index.antenna() << "." << index.beam() << " = [";
        std::vector<JonesJTerm>::const_iterator vec_it;
        for (vec_it = jterms.begin(); vec_it != jterms.end(); ++vec_it) {
            if (vec_it != jterms.begin()) {
                g11 << ", ";
                g22 << ", ";
            }
            g11 << "[" << vec_it->g1().real() << ", " << vec_it->g1().imag() << "]" << endl;
            g22 << "[" << vec_it->g2().real() << ", " << vec_it->g2().imag() << "]" << endl;
        }
        g11 << "]" << endl;
        g22 << "]" << endl;

        file << g11.str();
        file << g22.str();
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
    long gainID = -1;
    long leakageID = -1;
    long bandpassID = -1;

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
        ("gid,g", po::value<long>(&gainID)->default_value(-1),
             "Gains solution identifier (or -1 to get latest)")
        ("lid,l", po::value<long>(&leakageID)->default_value(-1),
             "Leakage solution identifier (or -1 to get latest)")
        ("bid,b", po::value<long>(&bandpassID)->default_value(-1),
             "Bandpass solution identifier (or -1 to get latest)")
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

    CalibrationDataServiceClient svc(locatorHost, locatorPort, serviceName);
    ofstream file(filename.c_str(), ios::out);

    // Gain Solution
    if (gainID == -1) {
        gainID = svc.getCurrentGainSolutionID();
        cout << "Calibration data service reports latest gain solution is: "
            << gainID << endl;
    }
    cout << "Obtaining gain solution " << gainID << endl;
    GainSolution gainSolution = svc.getGainSolution(gainID);
    dumpGainSolution(gainSolution, file);

    // Leakage Solution
    if (leakageID == -1) {
        leakageID = svc.getCurrentLeakageSolutionID();
        cout << "Calibration data service reports latest leakage solution is: "
            << leakageID << endl;
    }
    cout << "Obtaining leakage solution " << leakageID << endl;
    LeakageSolution leakageSolution = svc.getLeakageSolution(leakageID);
    dumpLeakageSolution(leakageSolution, file);

    // Bandpass Solution
    if (bandpassID == -1) {
        bandpassID = svc.getCurrentBandpassSolutionID();
        cout << "Calibration data service reports latest bandpass solution is: "
            << bandpassID << endl;
    }
    cout << "Obtaining bandpass solution " << bandpassID << endl;
    BandpassSolution bandpassSolution = svc.getBandpassSolution(bandpassID);
    dumpBandpassSolution(bandpassSolution, file);

    file.close();
    return 0;
}
