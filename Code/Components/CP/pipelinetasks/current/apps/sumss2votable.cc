/// @file sums2cmodel.cc
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

using namespace std;
using namespace askap;

static void writeHeader(std::ofstream& fs)
{
    fs << "<?xml version=\"1.0\"?>" << endl;
    fs << "<VOTABLE version=\"1.2\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << endl;
    fs << " xmlns=\"http://www.ivoa.net/xml/VOTable/v1.2\"" << endl;
    fs << " xmlns:stc=\"http://www.ivoa.net/xml/STC/v1.30\" >" << endl;
    fs << "  <RESOURCE name=\"SUMSS Catalog or catalog extract\">" << endl;
    fs << "    <TABLE name=\"catalog\">" << endl;
    fs << "      <GROUP ID=\"J2000\" utype=\"stc:AstroCoords\">" << endl;
    fs << "        <PARAM datatype=\"char\" arraysize=\"*\" ucd=\"pos.frame\" name=\"cooframe\"" << endl;
    fs << "               utype=\"stc:AstroCoords.coord_system_id\" value=\"UTC-ICRS-TOPO\" />" << endl;
    fs << "        <FIELDref ref=\"col1\"/>" << endl;
    fs << "        <FIELDref ref=\"col2\"/> " << endl;
    fs << "      </GROUP>" << endl;
    fs << "      <DESCRIPTION>Sydney University Molonglo Sky Survey</DESCRIPTION>" << endl;

    // RA
    fs << "      <FIELD name=\"RA\"   ID=\"col1\" ucd=\"pos.eq.ra;meta.main\" ref=\"J2000\"" << endl;
    fs << "             utype=\"stc:AstroCoords.Position2D.Value2.C1\"" << endl;
    fs << "             datatype=\"float\" width=\"11\" precision=\"8\" unit=\"deg\"/>" << endl;

    // Dec
    fs << "      <FIELD name=\"Dec\"   ID=\"col2\" ucd=\"pos.eq.dec;meta.main\" ref=\"J2000\"" << endl;
    fs << "             utype=\"stc:AstroCoords.Position2D.Value2.C2\"" << endl;
    fs << "             datatype=\"float\" width=\"11\" precision=\"8\" unit=\"deg\"/>" << endl;

    // Integrated Flux
    fs << "      <FIELD name=\"Flux\" ucd=\"phot.flux.density\"" << endl;
    fs << "             datatype=\"float\" width=\"7\" precision=\"2\" unit=\"mJy\"/>" << endl;

    // Major Axis
    fs << "      <FIELD name=\"Major axis\" ucd=\"phys.angSize.smajAxis\"" << endl;
    fs << "             datatype=\"float\" width=\"3\" precision=\"2\" unit=\"arcsec\"/>" << endl;

    // Minor Axis
    fs << "      <FIELD name=\"Minor axis\" ucd=\"phys.angSize.sminAxis\"" << endl;
    fs << "             datatype=\"float\" width=\"3\" precision=\"2\" unit=\"arcsec\"/>" << endl;
    
    // Position angle
    fs << "      <FIELD name=\"Position angle\" ucd=\"pos.posAng\"" << endl;
    fs << "             datatype=\"float\" width=\"3\" precision=\"2\" unit=\"deg\"/>" << endl;

    // Spectral index
    fs << "      <FIELD name=\"Spectral index\"" << endl;
    fs << "             datatype=\"float\" width=\"2\" precision=\"1\" unit=\"\"/>" << endl;

    // Spectral curvature
    fs << "      <FIELD name=\"Spectral curvature\"" << endl;
    fs << "             datatype=\"float\" width=\"2\" precision=\"1\" unit=\"\"/>" << endl;

    fs << "      <DATA>" << endl;
    fs << "        <TABLEDATA>" << endl;    
}

static void writeFooter(std::ofstream& fs)
{
    fs << "        </TABLEDATA>" << endl;
    fs << "      </DATA>" << endl;
    fs << "    </TABLE>" << endl;
    fs << "  </RESOURCE>" << endl;
    fs << "</VOTABLE>" << endl;
}

static void processLine(const std::string& line, std::ofstream& out)
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

    out << std::fixed << std::setprecision(8);
    out << "        <TR>" << endl;
    out << "          <TD>" << ra.getValue(deg) << "</TD>" << endl;
    out << "          <TD>" << dec.getValue(deg) << "</TD>" << endl;
    out << "          <TD>" << flux.getValue(mJy) << "</TD>" << endl;
    out << setprecision(2);
    out << "          <TD>" << majorAxis.getValue(arcsec) << "</TD>" << endl;
    out << "          <TD>" << minorAxis.getValue(arcsec) << "</TD>" << endl;
    out << "          <TD>" << positionAngle.getValue(deg) << "</TD>" << endl;
    out << "          <TD>0.0</TD>" << endl;
    out << "          <TD>0.0</TD>" << endl;
    out << "        </TR>" << endl; 
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

    std::ofstream out(argv[2], ios_base::trunc);
    if (!out) {
        std::cerr << "Error: Failed to open output file " << argv[2] << std::endl;
        return 1;
    }

    writeHeader(out);

    // Process each line
    std::string line;
    while (getline(in, line)) {
        if (line.find_first_of("#") == std::string::npos) {
            processLine(line, out);
        }
    }

    writeFooter(out);

    return 0;
}
