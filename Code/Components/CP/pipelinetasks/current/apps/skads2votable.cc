/// @file sums2votable.cc
///
/// @copyright (c) 2012 CSIRO
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

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <vector>
#include <utility>
#include <exception>
#include <stdlib.h>
#include <limits>

// ASKAPsoft include
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "boost/lexical_cast.hpp"
#include "casa/Quanta/Quantum.h"
#include "scimath/Fitting/LinearFit.h"
#include "scimath/Functionals/Polynomial.h"
#include "scimath/Mathematics/AutoDiff.h"
#include "casa/BasicMath/Math.h"
#include "casa/Arrays/Vector.h"
#include "votable/VOTable.h" // includes VOTable*.h

using namespace std;
using namespace askap;
using namespace askap::accessors;

ASKAP_LOGGER(logger, ".skads2votable");

template <typename T>
static std::string toString(const T& val, int precision)
{
    std::stringstream ss;
    ss << std::setprecision(precision);
    ss << val;
    return ss.str();
}

static void addFields(VOTableTable& tab)
{
    {
        // RA
        VOTableField f;
        f.setName("RA");
        f.setID("col1");
        f.setUCD("pos.eq.ra;meta.main");
        f.setRef("J2000");
        f.setUType("stc:AstroCoords.Position2D.Value2.C1");
        f.setDatatype("float"); 
        f.setUnit("deg");
        tab.addField(f);
    }

    {
        // Dec
        VOTableField f;
        f.setName("Dec");
        f.setID("col2");
        f.setUCD("pos.eq.dec;meta.main");
        f.setRef("J2000");
        f.setUType("stc:AstroCoords.Position2D.Value2.C2");
        f.setDatatype("float"); 
        f.setUnit("deg");
        tab.addField(f);
    }

    {
        // Integrated Flux
        VOTableField f;
        f.setName("Flux");
        f.setUCD("phot.flux.density.integrated");
        f.setDatatype("float"); 
        f.setUnit("mJy");
        tab.addField(f);
    }

    {
        // Major Axis
        VOTableField f;
        f.setName("Major axis");
        f.setUCD("phys.angSize.smajAxis");
        f.setDatatype("float"); 
        f.setUnit("arcsec");
        tab.addField(f);
    }

    {
        // Minor Axis
        VOTableField f;
        f.setName("Minor axis");
        f.setUCD("phys.angSize.sminAxis");
        f.setDatatype("float"); 
        f.setUnit("arcsec");
        tab.addField(f);
    }

    {
        // Position angle
        VOTableField f;
        f.setName("Position angle");
        f.setUCD("phys.angSize;pos.posAng");
        f.setDatatype("float"); 
        f.setUnit("deg");
        tab.addField(f);
    }

    {
        // Spectral index
        VOTableField f;
        f.setName("Spectral index");
        f.setUCD("spect.index");
        f.setDatatype("float"); 
        tab.addField(f);
    }

    {
        // Spectral curvature
        VOTableField f;
        f.setName("Spectral curvature");
        f.setUCD("spect.curvature");
        f.setDatatype("float"); 
        tab.addField(f);
    }
}

// Given a fitting solution (polynomial) verify the solution can correctly evaluate for x
//
// @param[in] solution  the solution to evaluate, must be degree 2
// @param[in] y         the expected value
// @param[in] x         the x value at which to evaluate the polynomial
// @param[in] freqstr   the string indicating the frequency being tested
static void verifyFit(const casa::Vector<double>& solution, double y, double x,
               const std::string& freqstr)
{
    // Assert Preconditions
    ASKAPCHECK(solution.size() == 3, "Expected 3 polynomial coefficients, got "
            << solution.size());

    // Test the result
    casa::Polynomial<double> poly(2);
    poly.setCoefficient(0, solution[0]);
    poly.setCoefficient(1, solution[1]);
    poly.setCoefficient(2, solution[2]);

    //const double tolerance = 1e-12;
    const double tolerance = std::numeric_limits<float>::epsilon();
    const double expected = y;
    const double actual = poly(x);
    ASKAPCHECK(casa::near(expected, actual, tolerance),
            "Fitting error " << freqstr << " - Expected: " << expected << ", actual: " << actual);
}

// Returns the pair (spectral index, spectral curvature)
static pair<double, double> fluxFit(double i_610, double i_1400, double i_4860)
{
    // Assemble input data
    const size_t n = 3;

    casa::Vector<double> x(n);
    x(0) = log10(610.0 / 1400.0);
    x(1) = log10(1400.0 / 1400.0);
    x(2) = log10(4860.0 / 1400.0);

    casa::Vector<double> fluxes(n);
    fluxes(0) = i_610;
    fluxes(1) = i_1400;
    fluxes(2) = i_4860;

    // Create a fitter
    casa::LinearFit<double> fitter;
    casa::Polynomial<casa::AutoDiff<double> > combination(2);
    fitter.setFunction(combination);

    // Do fitting
    const casa::Vector<double> solution = fitter.fit(x, fluxes);

    // Verify the fit result can recover the three input fluxes
    verifyFit(solution, fluxes(0), x(0), "i_610");
    verifyFit(solution, fluxes(1), x(1), "i_1400");
    verifyFit(solution, fluxes(2), x(2), "i_4860");

    return make_pair<double, double>(solution(1), solution(2));
}

static VOTableRow processLine(const std::string& line)
{
    // Tokenize the line
    stringstream iss(line);
    vector<string> tokens;
    tokens.reserve(13); // Expect 13 tokens
    copy(istream_iterator<string>(iss),
            istream_iterator<string>(),
            back_inserter<vector<string> >(tokens));

    ASKAPCHECK(tokens.size() == 13, "Expected 13 tokens, got " << tokens.size());

    // Create these once to avoid the performance impact of creating them over and over.
    static casa::Unit deg("deg");
    static casa::Unit rad("rad");
    static casa::Unit arcsec("arcsec");
    static casa::Unit Jy("Jy");
    static casa::Unit mJy("mJy");

    const casa::Quantity ra(boost::lexical_cast<casa::Double>(tokens[3]), deg);
    const casa::Quantity dec(boost::lexical_cast<casa::Double>(tokens[4]), deg);

    // Process Flux (log10 of integrated flux)
    const casa::Quantity flux(pow(10.0, boost::lexical_cast<casa::Double>(tokens[10])), Jy);

    // Process major axis (arcsec)
    const casa::Quantity majorAxis(boost::lexical_cast<casa::Double>(tokens[6]), arcsec);

    // Process minor axis (arcsec)
    const casa::Quantity minorAxis(boost::lexical_cast<casa::Double>(tokens[7]), arcsec);

    // Process position angle (degrees)
    const casa::Quantity positionAngle(boost::lexical_cast<casa::Double>(tokens[5]), rad);

    VOTableRow row;
    row.addCell(toString(ra.getValue(deg), 8));
    row.addCell(toString(dec.getValue(deg), 8));
    row.addCell(toString(flux.getValue(mJy), 8));
    row.addCell(toString(minorAxis.getValue(arcsec), 2));
    row.addCell(toString(majorAxis.getValue(arcsec), 2));
    row.addCell(toString(positionAngle.getValue(deg), 2));
    const pair<double, double> fluxChange = fluxFit(
            boost::lexical_cast<casa::Double>(tokens[9]),
            boost::lexical_cast<casa::Double>(tokens[10]),
            boost::lexical_cast<casa::Double>(tokens[11]));

    row.addCell(toString(fluxChange.first, 8)); // Spectral index
    row.addCell(toString(fluxChange.second, 8)); // Spectral curvature

    return row;
}

int main(int argc, char *argv[])
{
    // Usage
    if (argc != 3) {
        std::cerr << "Usage: skads2votable <input catalog filename> <output filename>" << std::endl;
        return 1;
    }

    ASKAPLOG_INIT("askap.log_cfg");

    try {
        std::ifstream in(argv[1]);
        if (!in) {
            std::cerr << "Error: Failed to open input file " << argv[1] << std::endl;
            return 1;
        }

        // Begin building the VOTable
        VOTable vot;
        VOTableResource vores;
        vores.setName("SKADS Catalog or catalog extract");

        VOTableTable vottab;
        vottab.setName("catalog");
        vottab.setDescription("Square Kilometre Array Design Studies (SKADS) Simulation");

        // Add group
        VOTableGroup grp;
        grp.setID("J2000");
        grp.setUType("stc:AstroCoords");
        {
            VOTableParam p;
            p.setDatatype("char");
            p.setArraysize("*");
            p.setUCD("pos.frame");
            p.setName("cooframe");
            p.setUType("stc:AstroCoords.coord_system_id");
            p.setValue("UTC-ICRS-TOPO");
            grp.addParam(p);
        }
        grp.addFieldRef("col1");
        grp.addFieldRef("col2");
        vottab.addGroup(grp);

        // Add fields
        addFields(vottab);

        // Add rows
        string line;
        unsigned long count = 0;
        while (getline(in, line)) {
            if (line.find_first_of("#") == string::npos) {
                const VOTableRow row = processLine(line);
                vottab.addRow(row);
                ++count;
                if (count % 100000 == 0) {
                    cout << "Processed " << count << " rows" << endl;
                }
            }
        }
        cout << "Processed " << count << " rows" << endl;

        vores.addTable(vottab);
        vot.addResource(vores);

        // Write the VOTable out
        cout << "Writing XML output" << endl;
        vot.toXML(argv[2]);

    } catch (const askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
        return 1;
    } catch (const std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
        return 1;
    }

    return 0;
}
