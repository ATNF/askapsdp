/// @file
///
/// XXX Notes on program XXX
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
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#include <askap_analysis.h>
#include <patternmatching/PointCatalogue.h>
#include <patternmatching/Point.h>
#include <patternmatching/Triangle.h>
#include <modelcomponents/ModelFactory.h>
#include <modelcomponents/Spectrum.h>
#include <coordutils/PositionUtilities.h>
#include <casainterface/CasaInterface.h>

#include <images/Images/ImageInterface.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <casa/Arrays/Vector.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <casa/Quanta.h>

#include <Common/ParameterSet.h>
#include <boost/shared_ptr.hpp>

#include <vector>
ASKAP_LOGGER(logger, ".matching.PointCat");

namespace askap {

namespace analysis {

namespace matching {

PointCatalogue::PointCatalogue():
    itsFilename(""),
    itsTrimSize(0),
    itsRatioLimit(defaultRatioLimit),
    itsFlagOffsetPositions(false),
    itsRAref(0.),
    itsDECref(0.),
    itsRadius(0.)
{
    itsFullPointList = std::vector<Point>(0);
    itsWorkingPointList = std::vector<Point>(0);
    itsTriangleList = std::vector<Triangle>(0);
}

PointCatalogue::PointCatalogue(LOFAR::ParameterSet &parset)
{
    itsFilename = parset.getString("filename", "");
    ASKAPCHECK(itsFilename != "", "No filename provided for the catalogue.");
    if (parset.getString("database", "Continuum") == "Selavy") {
        parset.replace("useDeconvolvedSizes", "true");
        // set this to true, just so we don't have to worry about the SelavyImage
    }
    itsFactory = analysisutilities::ModelFactory(parset);
    itsTrimSize = parset.getUint32("trimsize", 0);
    if (itsTrimSize <= 2) {
        ASKAPLOG_WARN_STR(logger, "Since trimsize<=2, the entire point list " <<
                          "will be used to generate triangles.");
    }
    itsRatioLimit = parset.getFloat("ratioLimit", defaultRatioLimit);
    itsFullPointList = std::vector<Point>(0);
    itsWorkingPointList = std::vector<Point>(0);
    itsTriangleList = std::vector<Triangle>(0);
    std::string raRef = parset.getString("raRef", "");
    std::string decRef = parset.getString("decRef", "");
    itsFlagOffsetPositions = (raRef != "" && decRef != "");
    if (itsFlagOffsetPositions) {
        itsRAref = analysisutilities::raToDouble(raRef);
        itsDECref = analysisutilities::decToDouble(decRef);
        ASKAPLOG_DEBUG_STR(logger,
                           "Using reference position (RA,DEC)=(" <<
                           itsRAref << "," << itsDECref << ")");
    } else {
        if (raRef != "" || decRef != "") {
            ASKAPLOG_WARN_STR(logger, "To offset positions, you need to provide both " <<
                              "raRef and decRef parameters");
        }
    }
    itsRadius = parset.getDouble("radius", -1.);
    itsReferenceImage = parset.getString("referenceImage", "");
}

bool PointCatalogue::read()
{
    std::ifstream fin(itsFilename.c_str());
    itsFullPointList = std::vector<Point>(0);
    if (!fin.is_open()) {
        ASKAPLOG_WARN_STR(logger, "Could not open filename " << itsFilename << ".");
        return false;
    } else {
        std::string line;
        casa::DirectionCoordinate dirCoo;
        int ndim = 0;
        if (itsReferenceImage != "") {
            const boost::shared_ptr<ImageInterface<Float> > imagePtr =
                analysisutilities::openImage(itsReferenceImage);

            int dirCooNum = imagePtr->coordinates().findCoordinate(casa::Coordinate::DIRECTION);
            dirCoo = imagePtr->coordinates().directionCoordinate(dirCooNum);

            ndim = imagePtr->ndim();

        }

        ASKAPLOG_DEBUG_STR(logger, "Reading catalogue from file " << itsFilename);
        while (getline(fin, line),
                !fin.eof()) {

            if (line[0] != '#') {  // ignore commented lines
                boost::shared_ptr<analysisutilities::Spectrum> spec = itsFactory.read(line);
                size_t listSize = itsFullPointList.size();
                Point newpoint(spec);
                if (itsFlagOffsetPositions) {
                    double radius =
                        analysisutilities::angularSeparation(itsRAref, itsDECref,
                                spec->raD(), spec->decD());
                    if (itsRadius < 0. || radius < itsRadius) {
                        itsFullPointList.push_back(newpoint);
                    }
                } else itsFullPointList.push_back(newpoint);
                if (itsFullPointList.size() > listSize && itsReferenceImage != "") {
                    casa::Vector<double> pix(ndim, 0), world(ndim, 0);
                    casa::Quantity ra(spec->raD(), "deg"), dec(spec->decD(), "deg");
                    world[0] = ra.getValue(dirCoo.worldAxisUnits()[0]);
                    world[1] = dec.getValue(dirCoo.worldAxisUnits()[1]);
                    dirCoo.toPixel(pix, world);
                    itsFullPointList.back().setX(pix[0]);
                    itsFullPointList.back().setY(pix[1]);
                }

            }
        }
    }
    itsWorkingPointList = itsFullPointList;
    this->makeTriangleList();
    return true;
}

void PointCatalogue::makeTriangleList()
{
    std::sort(itsWorkingPointList.begin(), itsWorkingPointList.end());
    std::reverse(itsWorkingPointList.begin(), itsWorkingPointList.end());
    size_t maxPoint = itsWorkingPointList.size();
    if (itsTrimSize > 2) {
        maxPoint = std::min(itsTrimSize, itsWorkingPointList.size());
    }

    ASKAPLOG_DEBUG_STR(logger, "Sorted the list of " << itsWorkingPointList.size() <<
                       " point and using the first " << maxPoint << " to generate triangles");
    ASKAPLOG_DEBUG_STR(logger, "First of list has flux " << itsWorkingPointList[0].flux());
    ASKAPLOG_DEBUG_STR(logger, "Second of list has flux " << itsWorkingPointList[1].flux());

    itsTriangleList = std::vector<Triangle>(0);
    for (size_t i = 0; i < maxPoint - 2; i++) {
        for (size_t j = i + 1; j < maxPoint - 1; j++) {
            for (size_t k = j + 1; k < maxPoint; k++) {
                Triangle tri(itsWorkingPointList[i],
                             itsWorkingPointList[j],
                             itsWorkingPointList[k]);

                if (tri.ratio() < itsRatioLimit) itsTriangleList.push_back(tri);
            }
        }
    }

    ASKAPLOG_INFO_STR(logger, "Generated a list of " << itsTriangleList.size() << " triangles");

}

bool PointCatalogue::crudeMatch(std::vector<Point> &other, double maxSep)
{
    ASKAPLOG_DEBUG_STR(logger, "Performing crude match with maximum separation = " << maxSep);
    std::vector<Point>::iterator mine, theirs;
    itsWorkingPointList = std::vector<Point>(0);
    for (mine = itsFullPointList.begin(); mine < itsFullPointList.end(); mine++) {
        bool stop = false;
        for (theirs = other.begin(); theirs < other.end() && !stop; theirs++) {

            if (theirs->sep(*mine) < maxSep) {
                itsWorkingPointList.push_back(*mine);
                ASKAPLOG_DEBUG_STR(logger, "crude match: (" <<
                                   theirs->ID() << ": " << theirs->x() << "," << theirs->y() <<
                                   ") <-> (" <<
                                   mine->ID() << ": " << mine->x() << "," << mine->y() << ")");
                stop = true;
            }
        }

    }

    bool matchWorked = (itsWorkingPointList.size() > 0);
    if (matchWorked) {
        ASKAPLOG_DEBUG_STR(logger, "Reduced list from " << itsFullPointList.size() <<
                           " points to " << itsWorkingPointList.size() << " points");
        this->makeTriangleList();
    } else {
        ASKAPLOG_WARN_STR(logger, "Crude matching of point lists did not return any matches");
        itsWorkingPointList = itsFullPointList;
    }

    return matchWorked;

}

}

}

}
