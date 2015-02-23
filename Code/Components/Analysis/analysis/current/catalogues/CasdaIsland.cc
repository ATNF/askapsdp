/// @file
///
/// XXX Notes on program XXX
///
/// @copyright (c) 2014 CSIRO
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#include <catalogues/CasdaIsland.h>
#include <catalogues/CatalogueEntry.h>
#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/RadioSource.h>
#include <outputs/AskapVOTableCatalogueWriter.h>
#include <Common/ParameterSet.h>
#include <duchamp/Outputs/CatalogueSpecification.hh>
#include <duchamp/Outputs/columns.hh>
#include <vector>

ASKAP_LOGGER(logger, ".casdaisland");

namespace askap {

namespace analysis {

CasdaIsland::CasdaIsland(sourcefitting::RadioSource &obj,
                         const LOFAR::ParameterSet &parset):
    CatalogueEntry(parset),
    itsName(obj.getName()),
    itsNumComponents(obj.numFits("best")),
    itsRAs(obj.getRAs()),
    itsDECs(obj.getDecs()),
    itsRA(obj.getRA()),
    itsDEC(obj.getDec()),
    itsFreq(obj.getVel()),
    itsMaj(obj.getMajorAxis()),
    itsMin(obj.getMinorAxis()),
    itsPA(obj.getPositionAngle()),
    itsFluxInt(obj.getIntegFlux()),
    itsFluxPeak(obj.getPeakFlux()),
    itsXmin(obj.getXmin()),
    itsXmax(obj.getXmax()),
    itsYmin(obj.getYmin()),
    itsYmax(obj.getYmax()),
    itsNumPix(obj.getSpatialSize()),
    itsXaverage(obj.getXaverage()),
    itsYaverage(obj.getYaverage()),
    itsXcentroid(obj.getXCentroid()),
    itsYcentroid(obj.getYCentroid()),
    itsXpeak(obj.getXPeak()),
    itsYpeak(obj.getYPeak()),
    itsFlag1(0),
    itsFlag2(0),
    itsFlag3(0),
    itsFlag4(0),
    itsComment("")
{
    std::stringstream id;
    id << itsIDbase << obj.getID();
    itsIslandID = id.str();
}

const float CasdaIsland::ra()
{
    return itsRA;
}

const float CasdaIsland::dec()
{
    return itsDEC;
}

const std::string CasdaIsland::id()
{
    return itsIslandID;
}

void CasdaIsland::printTableRow(std::ostream &stream,
                                duchamp::Catalogues::CatalogueSpecification &columns)
{
    stream.setf(std::ios::fixed);
    for (size_t i = 0; i < columns.size(); i++) {
        this->printTableEntry(stream, columns.column(i));
    }
    stream << "\n";
}

void CasdaIsland::printTableEntry(std::ostream &stream,
                                  duchamp::Catalogues::Column &column)
{
    std::string type = column.type();
    if (type == "ID") {
        column.printEntry(stream, itsIslandID);
    } else if (type == "NAME") {
        column.printEntry(stream, itsName);
    } else if (type == "NCOMP") {
        column.printEntry(stream, itsNumComponents);
    } else if (type == "RA") {
        column.printEntry(stream, itsRAs);
    } else if (type == "DEC") {
        column.printEntry(stream, itsDECs);
    } else if (type == "RAJD") {
        column.printEntry(stream, itsRA);
    } else if (type == "DECJD") {
        column.printEntry(stream, itsDEC);
    } else if (type == "FREQ") {
        column.printEntry(stream, itsFreq);
    } else if (type == "MAJ") {
        column.printEntry(stream, itsMaj);
    } else if (type == "MIN") {
        column.printEntry(stream, itsMin);
    } else if (type == "PA") {
        column.printEntry(stream, itsPA);
    } else if (type == "FINT") {
        column.printEntry(stream, itsFluxInt);
    } else if (type == "FPEAK") {
        column.printEntry(stream, itsFluxPeak);
    } else if (type == "XMIN") {
        column.printEntry(stream, itsXmin);
    } else if (type == "XMAX") {
        column.printEntry(stream, itsXmax);
    } else if (type == "YMIN") {
        column.printEntry(stream, itsYmin);
    } else if (type == "YMAX") {
        column.printEntry(stream, itsYmax);
    } else if (type == "NPIX") {
        column.printEntry(stream, itsNumPix);
    } else if (type == "XAV") {
        column.printEntry(stream, itsXaverage);
    } else if (type == "YAV") {
        column.printEntry(stream, itsYaverage);
    } else if (type == "XCENT") {
        column.printEntry(stream, itsXcentroid);
    } else if (type == "YCENT") {
        column.printEntry(stream, itsYcentroid);
    } else if (type == "XPEAK") {
        column.printEntry(stream, itsXpeak);
    } else if (type == "YPEAK") {
        column.printEntry(stream, itsYpeak);
    } else if (type == "FLAG1") {
        column.printEntry(stream, itsFlag1);
    } else if (type == "FLAG2") {
        column.printEntry(stream, itsFlag2);
    } else if (type == "FLAG3") {
        column.printEntry(stream, itsFlag3);
    } else if (type == "FLAG4") {
        column.printEntry(stream, itsFlag4);
    } else if (type == "COMMENT") {
        column.printEntry(stream, itsComment);
    } else {
        ASKAPTHROW(AskapError,
                   "Unknown column type " << type);
    }

}

void CasdaIsland::checkCol(duchamp::Catalogues::Column &column)
{
    std::string type = column.type();
    if (type == "ID") {
        column.check(itsIslandID);
    } else if (type == "NAME") {
        column.check(itsName);
    } else if (type == "NCOMP") {
        column.check(itsNumComponents);
    } else if (type == "RA") {
        column.check(itsRAs);
    } else if (type == "DEC") {
        column.check(itsDECs);
    } else if (type == "RAJD") {
        column.check(itsRA);
    } else if (type == "DECJD") {
        column.check(itsDEC);
    } else if (type == "FREQ") {
        column.check(itsFreq);
    } else if (type == "MAJ") {
        column.check(itsMaj);
    } else if (type == "MIN") {
        column.check(itsMin);
    } else if (type == "PA") {
        column.check(itsPA);
    } else if (type == "FINT") {
        column.check(itsFluxInt);
    } else if (type == "FPEAK") {
        column.check(itsFluxPeak);
    } else if (type == "XMIN") {
        column.check(itsXmin);
    } else if (type == "XMAX") {
        column.check(itsXmax);
    } else if (type == "YMIN") {
        column.check(itsYmin);
    } else if (type == "YMAX") {
        column.check(itsYmax);
    } else if (type == "NPIX") {
        column.check(itsNumPix);
    } else if (type == "XAV") {
        column.check(itsXaverage);
    } else if (type == "YAV") {
        column.check(itsYaverage);
    } else if (type == "XCENT") {
        column.check(itsXcentroid);
    } else if (type == "YCENT") {
        column.check(itsYcentroid);
    } else if (type == "XPEAK") {
        column.check(itsXpeak);
    } else if (type == "YPEAK") {
        column.check(itsYpeak);
    } else if (type == "FLAG1") {
        column.check(itsFlag1);
    } else if (type == "FLAG2") {
        column.check(itsFlag2);
    } else if (type == "FLAG3") {
        column.check(itsFlag3);
    } else if (type == "FLAG4") {
        column.check(itsFlag4);
    } else if (type == "COMMENT") {
        column.check(itsComment);
    } else {
        ASKAPTHROW(AskapError,
                   "Unknown column type " << type);
    }

}

void CasdaIsland::checkSpec(duchamp::Catalogues::CatalogueSpecification &spec)
{
    for (size_t i = 0; i < spec.size(); i++) {
        this->checkCol(spec.column(i));
    }
}



}

}
