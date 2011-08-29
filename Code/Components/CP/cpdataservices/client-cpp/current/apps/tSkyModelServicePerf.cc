/// @file tSkyModelServicePerf.cc
///
/// @description
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
#include <fstream>
#include <string>
#include <vector>
#include <stdint.h>
#include <unistd.h>
#include <sys/times.h>
#include <stdexcept>

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "CommandLineParser.h"

// Local package includes
#include "skymodelclient/SkyModelServiceClient.h"
#include "skymodelclient/Component.h"
#include "skymodelclient/ComponentResultSet.h"

using namespace std;
using namespace askap::cp::skymodelservice;

#include <sys/times.h>

class Stopwatch {
    public:
        Stopwatch() : m_start(static_cast<clock_t>(-1)) { }

        void start()
        {
            struct tms t;
            m_start = times(&t);

            if (m_start == static_cast<clock_t>(-1)) {
                throw runtime_error("Error calling times()");
            }

        }

        // Returns elapsed time (since start() was called) in seconds.
        double stop()
        {
            struct tms t;
            clock_t stop = times(&t);

            if (m_start == static_cast<clock_t>(-1)) {
                throw runtime_error("Start time not set");
            }

            if (stop == static_cast<clock_t>(-1)) {
                throw runtime_error("Error calling times()");
            }

            return (static_cast<double>(stop - m_start)) / (static_cast<double>(sysconf(_SC_CLK_TCK)));

        }

    private:
        clock_t m_start;
};

Component genRandomComponent(void)
{
    long id = -1;
    double rightAscension = drand48();
    double declination = drand48();
    double positionAngle = drand48();
    double majorAxis = drand48();
    double minorAxis = drand48();
    double i1400 = drand48();
    double spectralIndex = 0.0;

    return Component(id, rightAscension, declination, positionAngle, majorAxis,
            minorAxis, i1400, spectralIndex);
}

void populate(SkyModelServiceClient& svc, const uint32_t count)
{
    vector<Component> components;
    for (uint32_t i = 0; i < count; ++i) {
        components.push_back(genRandomComponent());
    }

    svc.addComponents(components);
}

size_t coneSearch(SkyModelServiceClient& svc, const casa::Quantity& ra,
        const casa::Quantity& dec, const casa::Quantity& searchRadius,
        const casa::Quantity& fluxLimit)
{
    ComponentResultSet results = svc.coneSearch(ra, dec, searchRadius, fluxLimit);

    // Iterate over the result set
    ComponentResultSet::Iterator it;
    for (it = results.createIterator(); it.hasNext(); it.next()) {
    }

    return results.size();
}

// main()
int main(int argc, char *argv[])
{
    // Command line parser
    cmdlineparser::Parser parser;

    // Command line parameter
    cmdlineparser::FlaggedParameter<string> inputsPar("-inputs", "tSkyModelServicePerf.in");

    // Throw an exception if the parameter is not present
    parser.add(inputsPar, cmdlineparser::Parser::return_default);
    parser.process(argc, const_cast<char**> (argv));

    // Create a subset
    LOFAR::ParameterSet parset(inputsPar);

    const string locatorHost = parset.getString("ice.locator.host");
    const string locatorPort = parset.getString("ice.locator.port");
    const string serviceName = parset.getString("skymodelservice.name");
    const uint32_t batchSize = parset.getUint32("test.batchsize");
    const uint32_t nBatches = parset.getUint32("test.nbatches");
    const string populateCSV = parset.getString("test.outfile.populate");
    const string coneCSV = parset.getString("test.outfile.conesearch");

    SkyModelServiceClient svc(locatorHost, locatorPort, serviceName);

    // Open the output (timings) file for population and cone search
    ofstream populateFile(populateCSV.c_str());
    ofstream coneFile(coneCSV.c_str());

    // To track total times for populate and conesearch
    double populateTotal = 0.0;
    double coneTotal = 0.0;

    // Cone search parameters
    const casa::Quantity ra(0.0, "deg");
    const casa::Quantity dec(0.0, "deg");
    const casa::Quantity searchRadius(180, "deg");
    const casa::Quantity fluxLimit(1, "uJy");

    Stopwatch sw;
    for (uint32_t i = 0; i < nBatches; ++i) {
        // Time populate()
        sw.start();
        populate(svc, batchSize);
        double time = sw.stop();
        populateTotal += time;
        populateFile << batchSize << ", " << time << endl;

        // Time conesearch()
        sw.start();
        size_t count = coneSearch(svc, ra, dec, searchRadius, fluxLimit);
        time = sw.stop();
        cout << "Cone search returned " << count << " components" << endl;
        coneTotal += time;
        coneFile << batchSize << ", " << count << ", " << time << endl;
    }

    cout << "Total time for populate():   " << populateTotal << " (seconds)" << endl;
    cout << "Total time for coneSearch(): " << coneTotal << " (seconds)" << endl;

    populateFile.close();
    coneFile.close();
    return 0;
}
