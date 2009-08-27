/// @file
///
/// @copyright (c) 2008 CSIRO
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

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/RadioSource.h>
#include <sourcefitting/Fitter.h>
#include <sourcefitting/FittingParameters.h>
#include <sourcefitting/FitResults.h>
#include <sourcefitting/Component.h>
#include <analysisutilities/AnalysisUtilities.h>
#include <analysisutilities/SubimageDef.h>

#include <duchamp/fitsHeader.hh>
#include <duchamp/PixelMap/Voxel.hh>
#include <duchamp/PixelMap/Object2D.hh>
#include <duchamp/PixelMap/Object3D.hh>
#include <duchamp/Cubes/cubes.hh>
#include <duchamp/Detection/detection.hh>
#include <duchamp/Detection/columns.hh>
#include <duchamp/Utils/Section.hh>
#include <duchamp/Utils/utils.hh>

#include <scimath/Fitting/FitGaussian.h>
#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Gaussian2D.h>
#include <scimath/Functionals/Gaussian3D.h>
#include <casa/namespace.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <utility>
#include <math.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".sourcefitting");

using namespace duchamp;

namespace askap {

    namespace analysis {

        namespace sourcefitting {

            RadioSource::RadioSource():
                    duchamp::Detection()
            {
                this->hasFit = false;
                this->atEdge = false;
                this->itsNoiseLevel = 1.;
            }

            RadioSource::RadioSource(duchamp::Detection obj):
                    duchamp::Detection(obj)
            {
                this->hasFit = false;
                this->atEdge = false;
                this->itsNoiseLevel = 1.;
            }

            RadioSource::RadioSource(const RadioSource& src):
                    duchamp::Detection(src)
            {
                operator=(src);
            }

            //**************************************************************//

            RadioSource& RadioSource::operator= (const RadioSource& src)
            {
                ((duchamp::Detection &) *this) = src;
                this->atEdge = src.atEdge;
                this->hasFit = src.hasFit;
                this->itsNoiseLevel = src.itsNoiseLevel;
                this->itsDetectionThreshold = src.itsDetectionThreshold;
                this->itsHeader = src.itsHeader;
                this->itsBoxMargins = src.itsBoxMargins;
                this->itsFitParams = src.itsFitParams;
                this->itsBestFit = src.itsBestFit;
                this->itsBestFitFULL = src.itsBestFitFULL;
                this->itsBestFitPSF = src.itsBestFitPSF;
                this->itsBestFitSHAPE = src.itsBestFitSHAPE;
                return *this;
            }

            //**************************************************************//

            void RadioSource::defineBox(duchamp::Section &sec, FittingParameters &fitParams)
            {
                /// @details Defines the maximum and minimum points of the box
                /// in each axis direction. The size of the image array is
                /// taken into account, using the axes array, so that the box
                /// does not go outside the allowed pixel area.
                this->itsBoxMargins.clear();
//  long zero = 0;
//  ASKAPLOG_DEBUG_STR(logger, "RadioSource::defineBox : "
//             << this->getXmin() << " " << this->getXmax() <<" "
//             << this->getYmin() << " " << this->getYmax() << "   "
//             << fitParams.boxPadSize() << "   "
//             << sec.getSection() << ": "
//             << sec.getStart(0) << " " << sec.getStart(1) << " " << sec.getStart(2) << " "
//             << sec.getEnd(0) << " " << sec.getEnd(1) << " " << sec.getEnd(2) << "   "
//             << this->xSubOffset << " " << this->ySubOffset << " " << this->zSubOffset
//             );
//  long xmin = std::max(zero, this->getXmin() - fitParams.boxPadSize());
//  long xmax = std::min(axes[0], this->getXmax() + fitParams.boxPadSize());
//      long ymin = std::max(zero, this->getYmin() - fitParams.boxPadSize());
//      long ymax = std::min(axes[1], this->getYmax() + fitParams.boxPadSize());
//      long zmin = std::max(zero, this->getZmin() - fitParams.boxPadSize());
//      long zmax = std::min(axes[2], this->getZmax() + fitParams.boxPadSize());
                long xmin = std::max(long(sec.getStart(0) - this->xSubOffset), this->getXmin() - fitParams.boxPadSize());
                long xmax = std::min(long(sec.getEnd(0) - this->xSubOffset), this->getXmax() + fitParams.boxPadSize());
                long ymin = std::max(long(sec.getStart(1) - this->ySubOffset), this->getYmin() - fitParams.boxPadSize());
                long ymax = std::min(long(sec.getEnd(1) - this->ySubOffset), this->getYmax() + fitParams.boxPadSize());
                long zmin = std::max(long(sec.getStart(2) - this->zSubOffset), this->getZmin() - fitParams.boxPadSize());
                long zmax = std::min(long(sec.getEnd(2) - this->zSubOffset), this->getZmax() + fitParams.boxPadSize());
                std::vector<std::pair<long, long> > vec(3);
                vec[0] = std::pair<long, long>(xmin, xmax);
                vec[1] = std::pair<long, long>(ymin, ymax);
                vec[2] = std::pair<long, long>(zmin, zmax);
                this->itsBoxMargins = vec;
//  ASKAPLOG_DEBUG_STR(logger,"RadioSource::defineBox : boxSize = " << this->boxSize() );
            }


            //**************************************************************//

	  void RadioSource::setAtEdge(duchamp::Cube &cube, SubimageDef &subimage, int workerNum)
            {
                /// @details Sets the atEdge flag based on the dimensions of
                /// the cube and the duchamp parameters flagAdjacent, threshS
                /// and threshV. If flagAdjacent is true, then the source is
                /// at the edge if it occupies a pixel on the boundary of the
                /// image (the z-direction is only examined if there is more
                /// than one channel). If flagAdjacent, the source must lie
                /// within the appropriate threshold (threshS for the spatial
                /// directions and threshV for the spectral/velocity) of the
                /// image boundary.
	        ///
	        /// The image boundary here takes into account the
	        /// size of any overlap region between neighbouring
	        /// subimages, but only for image sides that have a
	        /// neighbour (for those on the edge of the full
	        /// image, the boundary is assumed to be the image
	        /// boundary).
	        ///
	        /// @param cube The duchamp::Cube object that holds the dimensions and parameters
	        /// @param subimage The SubimageDef object that holds the information on the number of subimages & their overlap.
	        /// @param workerNum The number of the worker in question, starting at 0 (which subimage are we in?)

                bool flagBoundary = false;
                bool flagAdj = cube.pars().getFlagAdjacent();
                float threshS = cube.pars().getThreshS();
                float threshV = cube.pars().getThreshV();

		long xminEdge,xmaxEdge,yminEdge,ymaxEdge,zminEdge,zmaxEdge;
		if(workerNum<0){  // if it is the Master node
		  xminEdge = yminEdge = zminEdge = 0;
		  xmaxEdge = cube.getDimX() - 1;
		  ymaxEdge = cube.getDimY() - 1;
		  zmaxEdge = cube.getDimZ() - 1;
		}
		else {
		  int *nsub = subimage.nsub();
		  int *overlap = subimage.overlap();
		  int colnum=workerNum % nsub[0];
		  int rownum=workerNum / nsub[0];
		  int znum = workerNum % nsub[0]*nsub[1];
		  xminEdge = (colnum == 0 ) ? 0 : overlap[0];
		  xmaxEdge = (colnum == nsub[0]-1 ) ? cube.getDimX() - 1 : cube.getDimX() - 1 - overlap[0];
		  yminEdge = (rownum == 0 ) ? 0 : overlap[1];
		  ymaxEdge = (rownum == nsub[1]-1 ) ? cube.getDimY() - 1 : cube.getDimY() - 1 - overlap[1];
		  zminEdge = (znum == 0 ) ? 0 : overlap[2];
		  zmaxEdge = (znum == nsub[2]-1 ) ? cube.getDimZ() - 1 : cube.getDimZ() - 1 - overlap[2];
		}

                if (flagAdj) {
                    flagBoundary = flagBoundary || (this->getXmin() == xminEdge);
                    flagBoundary = flagBoundary || (this->getXmax() == xmaxEdge);
                    flagBoundary = flagBoundary || (this->getYmin() == yminEdge);
                    flagBoundary = flagBoundary || (this->getYmax() == ymaxEdge);

                    if (cube.getDimZ() > 1) {
                        flagBoundary = flagBoundary || (this->getZmin() == zminEdge);
                        flagBoundary = flagBoundary || (this->getZmax() == zmaxEdge);
                    }
                } else {
                    flagBoundary = flagBoundary || (this->getXmin()-xminEdge < threshS);
                    flagBoundary = flagBoundary || ((xmaxEdge - this->getXmax()) < threshS);
                    flagBoundary = flagBoundary || (this->getYmin()-yminEdge < threshS);
                    flagBoundary = flagBoundary || ((ymaxEdge - this->getYmax()) < threshS);

                    if (cube.getDimZ() > 1) {
                        flagBoundary = flagBoundary || (this->getZmin()-zminEdge < threshV);
                        flagBoundary = flagBoundary || ((zmaxEdge - this->getZmax()) < threshV);
                    }
                }

                this->atEdge = flagBoundary;
            }
            //**************************************************************//

            void RadioSource::setNoiseLevel(duchamp::Cube &cube, FittingParameters &fitparams)
            {
                /// @details Sets the value of the local noise level by taking
                /// the MADFM of the surrounding pixels from the Cube's array.
                /// Calls setNoiseLevel(float *, long *, int).
                /// @param cube The duchamp::Cube object containing the pixel array
                /// @param fitparams The set of parameters governing the fit
                if (fitparams.useNoise())
                    this->setNoiseLevel(cube.getArray(), cube.getDimArray(), fitparams.noiseBoxSize());
            }

            void RadioSource::setNoiseLevel(float *array, long *dim, int boxSize)
            {
                /// @details Sets the value of the local noise level by taking
                /// the MADFM of the surrounding pixels from the Cube's array.
                /// A box of side length boxSize is centred on the peak pixel
                /// of the detection, and the MADFM of the pixels therein is
                /// found. This is converted to a Gaussian rms, and stored as
                /// the RadioSource::itsNoiseLevel value.
                /// @param array Array of pixel values
                /// @param dim Set of dimensions for array
                /// @param boxSize The side length of the box used.
                int hw = boxSize / 2;
                float *localArray = new float[boxSize*boxSize];
                long xmin = max(0, this->xpeak - hw);
                long ymin = max(0, this->ypeak - hw);
                long xmax = min(dim[0] - 1, this->xpeak + hw);
                long ymax = min(dim[1] - 1, this->ypeak + hw);
                int size = 0;

                for (int x = xmin; x <= xmax; x++) {
                    for (int y = ymin; y <= ymax; y++) {
                        int pos = x + y * dim[0];
                        localArray[size++] = array[pos];
                    }
                }

                std::sort(localArray, localArray + size);
                float median, madfm;

                if (size % 2 == 0) median = (localArray[size/2] + localArray[size/2-1]) / 2.;
                else median = localArray[size/2];

                for (int i = 0; i < size; i++) localArray[i] = fabs(localArray[i] - median);

                std::sort(localArray, localArray + size);

                if ((size % 2) == 0) madfm = (localArray[size/2] + localArray[size/2-1]) / 2.;
                else madfm = localArray[size/2];

                this->itsNoiseLevel = Statistics::madfmToSigma(madfm);
                delete [] localArray;
            }


            //**************************************************************//

            void RadioSource::getFWHMestimate(float *fluxarray, double &angle, double &maj, double &min)
            {
                /// @details This returns an estimate of an object's shape,
                /// using the principle axes and position angle calculated in
                /// the duchamp::PixelInfo code. This is done by using the
                /// array of flux values given by f, thresholding at half the
                /// object's peak flux value, and averaging the x- and
                /// y-widths that the Duchamp code gives.
                ///
                /// It may be that the thresholding returns more than one
                /// object. In this case, we only look at the one with the
                /// same peak location as the base object.
//  angle = this->pixelArray.getSpatialMap().getPositionAngle();
//  std::pair<double,double> axes = this->pixelArray.getSpatialMap().getPrincipleAxes();
//  maj = std::max(axes.first,axes.second);
//  min = std::min(axes.first,axes.second);
//  return;
                long dim[2]; dim[0] = this->boxXsize(); dim[1] = this->boxYsize();
                duchamp::Image smlIm(dim);
                smlIm.saveArray(fluxarray, this->boxSize());
                smlIm.setMinSize(1);
                float thresh = (this->itsDetectionThreshold + this->peakFlux) / 2.;
                smlIm.stats().setThreshold(thresh);
                std::vector<PixelInfo::Object2D> objlist = smlIm.lutz_detect();
                std::vector<PixelInfo::Object2D>::iterator o;

                for (o = objlist.begin(); o < objlist.end(); o++) {
                    duchamp::Detection tempobj;
                    tempobj.pixels().addChannel(0, *o);
                    tempobj.calcFluxes(fluxarray, dim); // we need to know where the peak is.

                    if ((tempobj.getXPeak() + this->boxXmin()) == this->getXPeak()  &&
                            (tempobj.getYPeak() + this->boxYmin()) == this->getYPeak()) {
                        angle = o->getPositionAngle();
                        std::pair<double, double> axes = o->getPrincipleAxes();
                        maj = std::max(axes.first, axes.second);
                        min = std::min(axes.first, axes.second);
                    }
                }
            }

            //**************************************************************//

            std::vector<SubComponent> RadioSource::getSubComponentList(casa::Vector<casa::Double> &f)
            {
                std::vector<SubComponent> cmpntlist = this->getThresholdedSubComponentList(f);
                float dx = this->getXAverage() - this->getXPeak();
                float dy = this->getYAverage() - this->getYPeak();

                if (hypot(dx, dy) > 2.) {
                    SubComponent antipus;
                    //    if(this->itsHeader.getBmajKeyword()>0){
                    //      antipus.setPA(this->itsHeader.getBpaKeyword() * M_PI / 180.);
                    //      antipus.setMajor(this->itsHeader.getBmajKeyword()/this->itsHeader.getAvPixScale());
                    //      antipus.setMinor(this->itsHeader.getBminKeyword()/this->itsHeader.getAvPixScale());
                    //    }
                    //    else{
                    //      antipus.setPA(cmpntlist[0].pa());
                    //      antipus.setMajor(cmpntlist[0].maj());
                    //      antipus.setMinor(cmpntlist[0].min());
                    //    }
                    antipus.setPA(cmpntlist[0].pa());
                    antipus.setMajor(cmpntlist[0].maj());
                    antipus.setMinor(cmpntlist[0].min());
                    antipus.setX(this->getXPeak() + dx);
                    antipus.setY(this->getYPeak() + dy);
		    int pos = int(antipus.x()-this->boxXmin()) + this->boxXsize()*int(antipus.y()-this->boxYmin());
                    antipus.setPeak(f(pos));
                    cmpntlist.push_back(antipus);
                    SubComponent centre;
                    centre.setPA(antipus.pa());
                    centre.setMajor(antipus.maj());
                    centre.setMinor(antipus.min());
                    centre.setX(this->getXPeak());
                    centre.setY(this->getYPeak());
		    pos = int(centre.x()-this->boxXmin()) + this->boxXsize()*int(centre.y()-this->boxYmin());
                    centre.setPeak(f(pos));
                    cmpntlist.push_back(centre);
                }

                return cmpntlist;
            }

            std::vector<SubComponent> RadioSource::getThresholdedSubComponentList(casa::Vector<casa::Double> &f)
            {
                /// @details This function returns a vector list of
                /// subcomponents that make up the Detection. The pixel array
                /// f is searched at a series of thresholds spaced
                /// logarithmically between the Detection's peak flux and the
                /// original detection threshold. If more than one object is
                /// detected at any of these searches, getSubComponentList()
                /// is called on each of these objects. This recursive
                /// exectution will continue until only one object is left, at
                /// which point we return a SubComponent object that holds all
                /// parameters necessary to specify a 2D Gaussian (the shape
                /// parameters are determined using getFWHMestimate()).  The
                /// ultimate returned object is a vector list of
                /// SubComponents, ordered from highest to lowest peak flux.
                std::vector<SubComponent> fullList;
                long dim[2]; dim[0] = this->boxXsize(); dim[1] = this->boxYsize();
                duchamp::Image smlIm(dim);
                float *fluxarray = new float[this->boxSize()];
                PixelInfo::Object2D spatMap = this->pixelArray.getSpatialMap();

                for (int i = 0; i < this->boxSize(); i++) {
                    if (spatMap.isInObject(i % this->boxXsize() + this->boxXmin(), i / this->boxXsize() + this->boxYmin()))
                        fluxarray[i] = f(i);
                    else fluxarray[i] = 0.;
                }

                smlIm.saveArray(fluxarray, this->boxSize());
                smlIm.setMinSize(1);
                SubComponent base;
                base.setPeak(this->peakFlux);
                base.setX(this->xpeak);
                base.setY(this->ypeak);
                double a, b, c;
                this->getFWHMestimate(fluxarray, a, b, c);
                base.setPA(a);
                base.setMajor(b);
                base.setMinor(c);
//  base.setPA(this->pixelArray.getSpatialMap().getPositionAngle());
//  std::pair<double,double> axes = this->pixelArray.getSpatialMap().getPrincipleAxes();
//  base.setMajor(std::max(axes.first,axes.second));
//  base.setMinor(std::min(axes.first,axes.second));
//                const int numThresh = 20;
                const int numThresh = this->itsFitParams.numSubThresholds();
                float baseThresh = log10(this->itsDetectionThreshold);
                float threshIncrement = (log10(this->peakFlux) - baseThresh) / float(numThresh + 1);
                float thresh;
                int threshCtr = 0;
                std::vector<PixelInfo::Object2D> objlist;
                std::vector<PixelInfo::Object2D>::iterator obj;
                bool keepGoing;

                do {
                    threshCtr++;
                    thresh = pow(10., baseThresh + threshCtr * threshIncrement);
                    smlIm.stats().setThreshold(thresh);
                    objlist = smlIm.lutz_detect();
                    keepGoing = (objlist.size() == 1);
                } while (keepGoing && (threshCtr < numThresh));

                if (!keepGoing) {
                    for (obj = objlist.begin(); obj < objlist.end(); obj++) {
                        RadioSource newsrc;
			newsrc.setFitParams(this->itsFitParams);
                        newsrc.setDetectionThreshold(thresh);
                        newsrc.pixels().addChannel(0, *obj);
                        newsrc.calcFluxes(fluxarray, dim);
                        newsrc.setBox(this->box());
                        newsrc.pixels().addOffsets(this->boxXmin(), this->boxYmin(), 0);
                        newsrc.xpeak += this->boxXmin();
                        newsrc.ypeak += this->boxYmin();
                        std::vector<SubComponent> newlist = newsrc.getThresholdedSubComponentList(f);

                        for (uInt i = 0; i < newlist.size(); i++) fullList.push_back(newlist[i]);
                    }
                } else fullList.push_back(base);

                std::sort(fullList.begin(), fullList.end());
                std::reverse(fullList.begin(), fullList.end());
                delete [] fluxarray;
                return fullList;
            }


            //**************************************************************//

            std::multimap<int, PixelInfo::Voxel> RadioSource::findDistinctPeaks(casa::Vector<casa::Double> f)
            {
                /// @details
                ///
                /// Find a list of local maxima in the detection. This divides
                /// the flux interval between the object's peak flux and the
                /// detection threshold into 10, and searches for objects at
                /// each of these sub-thresholds. Maxima other than the overall
                /// peak will appear at some thresholds but not others.
                ///
                /// The list of peak locations is returned as a STL multimap,
                /// with the Key element being the number of times a peak
                /// location was found (the overall peak will be found 10
                /// times), and the Value element being the location of the
                /// peak, stored as a PixelInfo::Voxel.
	      //                const int numThresh = 10;
                const int numThresh = this->itsFitParams.numSubThresholds();
                std::multimap<int, PixelInfo::Voxel> peakMap;
                std::multimap<int, PixelInfo::Voxel>::iterator pk;
                long dim[2]; dim[0] = this->boxXsize(); dim[1] = this->boxYsize();
                duchamp::Image smlIm(dim);
                float *fluxarray = new float[this->boxSize()];

                for (int i = 0; i < this->boxSize(); i++) fluxarray[i] = f(i);

                smlIm.saveArray(fluxarray, this->boxSize());
                smlIm.setMinSize(1);
                float baseThresh = log10(this->itsDetectionThreshold);
                float threshIncrement = (log10(this->peakFlux) - baseThresh) / float(numThresh);
                PixelInfo::Object2D spatMap = this->pixelArray.getSpatialMap();

                for (int i = 1; i <= numThresh; i++) {
                    float thresh = pow(10., baseThresh + i * threshIncrement);
                    smlIm.stats().setThreshold(thresh);
                    std::vector<PixelInfo::Object2D> objlist = smlIm.lutz_detect();
                    std::vector<PixelInfo::Object2D>::iterator o;

                    for (o = objlist.begin(); o < objlist.end(); o++) {
                        duchamp::Detection tempobj;
                        tempobj.pixels().addChannel(0, *o);
                        tempobj.calcFluxes(fluxarray, dim);
                        bool pkInObj = spatMap.isInObject(tempobj.getXPeak() + this->boxXmin(),
                                                          tempobj.getYPeak() + this->boxYmin());

                        if (pkInObj) {
                            PixelInfo::Voxel peakLoc(tempobj.getXPeak() + this->boxXmin(),
                                                     tempobj.getYPeak() + this->boxYmin(),
                                                     tempobj.getZPeak(),
                                                     tempobj.getPeakFlux());
                            int freq = 1;
                            bool finished = false;

                            if (peakMap.size() > 0) {
                                pk = peakMap.begin();

                                while (!finished && pk != peakMap.end()) {
                                    if (!(pk->second == peakLoc)) pk++;
                                    else {
                                        freq = pk->first + 1;
                                        peakMap.erase(pk);
                                        finished = true;
                                    }
                                }
                            }

                            peakMap.insert(std::pair<int, PixelInfo::Voxel>(freq, peakLoc));
                        }
                    }
                }

                delete [] fluxarray;
                return peakMap;
            }

            //**************************************************************//

            /// @brief A simple way of printing fitted parameters
            void printparameters(Matrix<Double> &m)
            {
                cout.precision(3);
                cout.setf(ios::fixed);
                uInt g, p;

                for (g = 0; g < m.nrow(); g++) {
                    for (p = 0; p < m.ncolumn() - 1; p++) cout << m(g, p) << ", ";

                    cout << m(g, p) << endl;

                    if (g < m.nrow() - 1) cout << "                    ";
                }
            }

            //**************************************************************//

            bool RadioSource::fitGauss(std::vector<PixelInfo::Voxel> *voxelList, FittingParameters &baseFitter)
            {
                /// @details First defines the pixel array with the flux
                /// values by extracting the voxels from voxelList that are
                /// within the box surrounding the object. Their flux values
                /// are placed in the flux matrix, which is passed to
                /// fitGauss(casa::Matrix<casa::Double> pos,
                /// casa::Vector<casa::Double> f, casa::Vector<casa::Double>
                /// sigma).
                if (this->getSpatialSize() < baseFitter.minFitSize()) return false;

                casa::Matrix<casa::Double> pos;
                casa::Vector<casa::Double> f;
                casa::Vector<casa::Double> sigma;
                pos.resize(this->boxSize(), 2);
                f.resize(this->boxSize());
                sigma.resize(this->boxSize());
                casa::Vector<casa::Double> curpos(2);
                curpos = 0;
                bool failure = false;

                if (this->getZcentre() != this->getZmin() || this->getZcentre() != this->getZmax()) {
                    ASKAPLOG_ERROR(logger, "Can only do fitting for two-dimensional objects!");
                    return failure;
                }

                long z = this->getZPeak();

                for (long x = this->boxXmin(); x <= this->boxXmax() && !failure; x++) {
                    for (long y = this->boxYmin(); y <= this->boxYmax() && !failure; y++) {
                        int i = (x - this->boxXmin()) + (y - this->boxYmin()) * this->boxXsize();
                        PixelInfo::Voxel tempvox(x, y, z, 0.);
                        std::vector<PixelInfo::Voxel>::iterator vox = voxelList->begin();

                        while (!tempvox.match(*vox) && vox != voxelList->end()) vox++;

                        if (vox == voxelList->end()) failure = true;
                        else f(i) = vox->getF();

                        sigma(i) = this->itsNoiseLevel;
                        curpos(0) = x;
                        curpos(1) = y;
                        pos.row(i) = curpos;
                    }
                }

                if (failure) {
		  ASKAPLOG_ERROR_STR(logger, "RadioSource: Failed to allocate flux array for object at ("
				     << this->getXcentre() <<","<<this->getYcentre() <<","<<this->getZcentre()<<"), or " 
				     << this->ra << " " << this->dec << " " << this->vel);
                    return !failure;
                }

                return fitGauss(pos, f, sigma, baseFitter);
            }

            //**************************************************************//

            bool RadioSource::fitGauss(float *fluxArray, long *dimArray, FittingParameters &baseFitter)
            {
                /// @details First defines the pixel array with the flux
                /// values by extracting the voxels from fluxArray that are
                /// within the box surrounding the object. Their flux values
                /// are placed in the flux matrix, which is passed to
                /// fitGauss(casa::Matrix<casa::Double> pos,
                /// casa::Vector<casa::Double> f, casa::Vector<casa::Double>
                /// sigma).
                if (this->getSpatialSize() < baseFitter.minFitSize()) return false;

                if (this->getZcentre() != this->getZmin() || this->getZcentre() != this->getZmax()) {
                    ASKAPLOG_ERROR(logger, "Can only do fitting for two-dimensional objects!");
                    return false;
                }

                casa::Matrix<casa::Double> pos;
                casa::Vector<casa::Double> f;
                casa::Vector<casa::Double> sigma;
                pos.resize(this->boxSize(), 2);
                f.resize(this->boxSize());
                sigma.resize(this->boxSize());
                casa::Vector<casa::Double> curpos(2);
                curpos = 0;

                for (int x = this->boxXmin(); x <= this->boxXmax(); x++) {
                    for (int y = this->boxYmin(); y <= this->boxYmax(); y++) {
                        int i = (x - this->boxXmin()) + (y - this->boxYmin()) * this->boxXsize();
                        int j = x + y * dimArray[0];

                        if ((j >= 0) && (j < dimArray[0]*dimArray[1])) f(i) = fluxArray[j];
                        else f(i) = 0.;

                        sigma(i) = this->itsNoiseLevel;
                        curpos(0) = x;
                        curpos(1) = y;
                        pos.row(i) = curpos;
                    }
                }

                return fitGauss(pos, f, sigma, baseFitter);
            }

            //**************************************************************//

            bool RadioSource::fitGauss(casa::Matrix<casa::Double> pos, casa::Vector<casa::Double> f,
                                       casa::Vector<casa::Double> sigma, FittingParameters &baseFitter)
            {
                if (this->getSpatialSize() < baseFitter.minFitSize()) return false;

                this->itsFitParams = baseFitter;
                this->itsFitParams.saveBox(this->itsBoxMargins);
                this->itsFitParams.setPeakFlux(this->peakFlux);
                this->itsFitParams.setDetectThresh(this->itsDetectionThreshold);

                ASKAPLOG_INFO_STR(logger, "Fitting source at RA=" << this->raS << ", Dec=" << this->decS);
                ASKAPLOG_DEBUG_STR(logger, "detect thresh = " << this->itsDetectionThreshold
                                       << "  peak = " << this->peakFlux
                                       << "  noise level = " << this->itsNoiseLevel);
                std::vector<SubComponent> cmpntList = this->getSubComponentList(f);
                ASKAPLOG_DEBUG_STR(logger, "Found " << cmpntList.size() << " subcomponents");

                for (uInt i = 0; i < cmpntList.size(); i++)
                    ASKAPLOG_DEBUG_STR(logger, "SubComponent: " << cmpntList[i]);

		std::map<float, std::string> bestFitMap; // map reduced-chisq to fitType

		std::vector<std::string>::iterator type;
		std::vector<std::string> typelist = availableFitTypes;
		for(type = typelist.begin(); type<typelist.end(); type++){
                    if (this->itsFitParams.hasType(*type)) {
		        ASKAPLOG_INFO_STR(logger, "Commencing fits of type \""<<*type<<"\"");
                        this->itsFitParams.setFlagFitThisParam(*type);
                        int ctr = 0;
                        Fitter fit[this->itsFitParams.maxNumGauss()];
                        bool fitIsGood = false;
                        int bestFit = 0;
                        float bestRChisq = 9999.;

                        for (int g = 1; g <= this->itsFitParams.maxNumGauss(); g++) {
                            fit[ctr].setParams(this->itsFitParams);
                            fit[ctr].setNumGauss(g);
                            fit[ctr].setEstimates(cmpntList, this->itsHeader);
                            fit[ctr].setRetries();
                            fit[ctr].setMasks();
                            fit[ctr].fit(pos, f, sigma);

                            if (fit[ctr].acceptable()) {
                                if ((ctr == 0) || (fit[ctr].redChisq() < bestRChisq)) {
                                    fitIsGood = true;
                                    bestFit = ctr;
                                    bestRChisq = fit[ctr].redChisq();
                                }
                            }

                            ctr++;
                        } // end of 'g' for-loop

                        if (fitIsGood) {
                            this->hasFit = true;

                            if (*type == "full")     this->itsBestFitFULL.saveResults(fit[bestFit]);
                            else if (*type == "psf") this->itsBestFitPSF.saveResults(fit[bestFit]);
                            else if (*type == "shape") this->itsBestFitSHAPE.saveResults(fit[bestFit]);

			    bestFitMap.insert( std::pair<float,std::string>(fit[bestFit].redChisq(), *type) );
                        }
                    }
                } // end of type for-loop

                if (this->hasFit) {

		    std::string bestFitType = bestFitMap.begin()->second;
		    if(bestFitType == "full")        this->itsBestFit = this->itsBestFitFULL;
		    else if (bestFitType == "psf")   this->itsBestFit = this->itsBestFitPSF;
		    else if (bestFitType == "shape") this->itsBestFit = this->itsBestFitSHAPE;

                    ASKAPLOG_INFO_STR(logger, "BEST FIT: " << this->itsBestFit.numGauss() << " Gaussians"
				          << " with fit type \"" << bestFitType << "\""
                                          << ", chisq = " << this->itsBestFit.chisq()
                                          << ", chisq/nu =  "  << this->itsBestFit.redchisq()
                                          << ", RMS = " << this->itsBestFit.RMS());
                } else {
                    this->hasFit = false;
                    ASKAPLOG_INFO_STR(logger, "No good fit found.");
                }

                ASKAPLOG_INFO_STR(logger, "-----------------------");
                return this->hasFit;
            }

            //**************************************************************//

            void RadioSource::printSummary(std::ostream &stream, std::vector<duchamp::Column::Col> columns,
                                           std::string fittype, bool doHeader)
            {
                /// @details
                ///
                /// This function writes out the position and flux information
                /// for the detected object and its fitted componenets. The
                /// information includes:
                /// @li RA & Dec & Vel
                /// @li Detected peak flux (from duchamp::Detection object)
                /// @li Detected integrated flux (from duchamp::Detection)
                /// @li Number of fitted componente
                /// @li Peak & Integrated flux of fitted components (using all components)
                FitResults results = this->itsBestFit;

                if (fittype == "full") results = this->itsBestFitFULL;
                else if (fittype == "psf") results = this->itsBestFitPSF;
                else if (fittype == "shape") results = this->itsBestFitSHAPE;

                stream.setf(std::ios::fixed);
                int suffixCtr = 0;
                char firstSuffix = 'a';
                /// @todo Make this a more obvious parameter to change
                const int fluxPrec = 8;
                const int fluxWidth = fluxPrec + 12;
                columns[duchamp::Column::FINT].changePrec(fluxPrec);
                columns[duchamp::Column::FPEAK].changePrec(fluxPrec);
                columns[duchamp::Column::NUM].setName("ID");
                // new columns
                duchamp::Column::Col fIntfit("F_int(fit)", "", fluxWidth, fluxPrec);
                duchamp::Column::Col fPkfit("F_pk(fit)", "", fluxWidth, fluxPrec);
                duchamp::Column::Col majFit("Maj(fit)", "", 10, 3);
                duchamp::Column::Col minFit("Min(fit)", "", 10, 3);
                duchamp::Column::Col paFit("P.A.(fit)", "", 10, 2);
                duchamp::Column::Col chisqFit("Chisq(fit)", "", 11, 2);
                duchamp::Column::Col rmsIm("RMS(image)", "", fluxWidth, fluxPrec);
                duchamp::Column::Col rmsFit("RMS(fit)", "", 10, 2);
                duchamp::Column::Col nfree("Nfree(fit)", "", 11, 0);
                duchamp::Column::Col ndofFit("NDoF(fit)", "", 10, 0);
                duchamp::Column::Col npixFit("NPix(fit)", "", 10, 0);
                duchamp::Column::Col npixObj("NPix(obj)", "", 10, 0);

                if (doHeader) {
                    stream << "#";
                    columns[duchamp::Column::NUM].printTitle(stream);
//                     columns[duchamp::Column::RA].printTitle(stream);
//                     columns[duchamp::Column::DEC].printTitle(stream);
                    columns[duchamp::Column::RAJD].printTitle(stream);
                    columns[duchamp::Column::DECJD].printTitle(stream);
//    columns[duchamp::Column::VEL].printTitle(stream);
                    columns[duchamp::Column::FINT].printTitle(stream);
                    columns[duchamp::Column::FPEAK].printTitle(stream);
                    fIntfit.printTitle(stream);
                    fPkfit.printTitle(stream);
                    majFit.printTitle(stream);
                    minFit.printTitle(stream);
                    paFit.printTitle(stream);
                    chisqFit.printTitle(stream);
                    rmsIm.printTitle(stream);
                    rmsFit.printTitle(stream);
                    nfree.printTitle(stream);
                    ndofFit.printTitle(stream);
                    npixFit.printTitle(stream);
                    npixObj.printTitle(stream);
                    stream << "\n";
                    int width = columns[duchamp::Column::NUM].getWidth() +
//                                 columns[duchamp::Column::RA].getWidth() +
//                                 columns[duchamp::Column::DEC].getWidth() +
                                columns[duchamp::Column::RAJD].getWidth() +
                                columns[duchamp::Column::DECJD].getWidth() +
//      columns[duchamp::Column::VEL].getWidth() +
                                columns[duchamp::Column::FINT].getWidth() +
                                columns[duchamp::Column::FPEAK].getWidth() +
                                fIntfit.getWidth() +
                                fPkfit.getWidth() +
                                majFit.getWidth() +
                                minFit.getWidth() +
                                paFit.getWidth() +
                                chisqFit.getWidth() +
                                rmsIm.getWidth() +
                                rmsFit.getWidth() +
                                nfree.getWidth() +
                                ndofFit.getWidth() +
                                npixFit.getWidth() +
                                npixObj.getWidth();
                    stream << "#" << std::setfill('-') << std::setw(width) << '-' << "\n";
                }

                columns[duchamp::Column::NUM].widen(); // to account for the # characters at the start of the title lines

                if (!results.isGood()) { //if no fits were made...
                    float zero = 0.;
                    columns[duchamp::Column::NUM].printEntry(stream, this->getID());
//                     columns[duchamp::Column::RA].printEntry(stream, this->getRAs());
//                     columns[duchamp::Column::DEC].printEntry(stream, this->getDecs());
                    columns[duchamp::Column::RAJD].printEntry(stream, this->getRA());
                    columns[duchamp::Column::DECJD].printEntry(stream, this->getDec());
//    columns[duchamp::Column::VEL].printEntry(stream,this->getVel());
                    columns[duchamp::Column::FINT].printEntry(stream, this->getIntegFlux());
                    columns[duchamp::Column::FPEAK].printEntry(stream, this->getPeakFlux());
                    fIntfit.printEntry(stream, zero);
                    fPkfit.printEntry(stream, zero);
                    majFit.printEntry(stream, zero);
                    minFit.printEntry(stream, zero);
                    paFit.printEntry(stream, zero);
                    chisqFit.printEntry(stream, zero);
                    rmsIm.printEntry(stream, this->itsNoiseLevel);
                    rmsFit.printEntry(stream, zero);
                    nfree.printEntry(stream, zero);
                    ndofFit.printEntry(stream, zero);
                    npixFit.printEntry(stream, zero);
                    npixObj.printEntry(stream, this->getSize());
                    stream << "\n";
                }

                std::vector<casa::Gaussian2D<Double> > fitSet = results.fitSet();
                std::vector<casa::Gaussian2D<Double> >::iterator fit;

                for (fit = fitSet.begin(); fit < fitSet.end(); fit++) {
                    std::stringstream id;
                    id << this->getID() << char(firstSuffix + suffixCtr++);
                    double *pix = new double[3];
                    pix[0] = fit->xCenter();
                    pix[1] = fit->yCenter();
                    pix[2] = this->getZcentre();
                    double *wld = new double[3];
                    this->itsHeader.pixToWCS(pix, wld);
//                     std::string thisRA = decToDMS(wld[0], "RA");
//                     std::string thisDec = decToDMS(wld[1], "DEC");
		    double thisRA = wld[0];
		    double thisDec = wld[1];
                    delete [] pix;
                    delete [] wld;
                    float intfluxfit = fit->flux();

                    if (this->itsHeader.needBeamSize())
                        intfluxfit /= this->itsHeader.getBeamSize(); // Convert from Jy/beam to Jy

                    columns[duchamp::Column::NUM].printEntry(stream, id.str());
//                     columns[duchamp::Column::RA].printEntry(stream, thisRA);
//                     columns[duchamp::Column::DEC].printEntry(stream, thisDec);
                    columns[duchamp::Column::RAJD].printEntry(stream, thisRA);
                    columns[duchamp::Column::DECJD].printEntry(stream, thisDec);
//    columns[duchamp::Column::VEL].printEntry(stream,this->getVel());
                    columns[duchamp::Column::FINT].printEntry(stream, this->getIntegFlux());
                    columns[duchamp::Column::FPEAK].printEntry(stream, this->getPeakFlux());
                    fIntfit.printEntry(stream, intfluxfit);
                    fPkfit.printEntry(stream, fit->height());
                    majFit.printEntry(stream, fit->majorAxis()*this->itsHeader.getAvPixScale()*3600.); // convert from pixels to arcsec
                    minFit.printEntry(stream, fit->minorAxis()*this->itsHeader.getAvPixScale()*3600.);
                    paFit.printEntry(stream, fit->PA()*180. / M_PI);
                    chisqFit.printEntry(stream, results.chisq());
                    rmsIm.printEntry(stream, this->itsNoiseLevel);
                    rmsFit.printEntry(stream, results.RMS());
                    nfree.printEntry(stream, results.numFreeParam());
                    ndofFit.printEntry(stream, results.ndof());
                    npixFit.printEntry(stream, this->boxSize());
                    npixObj.printEntry(stream, this->getSize());
                    stream << "\n";
                }
            }

            //**************************************************************//

            void RadioSource::writeFitToAnnotationFile(std::ostream &stream)
            {
                /// @details
                ///
                /// This function writes the information about the fitted
                /// Gaussian components to a Karma annotation file. There are
                /// two different elements drawn for each RadioSource object.
                ///
                /// For each fitted component, an ellipse is drawn indicating
                /// the size and orientation of the Gaussian. The central
                /// position is converted to world coordinates, and the major
                /// and minor axes are converted to elliptical
                /// semimajor/semiminor axes by halving and dividing by 2*ln(2).
                ///
                /// Finally, a box is drawn around the detection, indicating the
                /// area used in the fitting. It includes the border around the
                /// detection fiven by sourcefitting::detectionBorder.
                double *pix = new double[12];
                double *world = new double[12];
                for(int i=0;i<4;i++) pix[i*3+2] = 0;
                std::vector<casa::Gaussian2D<Double> > fitSet = this->itsBestFit.fitSet();
                std::vector<casa::Gaussian2D<Double> >::iterator fit;

                for (fit = fitSet.begin(); fit < fitSet.end(); fit++) {
                    pix[0] = fit->xCenter();
                    pix[1] = fit->yCenter();
                    this->itsHeader.pixToWCS(pix, world);
                    stream.setf(std::ios::fixed);
                    stream.precision(6);
                    stream << "ELLIPSE "
                        << world[0] << " "
                        << world[1] << " "
                        << fit->majorAxis() * this->itsHeader.getAvPixScale() / 2. << " "
                        << fit->minorAxis() * this->itsHeader.getAvPixScale() / 2. << " "
                        << fit->PA() * 180. / M_PI << "\n";
                }

                pix[0] = pix[9] = this->getXmin() - this->itsFitParams.boxPadSize() - 0.5;
                pix[1] = pix[4] = this->getYmin() - this->itsFitParams.boxPadSize() - 0.5;
		pix[3] = pix[6] = this->getXmax() + this->itsFitParams.boxPadSize() + 0.5;
                pix[7] = pix[10] = this->getYmax() + this->itsFitParams.boxPadSize() + 0.5;
                this->itsHeader.pixToWCS(pix, world, 4);
		stream << "CLINES ";
		for(int i=0;i<4;i++) stream << world[i*3] << " " << world[i*3+1] << " ";
		stream << world[0] << " " << world[1] << "\n";
                delete [] pix;
                delete [] world;
            }


        }

    }

}
