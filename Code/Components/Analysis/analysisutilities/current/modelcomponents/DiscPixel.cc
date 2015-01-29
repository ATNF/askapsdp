/// @file
///
/// Class to handle operations on a single pixel that makes up a
/// uniform-surface-brightness elliptical disc
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
#include <modelcomponents/DiscPixel.h>
#include <modelcomponents/Ellipse.h>
#include <vector>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
ASKAP_LOGGER(logger, ".discpixel");

namespace askap {

namespace analysisutilities {

DiscPixel::DiscPixel(double x, double y):
    itsX(x), itsY(y)
{
    itsTmin = -1.;
    itsTmax = -1.;
    itsWidth = 1.;
    itsIsEdge = false;
    itsResolutionLimit = defaultResolution;
    itsDecimationFactor = defaultDecimationFactor;
}


DiscPixel& DiscPixel::operator=(const DiscPixel& other)
{
    if (this == &other) return *this;
    itsX = other.itsX;
    itsY = other.itsY;
    itsWidth = other.itsWidth;
    itsTmin = other.itsTmin;
    itsTmax = other.itsTmax;
    itsEllipse = other.itsEllipse;
    itsResolutionLimit = other.itsResolutionLimit;
    itsIsEdge = other.itsIsEdge;
    itsDecimationFactor = other.itsDecimationFactor;
    return *this;
}

std::vector<DiscPixel> DiscPixel::decimate()
{
    int num = itsDecimationFactor * itsDecimationFactor;
    std::vector<DiscPixel> outlist(num, *this);
    double xmin = itsX - itsWidth / 2., ymin = itsY - itsWidth / 2.;
    for (int i = 0; i < num; i++) {
        outlist[i].itsWidth /= itsDecimationFactor;
        outlist[i].itsX = xmin + (i % itsDecimationFactor + 0.5) * outlist[i].itsWidth;
        outlist[i].itsY = ymin + (i / itsDecimationFactor + 0.5) * outlist[i].itsWidth;
        outlist[i].itsIsEdge = false;
        outlist[i].itsTmin = -1.;
        outlist[i].itsTmax = -1.;
    }
    // ASKAPLOG_DEBUG_STR(logger, "Returning decimated sublist of length " << outlist.size() << " (should be " << num<<"). The width of the output pixels is " << outlist[0].itsWidth);
    return outlist;
}

double DiscPixel::flux()
{

    if (!itsIsEdge) {
        if (itsEllipse->isIn(itsX, itsY)) return itsWidth * itsWidth;
        else return 0.;
    } else {
        if (itsWidth < itsResolutionLimit) { // stopping condition
            int nVerticesGood = 0;
            if (itsEllipse->isIn(itsX + itsWidth / 2.,
                                 itsY + itsWidth / 2.))
                nVerticesGood++;
            if (itsEllipse->isIn(itsX - itsWidth / 2.,
                                 itsY + itsWidth / 2.))
                nVerticesGood++;
            if (itsEllipse->isIn(itsX + itsWidth / 2.,
                                 itsY - itsWidth / 2.))
                nVerticesGood++;
            if (itsEllipse->isIn(itsX - itsWidth / 2.,
                                 itsY - itsWidth / 2.))
                nVerticesGood++;
            return nVerticesGood * itsWidth * itsWidth / 4.;
        } else {
            std::vector<DiscPixel> subpixels = this->processedSublist();
            // ASKAPLOG_DEBUG_STR(logger, "Got list of subpixels of length " << subpixels.size());
            double flux = 0;
            for (size_t i = 0; i < subpixels.size(); i++) flux += subpixels[i].flux();
            return flux;
        }

    }


}

std::vector<DiscPixel> DiscPixel::processedSublist()
{

    std::vector<DiscPixel> subpixels = this->decimate();
    if (itsTmin > itsTmax) itsTmax += 2.*M_PI;

    // min & max values for x & y for this pixel
    double xmin = itsX - itsWidth / 2.;
    double ymin = itsY - itsWidth / 2.;
    double xmax = itsX + itsWidth / 2.;
    double ymax = itsY + itsWidth / 2.;
    double pixstep = subpixels[0].itsWidth;
    int oldx = 0, oldy = 0;
    size_t oldpos = 0;
    double tstep = (itsTmax - itsTmin) / defaultTresolution;
    for (int it = 0; it < int(defaultTresolution) + 1; it++) {
        double t = itsTmin + it * tstep;
        std::pair<double, double> pos = itsEllipse->parametric(t);
        // only consider points within the pixel. Ignore those on the border.
        if (pos.first > xmin && pos.second > ymin && pos.first < xmax && pos.second < ymax) {
            int xloc = lround((pos.first - xmin - pixstep / 2.) / pixstep);
            int yloc = lround((pos.second - ymin - pixstep / 2.) / pixstep);
            oldpos = oldx + oldy * itsDecimationFactor;
            size_t newpos = xloc + yloc * itsDecimationFactor;
            ASKAPCHECK(newpos < subpixels.size(),
                       "DiscPixel::processedSublist: current position " << newpos <<
                       " out of range (" << subpixels.size() << "). Have xloc=" <<
                       xloc << " (" << (pos.first - xmin - pixstep / 2.) / pixstep <<
                       "), yloc=" << yloc << "(" <<
                       (pos.second - ymin - pixstep / 2.) / pixstep << "), dim=" <<
                       itsDecimationFactor);
            if (xloc != oldx || yloc != oldy || it == 0) {
                if (it > 0) subpixels[oldpos].addTmax(t);
                oldx = xloc;
                oldy = yloc;
                subpixels[newpos].addTmin(t - tstep);
            }
            subpixels[newpos].itsIsEdge = true;
        }
    }
    subpixels[oldpos].addTmax(itsTmax);

    return subpixels;

}



}

}
