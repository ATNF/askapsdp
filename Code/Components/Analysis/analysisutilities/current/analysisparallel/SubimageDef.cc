/// @file
///
/// @brief Define and access subimages of a FITS file.
///
/// @copyright (c) 2007 CSIRO
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///

#include <askap_analysisutilities.h>
#include <analysisparallel/SubimageDef.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <askapparallel/AskapParallel.h>

#include <gsl/gsl_sf_gamma.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <set>

#include <casa/aipstype.h>
#include <images/Images/FITSImage.h>
#include <images/Images/ImageOpener.h>

#include <duchamp/fitsHeader.hh>
#include <duchamp/Utils/Statistics.hh>
#include <duchamp/Utils/Section.hh>
#include <duchamp/param.hh>
#include <duchamp/Outputs/KarmaAnnotationWriter.hh>

using namespace casa;

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".subimagedef");

namespace askap {
namespace analysisutilities {

SubimageDef::SubimageDef()
{
    itsNAxis = 0;
    itsNSubX = 1;
    itsNSubY = 1;
    itsNSubZ = 1;
    itsOverlapX = itsOverlapY = itsOverlapZ = 0;
    itsImageName = "";
    itsInputSection = "";
    itsNSub = std::vector<unsigned int>();
    itsOverlap = std::vector<unsigned int>();
    itsSectionList = std::vector<duchamp::Section>();
}

SubimageDef::SubimageDef(const LOFAR::ParameterSet& parset)
{
    itsNAxis = 0;
    itsImageName = parset.getString("image", "");
    itsNSubX = parset.getInt16("nsubx", 1);
    itsNSubY = parset.getInt16("nsuby", 1);
    itsNSubZ = parset.getInt16("nsubz", 1);
    itsOverlapX = parset.getInt16("overlapx", 0);
    itsOverlapY = parset.getInt16("overlapy", 0);
    itsOverlapZ = parset.getInt16("overlapz", 0);
    bool flagSub = parset.getBool("flagsubsection", false);
    if (flagSub) {
        itsInputSection = parset.getString("subsection", "");
    }

    itsAnnotationFile = parset.getString("subimageAnnotationFile",
                                         "selavy-SubimageLocations.ann");

    ASKAPLOG_DEBUG_STR(logger, "Defined subimageDef, subdivided " << itsNSubX << "x"
                       << itsNSubY << "x" << itsNSubZ << " with overlaps "
                       << itsOverlapX << "," << itsOverlapY << "," << itsOverlapZ);

}

void SubimageDef::setImageDim(std::vector<int> dim)
{
    itsFullImageDim = std::vector<long>(dim.size());
    for (size_t i = 0; i < dim.size(); i++) {
        itsFullImageDim[i] = dim[i];
    }
}

void SubimageDef::setImageDim(std::vector<long> dim)
{
    itsFullImageDim = dim;
}

void SubimageDef::setImageDim(std::vector<size_t> dim)
{
    itsFullImageDim = std::vector<long>(dim.size());
    for (size_t i = 0; i < dim.size(); i++) {
        itsFullImageDim[i] = dim[i];
    }
}

void SubimageDef::setImageDim(long *dim, size_t size)
{
    itsFullImageDim = std::vector<long>(size);
    for (size_t i = 0; i < size; i++) {
        itsFullImageDim[i] = dim[i];
    }
}

void SubimageDef::setImageDim(size_t *dim, size_t size)
{
    itsFullImageDim = std::vector<long>(size);
    for (size_t i = 0; i < size; i++) {
        itsFullImageDim[i] = dim[i];
    }
}

void SubimageDef::define(int numDim)
{

    struct wcsprm *wcs;
    wcs = (struct wcsprm *)calloc(1, sizeof(struct wcsprm));
    wcs->naxis = numDim;
    wcs->lng = 0;
    wcs->lat = 1;
    wcs->spec = 2;
    define(wcs);
    int nwcs = 1;
    wcsvfree(&nwcs, &wcs);

}

void SubimageDef::define(wcsprm *wcs)
{
    /// @details Define all the necessary variables within the
    /// SubimageDef class. The image (given by the parameter "image"
    /// in the parset) is to be split up according to the nsubx/y/z
    /// parameters, with overlaps in each direction given by the
    /// overlapx/y/z parameters (these are in pixels).
    ///
    /// The WCS parameters in wcs determine which axes are the x, y and z
    /// axes. The number of axes is also determined from the WCS
    /// parameter set.
    ///
    /// @param wcs The WCSLIB definition of the world coordinate system
    itsNAxis = wcs->naxis;
    itsLng  = wcs->lng;
    itsLat  = wcs->lat;
    itsSpec = wcs->spec;

    if (itsNAxis > 0) {
        itsNSub = std::vector<unsigned int>(itsNAxis, 1);
        itsOverlap = std::vector<unsigned int>(itsNAxis, 0);

        for (int i = 0; i < itsNAxis; i++) {
            if (i == itsLng) {
                itsNSub[i] = itsNSubX;
                itsOverlap[i] = itsOverlapX;
            } else if (i == itsLat) {
                itsNSub[i] = itsNSubY;
                itsOverlap[i] = itsOverlapY;
            } else if (i == itsSpec) {
                itsNSub[i] = itsNSubZ;
                itsOverlap[i] = itsOverlapZ;
            }

        }
    }

}

void SubimageDef::defineFITS(std::string FITSfilename)
{
    /// @details Define all the necessary variables within the
    /// SubimageDef class. The image (given by the parameter "image"
    /// in the parset) is to be split up according to the nsubx/y/z
    /// parameters, with overlaps in each direction given by the
    /// overlapx/y/z parameters (these are in pixels).
    ///
    /// This version is designed for FITS files. The Duchamp
    /// function duchamp::FitsHeader::defineWCS() is used to extract
    /// the WCS parameters from the FITS header. This is then sent
    /// to SubimageDef::define(wcsprm *) to define everything.
    ///
    /// @param FITSfilename The name of the FITS file.

    // This is needed for defineWCS(), but we don't care about the contents.
    duchamp::Param tempPar;
    duchamp::FitsHeader imageHeader;
    itsImageName = FITSfilename;
    imageHeader.defineWCS(itsImageName, tempPar);
    define(imageHeader.getWCS());
}

void SubimageDef::defineAllSections()
{
    if (itsFullImageDim.size() == 0) {
        ASKAPTHROW(AskapError,
                   "SubimageDef::defineAllSections : image dimensions have not been set!");
    }
    if (itsInputSection == "") {
        ASKAPLOG_WARN_STR(logger,
                          "SubimageDef::defineAllSections : input subsection not defined! " <<
                          "Setting to null subsection");
        itsInputSection = duchamp::nullSection(itsFullImageDim.size());
    }
    duchamp::Section inputSec(itsInputSection);
    inputSec.parse(itsFullImageDim);

    int nSub = itsNSubX * itsNSubY * itsNSubZ;
    itsSectionList = std::vector<duchamp::Section>(nSub);

    for (int i = 0; i < nSub; i++) {
        itsSectionList[i] = section(i);
    }


}

casa::IPosition SubimageDef::blc(int workerNum)
{
    duchamp::Section subsection = section(workerNum);
    casa::IPosition blc(subsection.getStartList());
    return blc;
}

duchamp::Section SubimageDef::section(int workerNum)
{
    /// @details Return the subsection object for the given worker
    /// number. (These start at 0). The subimages are tiled across
    /// the cube with the x-direction varying quickest, then y, then
    /// z.
    /// @return A duchamp::Section object containing all information
    /// on the subsection.

    if (itsFullImageDim.size() == 0) {
        ASKAPTHROW(AskapError, "SubimageDef::section : " <<
                   " tried to define a section but the image dimensions have not been set!");
    }

    if (workerNum < 0) {
        return itsInputSection;

    } else {
        if (itsInputSection == "") {
            ASKAPLOG_WARN_STR(logger, "SubimageDef::section : " <<
                              "input subsection not defined! Setting to null subsection");
            itsInputSection = duchamp::nullSection(itsFullImageDim.size());
        }
        duchamp::Section inputSec(itsInputSection);
        inputSec.parse(itsFullImageDim);
        std::vector<long> sub(itsNAxis, 0);

        sub[itsLng] = workerNum % itsNSub[0];
        sub[itsLat] = (workerNum % (itsNSub[0] * itsNSub[1])) / itsNSub[0];
        if (itsSpec >= 0)
            sub[itsSpec] = workerNum / (itsNSub[0] * itsNSub[1]);
        std::stringstream section;

        for (int i = 0; i < itsNAxis; i++) {
            if (itsNSub[i] > 1) {
                int length = inputSec.getDim(i);
                float sublength = float(length) / float(itsNSub[i]);
                int min = std::max(long(inputSec.getStart(i)),
                                   long(inputSec.getStart(i) +
                                        sub[i] * sublength - itsOverlap[i] / 2)) + 1;
                int max = std::min(long(inputSec.getEnd(i) + 1),
                                   long(inputSec.getStart(i) +
                                        (sub[i] + 1) * sublength + itsOverlap[i] / 2));
                section << min << ":" << max;
            } else
                section << inputSec.getSection(i);

            if (i != itsNAxis - 1) section << ",";
        }

        std::string secstring = "[" + section.str() + "]";
        duchamp::Section sec(secstring);
        sec.parse(itsFullImageDim);
        return sec;
    }
}

void SubimageDef::writeAnnotationFile(duchamp::FitsHeader &head,
                                      askap::askapparallel::AskapParallel& comms)
{
    /// @details This creates a Karma annotation file that simply has
    /// the borders of the subimages plotted on it.

    if (itsInputSection == "") {
        ASKAPLOG_WARN_STR(logger,
                          "SubimageDef::defineAllSections : \
input subsection not defined! Setting to null subsection");
        itsInputSection = duchamp::nullSection(itsFullImageDim.size());
    }
    std::stringstream dimss;
    for (size_t i = 0; i < itsFullImageDim.size() - 1; i++) {
        dimss << itsFullImageDim[i] << "x";
    }
    dimss << itsFullImageDim.back();
    ASKAPLOG_INFO_STR(logger, "Input subsection to be used is " << itsInputSection <<
                      " with dimensions " << dimss.str());
    duchamp::Section fullImageSubsection(itsInputSection);
    fullImageSubsection.parse(itsFullImageDim);

    duchamp::KarmaAnnotationWriter writer(itsAnnotationFile);
    writer.openCatalogue();
    if (writer.isOpen()) {
        ASKAPLOG_INFO_STR(logger, "Writing annotation file showing subimages to " <<
                          writer.name());
    } else {
        ASKAPLOG_WARN_STR(logger, "Could not open " << writer.name() <<
                          " for writing subimage outlines");
    }
    writer.setColourString("YELLOW");
    writer.writeTableHeader();

    double *pix = new double[12];
    double *wld = new double[12];
    float xcentre, ycentre;

    for (int i = 0; i < 4; i++) {
        pix[i * 3 + 2] = 0;
    }

    for (int w = 0; w < comms.nProcs() - 1; w++) {

        duchamp::Section workerSection = section(w);

        // x-start, in pixels relative to the image that has been read :
        pix[0] = pix[9] =  workerSection.getStart(0) - 0.5 - fullImageSubsection.getStart(0);
        // y-start :
        pix[1] = pix[4] =  workerSection.getStart(1) - 0.5 - fullImageSubsection.getStart(1);
        // x-end :
        pix[3] = pix[6] =  workerSection.getEnd(0)  + 0.5 - fullImageSubsection.getStart(0);
        // y-end :
        pix[7] = pix[10] = workerSection.getEnd(1)  + 0.5 - fullImageSubsection.getStart(1);

        head.pixToWCS(pix, wld, 4);
        xcentre = (wld[0] + wld[3] + wld[6] + wld[9]) / 4.;
        ycentre = (wld[1] + wld[4] + wld[7] + wld[10]) / 4.;

        std::vector<double> x, y;
        for (int i = 0; i <= 4; i++) {
            x.push_back(wld[(i % 4) * 3]);
            y.push_back(wld[(i % 4) * 3 + 1]);
        }
        writer.joinTheDots(x, y);
        std::stringstream ss;
        ss << w + 1;
        writer.text(xcentre, ycentre, ss.str());
    }

    delete [] pix;
    delete [] wld;

    writer.closeCatalogue();

}

std::set<int> SubimageDef::affectedWorkers(int x, int y, int z)
{
    if (itsFullImageDim.size() == 0) {
        ASKAPTHROW(AskapError, "SubimageDef::affectedWorkers : image dimensions have not been set!");
    }
    if (itsSectionList.size() == 0) {
        ASKAPTHROW(AskapError, "SubimageDef::affectedWorkers : worker sections have not been defined!");
    }

    int ref[3] = {x, y, z};
    int axID[3] = {itsLng, itsLat, itsSpec};
    std::set<int> goodNodes; // use sets, as can have more than one node that the pixel falls in

    int nsub = itsNSubX * itsNSubY * itsNSubZ;
    for (int n = 0; n < nsub; n++) {
        bool isIn = true;
        for (int i = 0; i < 3; i++) {
            if (axID[i] >= 0)
                isIn = isIn &&
                       (ref[i] >= itsSectionList[n].getStart(axID[i])) &&
                       (ref[i] <= itsSectionList[n].getEnd(axID[i]));
        }
        if (isIn) goodNodes.insert(n);
    }

    return goodNodes;
}

std::set<int> SubimageDef::affectedWorkers(float x, float y, float z)
{
    return affectedWorkers(int(floor(x)), int(floor(y)), int(floor(z)));
}

std::set<int> SubimageDef::affectedWorkers(casa::IPosition pos)
{
    ASKAPASSERT(pos.size() >= 3);
    return affectedWorkers(int(pos[0]), int(pos[1]), int(pos[2]));
}

std::set<int> SubimageDef::affectedWorkers(casa::Slicer &slice)
{

    IPosition blc = slice.start();
    IPosition trc = slice.end();
    std::set<int> start = affectedWorkers(blc);
    std::set<int> end = affectedWorkers(trc);
    std::set<int> result;
    // want all nodes in rectangular pattern from minimum of start to maximum of end
    int xmin = *(start.begin()) % itsNSubX;
    int ymin = *(start.begin()) / itsNSubX;
    int zmin = *(start.begin()) % (itsNSubX * itsNSubY);
    int xmax = *(end.begin()) % itsNSubX;
    int ymax = *(end.begin()) / itsNSubX;
    int zmax = *(end.begin()) % (itsNSubX * itsNSubY);
    for (int i = *(start.begin()); i <= *(end.rbegin()); i++) {
        int xpos = i % itsNSubX;
        int ypos = i / itsNSubX;
        int zpos = i % (itsNSubX * itsNSubY);
        if ((xpos >= xmin && xpos <= xmax) && (ypos >= ymin && ypos <= ymax) && (zpos >= zmin && zpos <= zmax)) result.insert(i);
    }
    return result;

}


}

}
