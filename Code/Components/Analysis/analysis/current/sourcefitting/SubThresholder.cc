/// @file
///
/// XXX Notes on program XXX
///
/// @copyright (c) 2010 CSIRO
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

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/SubThresholder.h>
#include <sourcefitting/RadioSource.h>
#include <sourcefitting/Component.h>

#include <duchamp/Cubes/cubes.hh>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>

#include <boost/scoped_ptr.hpp>

#include <math.h>
#include <vector>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".subthresholder");

namespace askap {

namespace analysis {

namespace sourcefitting {

SubThresholder::~SubThresholder()
{
}

// SubThresholder::SubThresholder(const SubThresholder &s) {
//     operator=(s);
// }
		  
// SubThresholder& SubThresholder::operator=(const SubThresholder &s) {
//     if(this == &s) return *this;
//     this->itsFirstGuess = s.itsFirstGuess;
//     this->itsSourceBox = s.itsSourceBox;
//     this->itsBaseThreshold = s.itsBaseThreshold;
//     this->itsThreshIncrement = s.itsThreshIncrement;
//     this->itsPeakFlux = s.itsPeakFlux;
//     this->itsSourceSize = s.itsSourceSize;
//     this->itsDim = s.itsDim;
//     this->itsFluxArray = s.itsFluxArray;
//     this->itsCurrentThreshold = s.itsCurrentThreshold;
//     this->itsFitParams = s.itsFitParams;
//     return *this;

// }

void SubThresholder::define(RadioSource &src,
                            casa::Matrix<casa::Double> pos,
                            casa::Vector<casa::Double> &array)
{
    this->saveArray(src, pos, array);
    this->define(src);
}


void SubThresholder::saveArray(RadioSource &src,
                               casa::Matrix<casa::Double> pos,
                               casa::Vector<casa::Double> &f)
{
    int xmin = src.boxXmin();
    int ymin = src.boxYmin();
    int xsize = src.boxXsize();
    int ysize = src.boxYsize();
    size_t size = xsize * ysize;
    itsFluxArray = std::vector<float>(size, 0.);
    PixelInfo::Object2D spatMap = src.getSpatialMap();
    for (size_t i = 0; i < f.size(); i++) {
        int x = int(pos(i, 0));
        int y = int(pos(i, 1));

        if (spatMap.isInObject(x, y)) {
            int loc = (x - xmin) + xsize * (y - ymin);
            itsFluxArray[loc] = float(f(i));
        }
    }

}


void SubThresholder::define(RadioSource &src)
{

    itsPeakFlux = src.getPeakFlux();
    itsSourceSize = src.getSize();

    itsDim = std::vector<size_t>(2);
    itsDim[0] = src.boxXsize();
    itsDim[1] = src.boxYsize();

    this->setFirstGuess(src);
    itsFitParams = src.fitparams();
    itsSourceBox = src.box();

    if (itsFitParams.flagLogarithmicIncrements()) {
        itsBaseThreshold = src.detectionThreshold() > 0 ?
                                 log10(src.detectionThreshold()) : -6.;
        itsThreshIncrement = (log10(itsPeakFlux) - itsBaseThreshold) /
                                   float(itsFitParams.numSubThresholds() + 1);
        itsCurrentThreshold = pow(10., itsBaseThreshold + itsThreshIncrement);
    } else {
        itsBaseThreshold = src.detectionThreshold();
        itsThreshIncrement = (itsPeakFlux - itsBaseThreshold) /
                                   float(itsFitParams.numSubThresholds() + 1);
        itsCurrentThreshold = itsBaseThreshold + itsThreshIncrement;
    }

}

void SubThresholder::setFirstGuess(RadioSource &src)
{

    itsFirstGuess.setPeak(src.getPeakFlux());
    itsFirstGuess.setX(src.getXPeak());
    itsFirstGuess.setY(src.getYPeak());
    double a, b, c;

    if (src.getSize() < 3) {
        itsFirstGuess.setPA(0);
        itsFirstGuess.setMajor(1.);
        itsFirstGuess.setMinor(1.);
    } else {
        src.getFWHMestimate(itsFluxArray, a, b, c);
        itsFirstGuess.setPA(a);
        itsFirstGuess.setMajor(b);
        itsFirstGuess.setMinor(c);
    }


}


void SubThresholder::keepObject(PixelInfo::Object2D &obj)
{

    for (size_t i = 0; i < itsDim[0]*itsDim[1]; i++) {
        int xbox = i % itsDim[0];
        int ybox = i / itsDim[0];

        if (!obj.isInObject(xbox, ybox)) {
            itsFluxArray[i] = 0.;
        }
    }

}

void SubThresholder::incrementThreshold()
{

    if (itsFitParams.flagLogarithmicIncrements()) {
        itsCurrentThreshold *= pow(10., itsThreshIncrement);
    } else {
        itsCurrentThreshold += itsThreshIncrement;
    }

}


std::vector<SubComponent> SubThresholder::find()
{

    std::vector<SubComponent> fullList;

    if (itsSourceSize < 3) {
        fullList.push_back(itsFirstGuess);
        return fullList;
    }

    std::vector<PixelInfo::Object2D> objlist;
    std::vector<PixelInfo::Object2D>::iterator obj;
    bool keepGoing = true;

    boost::scoped_ptr<duchamp::Image> theImage(new duchamp::Image(itsDim.data()));

    if (itsFluxArray.size() > 0) {
        ASKAPCHECK(itsFluxArray.size() == (itsDim[0]*itsDim[1]),
                   "Size of flux array (" << itsFluxArray.size() <<
                   ") doesn't match dimension (" << itsDim[0] <<
                   "x" << itsDim[1] << "=" << itsDim[0]*itsDim[1] << ")!");
        theImage->saveArray(&(itsFluxArray[0]), itsFluxArray.size());
    }
    theImage->setMinSize(1);
    theImage->pars().setFlagUserThreshold(true);

    while (itsCurrentThreshold <= itsPeakFlux && keepGoing) {
        theImage->stats().setThreshold(itsCurrentThreshold);
        theImage->pars().setThreshold(itsCurrentThreshold);
        objlist = theImage->findSources2D();
        keepGoing = (objlist.size() == 1);
        this->incrementThreshold();
    }

    if (!keepGoing) {

        if (objlist.size() == 0) {
            fullList.push_back(itsFirstGuess);
        } else {

            for (obj = objlist.begin(); obj < objlist.end(); obj++) {

                RadioSource src;
                src.addChannel(0, *obj);
                src.setFitParams(itsFitParams);
                src.setDetectionThreshold(itsCurrentThreshold);
                src.setBox(itsSourceBox);
                src.calcFluxes(&(itsFluxArray[0]), &(itsDim[0]));
                duchamp::Param par;
                par.setXOffset(itsSourceBox.start()[0]);
                par.setYOffset(itsSourceBox.start()[1]);
                src.setOffsets(par);
                src.addOffsets();
                SubThresholder newthresher(*this);
                newthresher.setFirstGuess(src);
                newthresher.keepObject(*obj);
                std::vector<SubComponent> newlist = newthresher.find();
                for (uInt i = 0; i < newlist.size(); i++) {
                    fullList.push_back(newlist[i]);
                }
            }
        }
    } else {
        fullList.push_back(itsFirstGuess);
    }

    if (fullList.size() > 1) {
        std::sort(fullList.begin(), fullList.end());
        std::reverse(fullList.begin(), fullList.end());
    }

    return fullList;

}

}
}

}
