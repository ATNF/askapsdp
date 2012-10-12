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

// ASKAPsoft include
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "boost/lexical_cast.hpp"
#include "casa/Quanta/Quantum.h"
#include "votable/VOTable.h" // includes VOTable*.h

using namespace std;
using namespace askap;
using namespace askap::accessors;

template <typename T>
static std::string toString(const T& val, int precision)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(8);
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
        f.setUCD("phot.flux.density");
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
        // Major Axis
        VOTableField f;
        f.setName("Minor axis");
        f.setUCD("phys.angSize.sminAxis");
        f.setDatatype("float"); 
        f.setUnit("arcsec");
        tab.addField(f);
    }

    {
        // Major Axis
        VOTableField f;
        f.setName("Position angle");
        f.setUCD("pos.posAng");
        f.setDatatype("float"); 
        f.setUnit("deg");
        tab.addField(f);
    }

    {
        // Spectral index
        VOTableField f;
        f.setName("Spectral index");
        f.setDatatype("float"); 
        tab.addField(f);
    }

    {
        // Spectral curvature
        VOTableField f;
        f.setName("Spectral curvature");
        f.setDatatype("float"); 
        tab.addField(f);
    }
}

static VOTableRow processLine(const std::string& line)
{
    // Tokenize the line
    stringstream iss(line);
    vector<string> tokens;
    tokens.reserve(22); // Expect 22 tokens
    copy(istream_iterator<string>(iss),
            istream_iterator<string>(),
            back_inserter<vector<string> >(tokens));

    ASKAPCHECK(tokens.size() == 22, "Expected 22 tokens, got " << tokens.size());

    // Create these once to avoid the performance impact of creating them over and over.
    static casa::Unit deg("deg");
    static casa::Unit arcsec("arcsec");
    static casa::Unit Jy("Jy");
    static casa::Unit mJy("mJy");

    // Process Right Ascension (e.g. 23 59 57.37)
    const double ra_hours = boost::lexical_cast<casa::Double>(tokens[0]);
    const double ra_mins = boost::lexical_cast<casa::Double>(tokens[1]);
    const double ra_secs = boost::lexical_cast<casa::Double>(tokens[2]);
    const casa::Quantity ra(15 * (ra_hours + ra_mins/60 + ra_secs/3600), deg);

    // Process declination (e.g. -31 09 53.1)
    const double dec_degs = boost::lexical_cast<casa::Double>(tokens[3]);
    const double dec_mins = boost::lexical_cast<casa::Double>(tokens[4]);
    const double dec_secs = boost::lexical_cast<casa::Double>(tokens[5]);
    casa::Quantity dec(fabs(dec_degs) + dec_mins/60 + dec_secs/3600, deg);
    if (dec_degs < 0.0) {
        dec *= -1.0;
    }

    // Process Flux (integrated flux in mJy)
    const casa::Quantity flux = casa::Quantity(boost::lexical_cast<casa::Double>(tokens[10]), mJy);

    // Process major axis (arcsec)
    const casa::Quantity majorAxis(boost::lexical_cast<casa::Double>(tokens[12]), arcsec);

    // Process minor axis (arcsec)
    const casa::Quantity minorAxis(boost::lexical_cast<casa::Double>(tokens[13]), arcsec);

    // Process position angle (degrees)
    const casa::Quantity positionAngle(boost::lexical_cast<casa::Double>(tokens[14]), deg);

    VOTableRow row;
    row.addCell(toString(ra.getValue(deg), 8));
    row.addCell(toString(dec.getValue(deg), 8));
    row.addCell(toString(flux.getValue(mJy), 8));
    row.addCell(toString(minorAxis.getValue(arcsec), 2));
    row.addCell(toString(majorAxis.getValue(arcsec), 2));
    row.addCell(toString(positionAngle.getValue(deg), 2));
    row.addCell(toString(0.0, 2)); // Spectral index
    row.addCell(toString(0.0, 2)); // Spectral curvature

    return row;
}

int main(int argc, char *argv[])
{
    // Usage
    if (argc != 3) {
        std::cerr << "Usage: sumss2votable <input catalog filename> <output filename>" << std::endl;
        return 1;
    }

    std::ifstream in(argv[1]);
    if (!in) {
        std::cerr << "Error: Failed to open input file " << argv[1] << std::endl;
        return 1;
    }

    // Begin building the VOTable
    VOTable vot;
    VOTableResource vores;
    vores.setName("SUMSS Catalog or catalog extract");

    VOTableTable vottab;
    vottab.setName("catalog");
    vottab.setDescription("Sydney University Molonglo Sky Survey");

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
    std::string line;
    while (getline(in, line)) {
        if (line.find_first_of("#") == std::string::npos) {
            const VOTableRow row = processLine(line);
            vottab.addRow(row);
        }
    }

    vores.addTable(vottab);
    vot.addResource(vores);

    // Write the VOTable out
    vot.toXml(argv[2]);

    return 0;
}
