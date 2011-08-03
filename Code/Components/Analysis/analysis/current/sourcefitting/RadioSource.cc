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
#include <sourcefitting/SubThresholder.h>
#include <analysisutilities/AnalysisUtilities.h>
#include <analysisutilities/SubimageDef.h>
#include <analysisutilities/CasaImageUtil.h>

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
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slicer.h>

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
ASKAP_LOGGER(logger, ".radioSource");

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
                std::vector<std::string>::iterator type;
                std::vector<std::string> typelist = availableFitTypes;

                for (type = typelist.begin(); type < typelist.end(); type++) {
                    this->itsAlphaMap[*type] = std::vector<float>(1, -99.);
                    this->itsBetaMap[*type] = std::vector<float>(1, -99.);
                }

                this->itsAlphaMap["best"] = std::vector<float>(1, -99.);
                this->itsBetaMap["best"] = std::vector<float>(1, -99.);
            }

            RadioSource::RadioSource(duchamp::Detection obj):
                    duchamp::Detection(obj)
            {
                this->hasFit = false;
                this->atEdge = false;
                this->itsNoiseLevel = 1.;
                std::vector<std::string>::iterator type;
                std::vector<std::string> typelist = availableFitTypes;

                for (type = typelist.begin(); type < typelist.end(); type++) {
                    this->itsAlphaMap[*type] = std::vector<float>(1, -99.);
                    this->itsBetaMap[*type] = std::vector<float>(1, -99.);
                }

                this->itsAlphaMap["best"] = std::vector<float>(1, -99.);
                this->itsBetaMap["best"] = std::vector<float>(1, -99.);
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
                this->itsBox = src.itsBox;
                this->itsFitParams = src.itsFitParams;
                this->itsBestFitMap = src.itsBestFitMap;
                this->itsBestFitType = src.itsBestFitType;
                this->itsAlphaMap = src.itsAlphaMap;
                this->itsBetaMap = src.itsBetaMap;
                return *this;
            }

            //**************************************************************//

            void RadioSource::defineBox(duchamp::Section &sec, FittingParameters &fitParams, int spectralAxis)
            {
                /// @details Defines the maximum and minimum points of the box
                /// in each axis direction. The size of the image array is
                /// taken into account, using the axes array, so that the box
                /// does not go outside the allowed pixel area.

                casa::IPosition start(3, 0), end(3, 0), stride(3, 1);
                start(0) = std::max(long(sec.getStart(0) - this->xSubOffset), this->getXmin() - fitParams.boxPadSize());
                end(0)   = std::min(long(sec.getEnd(0) - this->xSubOffset), this->getXmax() + fitParams.boxPadSize());
                start(1) = std::max(long(sec.getStart(1) - this->ySubOffset), this->getYmin() - fitParams.boxPadSize());
                end(1)   = std::min(long(sec.getEnd(1) - this->ySubOffset), this->getYmax() + fitParams.boxPadSize());
                start(2) = std::max(long(sec.getStart(spectralAxis) - this->zSubOffset), this->getZmin() - fitParams.boxPadSize());
                end(2)   = std::min(long(sec.getEnd(spectralAxis) - this->zSubOffset), this->getZmax() + fitParams.boxPadSize());
                this->itsBox = casa::Slicer(start, end, stride, Slicer::endIsLast);
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

                long xminEdge, xmaxEdge, yminEdge, ymaxEdge, zminEdge, zmaxEdge;

                if (workerNum < 0) {  // if it is the Master node
                    xminEdge = yminEdge = zminEdge = 0;
                    xmaxEdge = cube.getDimX() - 1;
                    ymaxEdge = cube.getDimY() - 1;
                    zmaxEdge = cube.getDimZ() - 1;
                } else {
                    int *nsub = subimage.nsub();
                    int *overlap = subimage.overlap();
                    int colnum = workerNum % nsub[0];
                    int rownum = workerNum / nsub[0];
                    int znum = workerNum / (nsub[0] * nsub[1]);
                    xminEdge = (colnum == 0) ? 0 : overlap[0];
                    xmaxEdge = (colnum == nsub[0] - 1) ? cube.getDimX() - 1 : cube.getDimX() - 1 - overlap[0];
                    yminEdge = (rownum == 0) ? 0 : overlap[1];
                    ymaxEdge = (rownum == nsub[1] - 1) ? cube.getDimY() - 1 : cube.getDimY() - 1 - overlap[1];
                    zminEdge = (znum == 0) ? 0 : overlap[2];
                    zmaxEdge = (znum == nsub[2] - 1) ? cube.getDimZ() - 1 : cube.getDimZ() - 1 - overlap[2];
                }


                if (flagAdj) {
                    flagBoundary = flagBoundary || (this->getXmin() <= xminEdge);
                    flagBoundary = flagBoundary || (this->getXmax() >= xmaxEdge);
                    flagBoundary = flagBoundary || (this->getYmin() <= yminEdge);
                    flagBoundary = flagBoundary || (this->getYmax() >= ymaxEdge);

                    if (cube.getDimZ() > 1) {
                        flagBoundary = flagBoundary || (this->getZmin() <= zminEdge);
                        flagBoundary = flagBoundary || (this->getZmax() >= zmaxEdge);
                    }
                } else {
                    flagBoundary = flagBoundary || ((this->getXmin() - xminEdge) < threshS);
                    flagBoundary = flagBoundary || ((xmaxEdge - this->getXmax()) < threshS);
                    flagBoundary = flagBoundary || ((this->getYmin() - yminEdge) < threshS);
                    flagBoundary = flagBoundary || ((ymaxEdge - this->getYmax()) < threshS);

                    if (cube.getDimZ() > 1) {
                        flagBoundary = flagBoundary || ((this->getZmin() - zminEdge) < threshV);
                        flagBoundary = flagBoundary || ((zmaxEdge - this->getZmax()) < threshV);
                    }
                }
		//		if(this->getSize()==557) ASKAPLOG_DEBUG_STR(logger, "Odd source! atEdge flag = " << flagBoundary);

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
	        if(boxSize%2==0) boxSize+=1;
	        int hw = boxSize / 2;
                float *localArray = new float[boxSize*boxSize];
                long xmin = max(0, this->xpeak - hw);
                long ymin = max(0, this->ypeak - hw);
                long xmax = min(dim[0] - 1, this->xpeak + hw);
                long ymax = min(dim[1] - 1, this->ypeak + hw);
		//		ASKAPLOG_DEBUG_STR(logger, "boxSize="<<boxSize<<" xmin="<<xmin<<" ymin="<<ymin <<" xmax="<<xmax <<" ymax="<<ymax<<" xpeak="<<xpeak << " ypeak="<<ypeak << " dim[0]="<<dim[0]<<" dim[1]="<<dim[1]);
		ASKAPASSERT((xmax-xmin+1)*(ymax-ymin+1) <= boxSize*boxSize);
                size_t size = 0;

                for (int x = xmin; x <= xmax; x++) {
                    for (int y = ymin; y <= ymax; y++) {
                        int pos = x + y * dim[0];
                        localArray[size++] = array[pos];
                    }
                }

                std::nth_element(localArray, localArray + size / 2, localArray + size);
                float median = localArray[size/2];

                if (size % 2 == 0) {
                    std::nth_element(localArray, localArray + size / 2 - 1, localArray + size);
                    median += localArray[size/2-1];
                    median /= 2.;
                }

                for (size_t i = 0; i < size; i++) localArray[i] = fabs(localArray[i] - median);

                std::nth_element(localArray, localArray + size / 2, localArray + size);
                float madfm = localArray[size/2];

                if (size % 2 == 0) {
                    std::nth_element(localArray, localArray + size / 2 - 1, localArray + size);
                    median += localArray[size/2-1];
                    median /= 2.;
                }

                this->itsNoiseLevel = Statistics::madfmToSigma(madfm);
		ASKAPLOG_DEBUG_STR(logger, "Setting noise level to " << this->itsNoiseLevel);

                delete [] localArray;
            }


            //**************************************************************//

	  void RadioSource::setDetectionThreshold(duchamp::Cube &cube, bool flagMedianSearch)
	  {
	    

	    if (flagMedianSearch) {
	      std::vector<PixelInfo::Voxel> voxSet = this->getPixelSet();
	      std::vector<PixelInfo::Voxel>::iterator vox = voxSet.begin();
	      this->peakSNR = cube.getReconValue(vox->getX(), vox->getY(), vox->getZ());
	      this->itsDetectionThreshold = cube.getPixValue(vox->getX(), vox->getY(), vox->getZ());
	      
	      for (; vox < voxSet.end(); vox++) {
		this->peakSNR = std::max(this->peakSNR, cube.getReconValue(vox->getX(), vox->getY(), vox->getZ()));
		this->itsDetectionThreshold = std::min(this->itsDetectionThreshold, cube.getPixValue(vox->getX(), vox->getY(), vox->getZ()));
	      }
	      
	    } else {

	      this->itsDetectionThreshold = cube.stats().getThreshold();
	      
	      if (cube.pars().getFlagGrowth()) {
		if (cube.pars().getFlagUserGrowthThreshold())
		  this->itsDetectionThreshold = std::min(this->itsDetectionThreshold, cube.pars().getGrowthThreshold());
		else
		  this->itsDetectionThreshold = std::min(this->itsDetectionThreshold, cube.stats().snrToValue(cube.pars().getGrowthCut()));
	      }
	    }

	  }

            //**************************************************************//

	  void RadioSource::setDetectionThreshold(std::vector<PixelInfo::Voxel> &inVoxlist,std::vector<PixelInfo::Voxel> &inSNRvoxlist,  bool flagMedianSearch)
	  {

	    if (flagMedianSearch) {
	      std::vector<PixelInfo::Voxel> voxSet = this->getPixelSet();
	      std::vector<PixelInfo::Voxel>::iterator vox = voxSet.begin();
	      this->peakSNR = 0.;
	      
	      for (; vox < voxSet.end(); vox++) {
		std::vector<PixelInfo::Voxel>::iterator pixvox = inVoxlist.begin();
		
		while (pixvox < inVoxlist.end() && !vox->match(*pixvox)) pixvox++;
		
		if (pixvox == inVoxlist.end())
		  ASKAPLOG_ERROR_STR(logger, "Missing a voxel in the pixel list comparison: (" << vox->getX() << "," << vox->getY() << ")");
		
		if (vox == voxSet.begin()) this->itsDetectionThreshold = pixvox->getF();
		else this->itsDetectionThreshold = std::min(this->itsDetectionThreshold, pixvox->getF());
		
		std::vector<PixelInfo::Voxel>::iterator snrvox = inSNRvoxlist.begin();
		
		while (snrvox < inSNRvoxlist.end() && !vox->match(*snrvox)) snrvox++;
		
		if (snrvox == inSNRvoxlist.end())
		  ASKAPLOG_ERROR_STR(logger, "Missing a voxel in the SNR list comparison: (" << vox->getX() << "," << vox->getY() << ")");
		
		if (vox == voxSet.begin()) this->peakSNR = snrvox->getF();
		else this->peakSNR = std::max(this->peakSNR, snrvox->getF());
	      }
	      
	    }

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

                long dim[2]; dim[0] = this->boxXsize(); dim[1] = this->boxYsize();
                duchamp::Image *smlIm = new duchamp::Image(dim);
                smlIm->saveArray(fluxarray, this->boxSize());
                smlIm->setMinSize(1);
                float thresh = (this->itsDetectionThreshold + this->peakFlux) / 2.;
                smlIm->stats().setThreshold(thresh);
                std::vector<PixelInfo::Object2D> objlist = smlIm->findSources2D();
                std::vector<PixelInfo::Object2D>::iterator o;

                for (o = objlist.begin(); o < objlist.end(); o++) {
                    duchamp::Detection tempobj;
                    tempobj.addChannel(0, *o);
                    tempobj.calcFluxes(fluxarray, dim); // we need to know where the peak is.

                    if ((tempobj.getXPeak() + this->boxXmin()) == this->getXPeak()  &&
                            (tempobj.getYPeak() + this->boxYmin()) == this->getYPeak()) {
		      // measure parameters only for source at peak
                        angle = o->getPositionAngle();
                        std::pair<double, double> axes = o->getPrincipleAxes();
                        maj = std::max(axes.first, axes.second);
                        min = std::min(axes.first, axes.second);
                    }

                }

                delete smlIm;
            }

            //**************************************************************//

            std::vector<SubComponent> RadioSource::getSubComponentList(casa::Matrix<casa::Double> pos, casa::Vector<casa::Double> &f)
            {

		SubThresholder subThresh;
		subThresh.define(this, pos,f);
		std::vector<SubComponent> cmpntlist = subThresh.find();
 
		// get distance between average centre and peak location
		float dx = this->getXaverage() - this->getXPeak();
                float dy = this->getYaverage() - this->getYPeak();

                if (hypot(dx, dy) > 2.) {
// 		  ASKAPLOG_DEBUG_STR(logger, "Using antipus with " << cmpntlist.size() << " subcomponents");
// 		  ASKAPLOG_DEBUG_STR(logger, "Component 0 = " << cmpntlist[0]);
		  // if this distance is suitably small, add two more
		  // subcomponents: the "antipus", being the
		  // rotational reflection of the peak about the
		  // average centre; and the peak itself.
                    SubComponent antipus;
                    antipus.setPA(cmpntlist[0].pa());
                    antipus.setMajor(cmpntlist[0].maj());
                    antipus.setMinor(cmpntlist[0].min());
                    antipus.setX(this->getXPeak() + dx);
                    antipus.setY(this->getYPeak() + dy);
		    antipus.setPeak(this->getPeakFlux());
		    cmpntlist.push_back(antipus);
		    
                    SubComponent centre;
                    centre.setPA(antipus.pa());
                    centre.setMajor(antipus.maj());
                    centre.setMinor(antipus.min());
                    centre.setX(this->getXPeak());
                    centre.setY(this->getYPeak());
		    centre.setPeak(this->getPeakFlux());
		    cmpntlist.push_back(centre);
		}

                return cmpntlist;
            }

            std::vector<SubComponent> RadioSource::getThresholdedSubComponentList(float *fluxarray)
            {
                /// @details This function returns a vector list of
                /// subcomponents that make up the Detection. The pixel array
                /// f is searched at a series of thresholds spaced
                /// logarithmically between the Detection's peak flux and the
                /// original detection threshold. If more than one object is
                /// detected at any of these searches, getSubComponentList()
                /// is called on each of these objects.
                ///
                /// This recursive exectution will continue until only
                /// one object is left, at which point we return a
                /// SubComponent object that holds all parameters
                /// necessary to specify a 2D Gaussian (the shape
                /// parameters are determined using
                /// getFWHMestimate()).
                ///
                /// @return The ultimate returned object is a vector
                /// list of SubComponents, ordered from highest to
                /// lowest peak flux.

                std::vector<SubComponent> fullList;
                long dim[2]; dim[0] = this->boxXsize(); dim[1] = this->boxYsize();
                duchamp::Image *smlIm = new duchamp::Image(dim);
                smlIm->saveArray(fluxarray, this->boxSize());
                smlIm->setMinSize(1);
                SubComponent base;
                base.setPeak(this->peakFlux);
                base.setX(this->xpeak);
                base.setY(this->ypeak);
                double a, b, c;

                if (this->getSize() < 3) {
                    base.setPA(0);
                    base.setMajor(1.);
                    base.setMinor(1.);
                    fullList.push_back(base);
                    return fullList;
                }

                this->getFWHMestimate(fluxarray, a, b, c);
                base.setPA(a);
                base.setMajor(b);
                base.setMinor(c);
                const int numThresh = this->itsFitParams.numSubThresholds();
                float baseThresh = this->itsDetectionThreshold > 0 ? log10(this->itsDetectionThreshold) : -6.;
                float threshIncrement = (log10(this->peakFlux) - baseThresh) / float(numThresh + 1);
                float thresh;
                int threshCtr = 0;
                std::vector<PixelInfo::Object2D> objlist;
                std::vector<PixelInfo::Object2D>::iterator obj;
                bool keepGoing;

                do {
                    threshCtr++;
                    thresh = pow(10., baseThresh + threshCtr * threshIncrement);
                    smlIm->stats().setThreshold(thresh);
                    objlist = smlIm->findSources2D();
                    keepGoing = (objlist.size() == 1);
                } while (keepGoing && (threshCtr < numThresh));

                delete smlIm;

                if (!keepGoing) {
                    for (obj = objlist.begin(); obj < objlist.end(); obj++) {
                        RadioSource newsrc;
                        newsrc.setFitParams(this->itsFitParams);
                        newsrc.setDetectionThreshold(thresh);
                        newsrc.addChannel(0, *obj);
                        newsrc.calcFluxes(fluxarray, dim);
                        newsrc.setBox(this->box());
                        newsrc.addOffsets(this->boxXmin(), this->boxYmin(), 0);
                        newsrc.xpeak += this->boxXmin();
                        newsrc.ypeak += this->boxYmin();
                        // now change the flux array so that we only see the current object
                        float *newfluxarray = new float[this->boxSize()];

                        for (int i = 0; i < this->boxSize(); i++) {
                            int xbox = i % this->boxXsize();
                            int ybox = i / this->boxXsize();
                            PixelInfo::Object2D spatMap = newsrc.getSpatialMap();

                            if (spatMap.isInObject(xbox + this->boxXmin(), ybox + this->boxYmin())) newfluxarray[i] = fluxarray[i];
                            else newfluxarray[i] = 0.;
                        }

                        std::vector<SubComponent> newlist = newsrc.getThresholdedSubComponentList(newfluxarray);
                        delete [] newfluxarray;

                        for (uInt i = 0; i < newlist.size(); i++) fullList.push_back(newlist[i]);
                    }
                } else fullList.push_back(base);

                if (fullList.size() > 1) {
                    std::sort(fullList.begin(), fullList.end());
                    std::reverse(fullList.begin(), fullList.end());
                }

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
                PixelInfo::Object2D spatMap = this->getSpatialMap();

                for (int i = 1; i <= numThresh; i++) {
                    float thresh = pow(10., baseThresh + i * threshIncrement);
                    smlIm.stats().setThreshold(thresh);
                    std::vector<PixelInfo::Object2D> objlist = smlIm.findSources2D();
                    std::vector<PixelInfo::Object2D>::iterator o;

                    for (o = objlist.begin(); o < objlist.end(); o++) {
                        duchamp::Detection tempobj;
                        tempobj.addChannel(0, *o);
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

            bool RadioSource::fitGaussNew(std::vector<PixelInfo::Voxel> *voxelList, FittingParameters &baseFitter)
            {
                /// @details First defines the pixel array with the
                /// flux values of just the detected pixels by
                /// extracting the voxels from the given
                /// voxelList. Their flux values are placed in the
                /// flux matrix, which is passed to
                /// fitGauss(casa::Matrix<casa::Double> pos,
                /// casa::Vector<casa::Double> f,
                /// casa::Vector<casa::Double> sigma).
	      //                if (this->getSpatialSize() < baseFitter.minFitSize()) return false;

                int size = this->getSize();
                casa::Matrix<casa::Double> pos;
                casa::Vector<casa::Double> f;
                casa::Vector<casa::Double> sigma;
                pos.resize(size, 2);
                f.resize(size);
                sigma.resize(size);
                casa::Vector<casa::Double> curpos(2);
                curpos = 0;

                if (this->getZcentre() != this->getZmin() || this->getZcentre() != this->getZmax()) {
                    ASKAPLOG_ERROR_STR(logger, "Can only do fitting for two-dimensional objects!");
                    return false;
                }

                int i = 0;
                std::vector<PixelInfo::Voxel>::iterator vox = voxelList->begin();

                for (; vox < voxelList->end(); vox++) {
                    if (this->isInObject(*vox)) { // just to make sure it is a source pixel
                        sigma(i) = this->itsNoiseLevel;
                        curpos(0) = vox->getX();
                        curpos(1) = vox->getY();
                        pos.row(i) = curpos;
                        f(i) = vox->getF();
                        i++;
                    }
                }

                return fitGauss(pos, f, sigma, baseFitter);
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
                // if (this->getSpatialSize() < baseFitter.minFitSize()) return false;

                casa::Matrix<casa::Double> pos;
                casa::Vector<casa::Double> f;
                casa::Vector<casa::Double> sigma;
                pos.resize(this->boxSize(), 2);
                f.resize(this->boxSize());
                sigma.resize(this->boxSize());
                casa::Vector<casa::Double> curpos(2);
                curpos = 0;

                if (this->getZcentre() != this->getZmin() || this->getZcentre() != this->getZmax()) {
                    ASKAPLOG_ERROR(logger, "Can only do fitting for two-dimensional objects!");
                    return false;
                }

                long z = this->getZPeak();

                bool failed = false;

                for (long x = this->boxXmin(); x <= this->boxXmax() && !failed; x++) {
                    for (long y = this->boxYmin(); y <= this->boxYmax() && !failed; y++) {
                        int i = (x - this->boxXmin()) + (y - this->boxYmin()) * this->boxXsize();
                        PixelInfo::Voxel tempvox(x, y, z, 0.);
                        std::vector<PixelInfo::Voxel>::iterator vox = voxelList->begin();

                        while (!tempvox.match(*vox) && vox != voxelList->end()) vox++;

                        if (vox == voxelList->end()) failed = true;
                        else f(i) = vox->getF();

                        if (failed) ASKAPLOG_ERROR_STR(logger, "RadioSource flux allocation failed on voxel (" << x << "," << y << "," << z << ")");

                        sigma(i) = this->itsNoiseLevel;
                        curpos(0) = x;
                        curpos(1) = y;
                        pos.row(i) = curpos;
                    }
                }

                if (failed) {
                    ASKAPLOG_ERROR_STR(logger, "RadioSource: Failed to allocate flux array for object at ("
                                           << this->getXcentre() << "," << this->getYcentre() << "," << this->getZcentre() << "), or "
                                           << this->ra << " " << this->dec << " " << this->vel);
                    return false;
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
                // if (this->getSpatialSize() < baseFitter.minFitSize()) return false;

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
                /// @details This function drives the fitting of the
                /// Gaussian functions. It first sets up the fitting
                /// parameters, then finds the sub-components present in
                /// the box. The main loop is over the requested fit
                /// types. For each valid type, the Fitter is
                /// initialised and run for a number of gaussians from 1
                /// up to the maximum number requested.
                ///
                /// Each fit is compared to the best thus far, whose
                /// properties are tracked for later use. The best fit
                /// for each type is saved for later writing to the
                /// appropriate summary file, and the best overall is
                /// saved in itsBestFit
                ///
                /// @return The return value is the value of hasFit,
                /// which indicates whether a valid fit was made.

	      ASKAPLOG_INFO_STR(logger, "Fitting source at RA=" << this->raS << ", Dec=" << this->decS 
				<< ", or global position (x,y)=("<<this->getXcentre()+this->getXOffset()
				<< "," << this->getYcentre()+this->getYOffset() << ")");
	      if (this->getSpatialSize() < baseFitter.minFitSize()){
		ASKAPLOG_INFO_STR(logger, "Not fitting- source is too small - spatial size = " << this->getSpatialSize() << " cf. minFitSize = " << baseFitter.minFitSize());
		return false;
	      }
                this->itsFitParams = baseFitter;
                this->itsFitParams.saveBox(this->itsBox);
                this->itsFitParams.setPeakFlux(this->peakFlux);
                this->itsFitParams.setDetectThresh(this->itsDetectionThreshold);

                ASKAPLOG_INFO_STR(logger, "detect threshold = " << this->itsDetectionThreshold
                                       << ",  peak flux = " << this->peakFlux
                                       << ",  noise level = " << this->itsNoiseLevel);
                //
                // Get the list of subcomponents
                std::vector<SubComponent> cmpntList = this->getSubComponentList(pos, f);
                ASKAPLOG_DEBUG_STR(logger, "Found " << cmpntList.size() << " subcomponents");

                for (uInt i = 0; i < cmpntList.size(); i++)
                    ASKAPLOG_DEBUG_STR(logger, "SubComponent: " << cmpntList[i]);

                std::map<float, std::string> bestChisqMap; // map reduced-chisq to fitType

                std::vector<std::string>::iterator type;
                std::vector<std::string> typelist = availableFitTypes;

                for (type = typelist.begin(); type < typelist.end(); type++) {
                    if (this->itsFitParams.hasType(*type)) {
                        ASKAPLOG_INFO_STR(logger, "Commencing fits of type \"" << *type << "\"");
                        this->itsFitParams.setFlagFitThisParam(*type);

                        if (*type == "psf") {
                            for (size_t i = 0; i < cmpntList.size(); i++) {
//                                 cmpntList[i].setMajor(this->itsHeader.getBeamSize());
//                                 cmpntList[i].setMinor(this->itsHeader.getBeamSize());
                                cmpntList[i].setMajor(this->itsHeader.beam().area());
                                cmpntList[i].setMinor(this->itsHeader.beam().area());			      
                                cmpntList[i].setPA(0.);
                            }
                        }

                        int ctr = 0;
                        Fitter fit[this->itsFitParams.maxNumGauss()];
                        bool fitIsGood = false;
                        int bestFit = 0;
                        float bestRChisq = 9999.;

                        int maxGauss = std::min(this->itsFitParams.maxNumGauss(), int(f.size()));

			bool fitPossible=true;
                        for (int g = 1; g <= maxGauss && fitPossible; g++) {
			    ASKAPLOG_DEBUG_STR(logger, "Number of Gaussian components = " << g);

                            fit[ctr].setParams(this->itsFitParams);
                            fit[ctr].setNumGauss(g);
                            fit[ctr].setEstimates(cmpntList, this->itsHeader);
                            fit[ctr].setRetries();
                            fit[ctr].setMasks();
			    fitPossible = fit[ctr].fit(pos, f, sigma);

                            if (fitPossible && fit[ctr].acceptable()) {
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

                            this->itsBestFitMap[*type].saveResults(fit[bestFit]);

                            bestChisqMap.insert(std::pair<float, std::string>(fit[bestFit].redChisq(), *type));
                        }
                    }
                } // end of type for-loop

                if (this->hasFit) {

                    this->itsBestFitType = bestChisqMap.begin()->second;
                    this->itsBestFitMap["best"] = this->itsBestFitMap[this->itsBestFitType];

                    ASKAPLOG_INFO_STR(logger, "BEST FIT: " << this->itsBestFitMap["best"].numGauss() << " Gaussians"
                                          << " with fit type \"" << bestChisqMap.begin()->second << "\""
                                          << ", chisq = " << this->itsBestFitMap["best"].chisq()
                                          << ", chisq/nu =  "  << this->itsBestFitMap["best"].redchisq()
                                          << ", RMS = " << this->itsBestFitMap["best"].RMS());
		    this->itsBestFitMap["best"].logIt("INFO");
                } else {
                    this->hasFit = false;
                    ASKAPLOG_INFO_STR(logger, "No good fit found.");
                }

                ASKAPLOG_INFO_STR(logger, "-----------------------");
                return this->hasFit;
            }

            //**************************************************************//

            void RadioSource::findAlpha(std::string imageName, bool doCalc)
            {
                /// @details This function finds the value of the spectral
                /// index for each Gaussian component fitted to the zeroth
                /// Taylor term image. The procedure is:
                /// @li Find the Taylor 1 image from the provided image
                /// name (must be of format *.taylor.0*)
                /// @li Extract pixel values within the source's box
                /// @li For each Gaussian component of the source, and for
                /// each fit type, fit the same shape & location Gaussian
                /// (that is, only fit the height of the Gaussian).
                /// @li Calculate the spectral index using the ratio of
                /// the fluxes of the Taylor1 & Taylor0 Gaussian
                /// components
                /// @li Store the spectral index value in a map indexed by
                /// fit type.
                /// Note that if the imageName provided is not of the correct format, nothing is done.
                /// @param imageName The name of the image in which sources have been found
                /// @param doCalc If true, do all the calculations. If false, fill the alpha values to -99. for all fit types.

                size_t pos = imageName.rfind(".taylor.0");

                if (!doCalc || pos == std::string::npos) {
                    // image provided is not a Taylor series term - notify and do nothing
                    if (doCalc)
                        ASKAPLOG_WARN_STR(logger, "radioSource::findAlpha : Image name provided ("
                                              << imageName << ") is not a Taylor term. Cannot find spectral index.");

                    std::vector<std::string>::iterator type;
                    std::vector<std::string> typelist = availableFitTypes;

                    for (type = typelist.begin(); type < typelist.end(); type++) {
                        this->itsAlphaMap[*type] = std::vector<float>(this->itsBestFitMap[*type].numFits(), -99.);
                    }

                    this->itsAlphaMap["best"] = this->itsAlphaMap[this->itsBestFitType];

                } else {

                    ASKAPLOG_DEBUG_STR(logger, "About to find the spectral index, for image " << imageName);

                    // Get Taylor 1 name
                    std::string taylor1Name = imageName.replace(pos, 9, ".taylor.1");

                    ASKAPLOG_DEBUG_STR(logger, "Using Taylor 1 image " << taylor1Name);

                    // Get taylor1 values for box, and define positions
                    casa::Matrix<casa::Double> pos;
                    casa::Vector<casa::Double> sigma;
                    pos.resize(this->boxSize(), 2);
                    sigma.resize(this->boxSize());
                    casa::Vector<casa::Double> curpos(2);
                    curpos = 0;

                    for (int x = this->boxXmin(); x <= this->boxXmax(); x++) {
                        for (int y = this->boxYmin(); y <= this->boxYmax(); y++) {
                            int i = (x - this->boxXmin()) + (y - this->boxYmin()) * this->boxXsize();
                            sigma(i) = 1.;
                            curpos(0) = x;
                            curpos(1) = y;
                            pos.row(i) = curpos;
                        }
                    }

                    casa::Vector<casa::Double> f = getPixelsInBox(taylor1Name, this->itsBox);

                    ASKAPLOG_DEBUG_STR(logger, "Preparing the fit for the taylor 1 term");

                    // Set up fit with same parameters and do the fit
                    std::vector<std::string>::iterator type;
                    std::vector<std::string> typelist = availableFitTypes;

                    for (type = typelist.begin(); type < typelist.end(); type++) {
                        std::vector<float> alphaValues(this->itsBestFitMap[*type].numGauss(), -99.);
                        ASKAPLOG_DEBUG_STR(logger, "Finding alpha values for fit type \"" << *type << "\", with " << this->itsBestFitMap[*type].numGauss() << " components ");

                        if (!this->itsBestFitMap[*type].isGood())
                            ASKAPLOG_DEBUG_STR(logger, "Actually, not doing this one...");
                        else {
                            Fitter fit;
                            fit.setParams(this->itsFitParams);
                            fit.rparams().setFlagFitThisParam("height");
                            fit.rparams().setNegativeFluxPossible(true);
                            fit.setNumGauss(this->itsBestFitMap[*type].numGauss());
                            fit.setEstimates(this->itsBestFitMap[*type].getCmpntList(), this->itsHeader);
                            fit.setRetries();
                            fit.setMasks();
                            bool fitPossible = fit.fit(pos, f, sigma);

                            // Calculate alpha.

                            if (fitPossible && fit.passConverged() && fit.passChisq()) { // the fit is OK
                                ASKAPLOG_DEBUG_STR(logger, "Alpha fitting worked! Values (" << this->itsBestFitMap[*type].numGauss() << " of them) follow:");

                                for (int i = 0; i < this->itsBestFitMap[*type].numGauss(); i++) {
                                    alphaValues[i] = fit.gaussian(i).flux() / this->itsBestFitMap[*type].gaussian(i).flux();
                                    ASKAPLOG_DEBUG_STR(logger, "   Component " << i << ": " << alphaValues[i]);
                                }
                            }

                        }

                        this->itsAlphaMap[*type] = alphaValues;
                    }

                    ASKAPLOG_DEBUG_STR(logger, "Finished finding the alpha values");

                }

                this->itsAlphaMap["best"] = this->itsAlphaMap[this->itsBestFitType];

            }


            //**************************************************************//

            void RadioSource::findBeta(std::string imageName, bool doCalc)
            {
                /// @details This function finds the value of the spectral
                /// curvature for each Gaussian component fitted to the zeroth
                /// Taylor term image. The procedure is:
                /// @li Find the Taylor 2 image from the provided image
                /// name (must be of format *.taylor.0*)
                /// @li Extract pixel values within the source's box
                /// @li For each Gaussian component of the source, and for
                /// each fit type, fit the same shape & location Gaussian
                /// (that is, only fit the height of the Gaussian).
                /// @li Calculate the spectral curvature using the ratio of
                /// the fluxes of the Taylor2 & Taylor0 Gaussian
                /// components, and subtracting off \f$\alpha(\alpha-1)/2\f$.
                /// @li Store the spectral curvature value in a map indexed by
                /// fit type.
                /// Note that if the imageName provided is not of the correct format, nothing is done.
                /// @param imageName The name of the image in which sources have been found
                /// @param doCalc If true, do all the calculations. If false, fill the alpha values to -99. for all fit types.

                size_t pos = imageName.rfind(".taylor.0");

                if (!doCalc || pos == std::string::npos) {
                    // image provided is not a Taylor series term - notify and do nothing
                    if (doCalc)
                        ASKAPLOG_WARN_STR(logger, "radioSource::findBeta : Image name provided ("
                                              << imageName << ") is not a Taylor term. Cannot find spectral index.");

                    std::vector<std::string>::iterator type;
                    std::vector<std::string> typelist = availableFitTypes;

                    for (type = typelist.begin(); type < typelist.end(); type++) {
                        this->itsBetaMap[*type] = std::vector<float>(this->itsBestFitMap[*type].numFits(), -99.);
                    }

                    this->itsBetaMap["best"] = this->itsBetaMap[this->itsBestFitType];

                } else {

                    ASKAPLOG_DEBUG_STR(logger, "About to find the spectral curvature, for image " << imageName);

                    // Get Taylor 2 name
                    std::string taylor2Name = imageName.replace(pos, 9, ".taylor.2");

                    ASKAPLOG_DEBUG_STR(logger, "Using Taylor 2 image " << taylor2Name);

                    // Get taylor1 values for box, and define positions
                    casa::Matrix<casa::Double> pos;
                    casa::Vector<casa::Double> sigma;
                    pos.resize(this->boxSize(), 2);
                    sigma.resize(this->boxSize());
                    casa::Vector<casa::Double> curpos(2);
                    curpos = 0;

                    for (int x = this->boxXmin(); x <= this->boxXmax(); x++) {
                        for (int y = this->boxYmin(); y <= this->boxYmax(); y++) {
                            int i = (x - this->boxXmin()) + (y - this->boxYmin()) * this->boxXsize();
                            sigma(i) = 1.;
                            curpos(0) = x;
                            curpos(1) = y;
                            pos.row(i) = curpos;
                        }
                    }

                    casa::Vector<casa::Double> f = getPixelsInBox(taylor2Name, this->itsBox);

                    ASKAPLOG_DEBUG_STR(logger, "Preparing the fit for the taylor 2 term");

                    // Set up fit with same parameters and do the fit
                    std::vector<std::string>::iterator type;
                    std::vector<std::string> typelist = availableFitTypes;

                    for (type = typelist.begin(); type < typelist.end(); type++) {
                        std::vector<float> betaValues(this->itsBestFitMap[*type].numGauss(), -99.);
                        ASKAPLOG_DEBUG_STR(logger, "Finding beta values for fit type \"" << *type << "\", with " << this->itsBestFitMap[*type].numGauss() << " components ");

                        if (!this->itsBestFitMap[*type].isGood())
                            ASKAPLOG_DEBUG_STR(logger, "Actually, not doing this one...");
                        else {
                            Fitter fit;
                            fit.setParams(this->itsFitParams);
                            fit.rparams().setFlagFitThisParam("height");
                            fit.rparams().setNegativeFluxPossible(true);
                            fit.setNumGauss(this->itsBestFitMap[*type].numGauss());
                            fit.setEstimates(this->itsBestFitMap[*type].getCmpntList(), this->itsHeader);
                            fit.setRetries();
                            fit.setMasks();
                            bool fitPossible = fit.fit(pos, f, sigma);

                            // Calculate beta.

                            if (fitPossible && fit.passConverged() && fit.passChisq()) { // the fit is OK
                                ASKAPLOG_DEBUG_STR(logger, "Beta fitting worked! Values (" << this->itsBestFitMap[*type].numGauss() << " of them) follow:");

                                for (int i = 0; i < this->itsBestFitMap[*type].numGauss(); i++) {
                                    float alpha = this->itsAlphaMap[*type][i];
                                    betaValues[i] = fit.gaussian(i).flux() / this->itsBestFitMap[*type].gaussian(i).flux() - 0.5 * alpha * (alpha - 1.);
                                    ASKAPLOG_DEBUG_STR(logger, "   Component " << i << ": " << betaValues[i]);
                                }
                            }

                        }

                        this->itsBetaMap[*type] = betaValues;
                    }

                    ASKAPLOG_DEBUG_STR(logger, "Finished finding the beta values");

                }

                this->itsBetaMap["best"] = this->itsBetaMap[this->itsBestFitType];

            }


            //**************************************************************//

            void RadioSource::printSummary(std::ostream &stream, std::vector<duchamp::Column::Col> columns,
                                           std::string fittype, bool doHeader, bool doSpectralIndex)
            {
                /// @details
                ///
                /// This function writes out the position and flux information
                /// for the detected object and its fitted components. The
                /// information includes:
                /// @li RA & Dec
                /// @li Detected peak flux (from duchamp::Detection object)
                /// @li Detected integrated flux (from duchamp::Detection)
                /// @li Number of fitted components
                /// @li Peak & Integrated flux, and shape, of fitted components (using all components)
                /// @li RMS of the image in the local vicinity of the object
                /// @li Chi-squared and RMS values from the fit
                /// @li Number of free parameters and degrees of freedom in the fit
                /// @li Number of pixels used in the fit, and the number in the object itself

                FitResults results = this->itsBestFitMap[fittype];

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
                duchamp::Column::Col majDeconv("Maj(fit, deconv.)", "", 19, 3);
                duchamp::Column::Col minDeconv("Min(fit, deconv.)", "", 19, 3);
                duchamp::Column::Col paDeconv("P.A.(fit, deconv.)", "", 19, 2);		
		duchamp::Column::Col alpha("Alpha", "", 10, 2);
		duchamp::Column::Col beta("Beta", "", 10, 2);
                duchamp::Column::Col chisqFit("Chisq(fit)", "", 20, 9);
                duchamp::Column::Col rmsIm("RMS(image)", "", fluxWidth, fluxPrec);
                duchamp::Column::Col rmsFit("RMS(fit)", "", 11, 6);
                duchamp::Column::Col nfree("Nfree(fit)", "", 11, 0);
                duchamp::Column::Col ndofFit("NDoF(fit)", "", 10, 0);
                duchamp::Column::Col npixFit("NPix(fit)", "", 10, 0);
                duchamp::Column::Col npixObj("NPix(obj)", "", 10, 0);

                if (doHeader) {
                    stream << "#";
                    columns[duchamp::Column::NUM].printTitle(stream);
                    columns[duchamp::Column::RAJD].printTitle(stream);
                    columns[duchamp::Column::DECJD].printTitle(stream);
                    columns[duchamp::Column::FINT].printTitle(stream);
                    columns[duchamp::Column::FPEAK].printTitle(stream);
                    fIntfit.printTitle(stream);
                    fPkfit.printTitle(stream);
                    majFit.printTitle(stream);
                    minFit.printTitle(stream);
                    paFit.printTitle(stream);
                    majDeconv.printTitle(stream);
                    minDeconv.printTitle(stream);
                    paDeconv.printTitle(stream);
		    if(doSpectralIndex){
		      alpha.printTitle(stream);
		      beta.printTitle(stream);
		    }
                    chisqFit.printTitle(stream);
                    rmsIm.printTitle(stream);
                    rmsFit.printTitle(stream);
                    nfree.printTitle(stream);
                    ndofFit.printTitle(stream);
                    npixFit.printTitle(stream);
                    npixObj.printTitle(stream);
                    stream << "\n";
                    int width = columns[duchamp::Column::NUM].getWidth() +
                                columns[duchamp::Column::RAJD].getWidth() +
                                columns[duchamp::Column::DECJD].getWidth() +
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
		    if(doSpectralIndex) width +=  alpha.getWidth() + beta.getWidth();

                    stream << "#" << std::setfill('-') << std::setw(width) << '-' << "\n";
                }

                columns[duchamp::Column::NUM].widen(); // to account for the # characters at the start of the title lines

                if (!results.isGood()) { //if no fits were made...
                    float zero = 0.;
                    columns[duchamp::Column::NUM].printEntry(stream, this->getID());
                    columns[duchamp::Column::RAJD].printEntry(stream, this->getRA());
                    columns[duchamp::Column::DECJD].printEntry(stream, this->getDec());
                    columns[duchamp::Column::FINT].printEntry(stream, this->getIntegFlux());
                    columns[duchamp::Column::FPEAK].printEntry(stream, this->getPeakFlux());
                    fIntfit.printEntry(stream, zero);
                    fPkfit.printEntry(stream, zero);
                    majFit.printEntry(stream, zero);
                    minFit.printEntry(stream, zero);
                    paFit.printEntry(stream, zero);
                    majDeconv.printEntry(stream, zero);
                    minDeconv.printEntry(stream, zero);
                    paDeconv.printEntry(stream, zero);
		    if(doSpectralIndex){
		      alpha.printEntry(stream, zero);
		      beta.printEntry(stream, zero);
		    }
                    chisqFit.printEntry(stream, zero);
                    rmsIm.printEntry(stream, this->itsNoiseLevel);
                    rmsFit.printEntry(stream, zero);
                    nfree.printEntry(stream, zero);
                    ndofFit.printEntry(stream, zero);
                    npixFit.printEntry(stream, zero);
                    npixObj.printEntry(stream, this->getSize());
                    stream << "\n";
                } else {
                    std::vector<casa::Gaussian2D<Double> > fitSet = results.fitSet();
                    std::vector<casa::Gaussian2D<Double> >::iterator fit = fitSet.begin();
                    std::vector<float>::iterator alphaIter = this->itsAlphaMap[fittype].begin();
                    std::vector<float>::iterator betaIter = this->itsBetaMap[fittype].begin();
                    ASKAPCHECK(fitSet.size() == this->itsAlphaMap[fittype].size(),
                               "Sizes of fitSet (" << fitSet.size() << ") and alpha Set (" << this->itsAlphaMap[fittype].size() << ") don't match!");
                    ASKAPCHECK(fitSet.size() == this->itsBetaMap[fittype].size(),
                               "Sizes of fitSet (" << fitSet.size() << ") and beta Set (" << this->itsBetaMap[fittype].size() << ") don't match!");

                    for (; fit < fitSet.end(); fit++, alphaIter++, betaIter++) {

		      std::vector<Double> deconv = deconvolveGaussian(*fit,this->itsHeader.getBeam());

                        std::stringstream id;
                        id << this->getID() << char(firstSuffix + suffixCtr++);
                        double *pix = new double[3];
                        pix[0] = fit->xCenter();
                        pix[1] = fit->yCenter();
                        pix[2] = this->getZcentre();
                        double *wld = new double[3];
                        this->itsHeader.pixToWCS(pix, wld);
                        double thisRA = wld[0];
                        double thisDec = wld[1];
                        delete [] pix;
                        delete [] wld;
                        float intfluxfit = fit->flux();

                        if (this->itsHeader.needBeamSize())
                            intfluxfit /= this->itsHeader.beam().area(); // Convert from Jy/beam to Jy
//                             intfluxfit /= this->itsHeader.getBeamSize(); // Convert from Jy/beam to Jy

                        columns[duchamp::Column::NUM].printEntry(stream, id.str());
                        columns[duchamp::Column::RAJD].printEntry(stream, thisRA);
                        columns[duchamp::Column::DECJD].printEntry(stream, thisDec);
                        columns[duchamp::Column::FINT].printEntry(stream, this->getIntegFlux());
                        columns[duchamp::Column::FPEAK].printEntry(stream, this->getPeakFlux());
                        fIntfit.printEntry(stream, intfluxfit);
                        fPkfit.printEntry(stream, fit->height());
                        majFit.printEntry(stream, fit->majorAxis()*this->itsHeader.getAvPixScale()*3600.); // convert from pixels to arcsec
                        minFit.printEntry(stream, fit->minorAxis()*this->itsHeader.getAvPixScale()*3600.);
                        paFit.printEntry(stream, fit->PA()*180. / M_PI);
                        majDeconv.printEntry(stream, deconv[0]*this->itsHeader.getAvPixScale()*3600.); // convert from pixels to arcsec
                        minDeconv.printEntry(stream, deconv[1]*this->itsHeader.getAvPixScale()*3600.);
                        paDeconv.printEntry(stream, deconv[2]*180. / M_PI);
			if(doSpectralIndex){
			  alpha.printEntry(stream, *alphaIter);
			  beta.printEntry(stream, *betaIter);
			}
                        chisqFit.printEntry(stream, results.chisq());
                        rmsIm.printEntry(stream, this->itsNoiseLevel);
                        rmsFit.printEntry(stream, results.RMS());
                        nfree.printEntry(stream, results.numFreeParam());
                        ndofFit.printEntry(stream, results.ndof());
                        npixFit.printEntry(stream, results.numPix());
                        npixObj.printEntry(stream, this->getSize());
                        stream << "\n";
                    }
                }
            }

            //**************************************************************//

            void RadioSource::writeFitToAnnotationFile(std::ostream &stream, bool doEllipse, bool doBox)
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
                /// detection given by FittingParameters::boxPadSize()

                double *pix = new double[12];
                double *world = new double[12];

                for (int i = 0; i < 4; i++) pix[i*3+2] = 0;

                std::vector<casa::Gaussian2D<Double> > fitSet = this->itsBestFitMap["best"].fitSet();
                std::vector<casa::Gaussian2D<Double> >::iterator fit;

                for (fit = fitSet.begin(); fit < fitSet.end(); fit++) {
                    pix[0] = fit->xCenter();
                    pix[1] = fit->yCenter();
                    this->itsHeader.pixToWCS(pix, world);
                    stream.setf(std::ios::fixed);
                    stream.precision(6);

                    if (doEllipse) {
                        stream << "ELLIPSE "
                            << world[0] << " "
                            << world[1] << " "
                            << fit->majorAxis() * this->itsHeader.getAvPixScale() / 2. << " "
                            << fit->minorAxis() * this->itsHeader.getAvPixScale() / 2. << " "
                            << fit->PA() * 180. / M_PI << "\n";
                    }
                }

                pix[0] = pix[9] = this->getXmin() - this->itsFitParams.boxPadSize() - 0.5;
                pix[1] = pix[4] = this->getYmin() - this->itsFitParams.boxPadSize() - 0.5;
                pix[3] = pix[6] = this->getXmax() + this->itsFitParams.boxPadSize() + 0.5;
                pix[7] = pix[10] = this->getYmax() + this->itsFitParams.boxPadSize() + 0.5;
                this->itsHeader.pixToWCS(pix, world, 4);

                if (doBox) {
                    stream << "CLINES ";

                    for (int i = 0; i < 4; i++) stream << world[i*3] << " " << world[i*3+1] << " ";

                    stream << world[0] << " " << world[1] << "\n";
                }

                delete [] pix;
                delete [] world;
            }


        }

    }

}
