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
#include <duchamp/Outputs/columns.hh>
#include <duchamp/Outputs/CatalogueSpecification.hh>
#include <duchamp/Outputs/AnnotationWriter.hh>
#include <duchamp/Outputs/KarmaAnnotationWriter.hh>
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
		this->itsFitParams = FittingParameters();
                this->itsNoiseLevel = this->itsFitParams.noiseLevel();
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
		this->itsFitParams = FittingParameters();
                this->itsNoiseLevel = this->itsFitParams.noiseLevel();
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
		if(start>=end){
		  ASKAPLOG_DEBUG_STR(logger, "RadioSource::defineBox failing : sec="<<sec.getSection()
				     <<", offsets: "<<this->xSubOffset << " " << this->ySubOffset << " " << this->zSubOffset
				     <<", mins: " <<this->getXmin() << " " << this->getYmin() << " " << this->getZmin()
				     <<", maxs: " <<this->getXmax() << " " << this->getYmax() << " " << this->getZmax()
				     <<", boxpadsize: " << fitParams.boxPadSize());
		  ASKAPTHROW(AskapError, "RadioSource::defineBox bad slicer: end("<<end<<") < start ("<<start<<")");
		}
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
	      else
		this->itsNoiseLevel = fitparams.noiseLevel();
            }

//D1.1.13   void RadioSource::setNoiseLevel(float *array, long *dim, int boxSize)
            void RadioSource::setNoiseLevel(float *array, size_t *dim, int boxSize)
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
                long xmax = min(int(dim[0]) - 1, this->xpeak + hw);
                long ymax = min(int(dim[1]) - 1, this->ypeak + hw);
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
		//		ASKAPLOG_DEBUG_STR(logger, "Setting noise level to " << this->itsNoiseLevel);

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

//D1.1.13       long dim[2]; dim[0] = this->boxXsize(); dim[1] = this->boxYsize();
                size_t dim[2]; dim[0] = this->boxXsize(); dim[1] = this->boxYsize();
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
 
//		// get distance between average centre and peak location
//		float dx = this->getXaverage() - this->getXPeak();
//                float dy = this->getYaverage() - this->getYPeak();
//
//                 if (hypot(dx, dy) > 2.) {
// // 		  ASKAPLOG_DEBUG_STR(logger, "Using antipus with " << cmpntlist.size() << " subcomponents");
// // 		  ASKAPLOG_DEBUG_STR(logger, "Component 0 = " << cmpntlist[0]);
// 		  // if this distance is suitably small, add two more
// 		  // subcomponents: the "antipus", being the
// 		  // rotational reflection of the peak about the
// 		  // average centre; and the peak itself.
//                     SubComponent antipus;
//                     antipus.setPA(cmpntlist[0].pa());
//                     antipus.setMajor(cmpntlist[0].maj());
//                     antipus.setMinor(cmpntlist[0].min());
//                     antipus.setX(this->getXPeak() + dx);
//                     antipus.setY(this->getYPeak() + dy);
// 		    antipus.setPeak(this->getPeakFlux());
// 		    cmpntlist.push_back(antipus);
		    
//                     SubComponent centre;
//                     centre.setPA(antipus.pa());
//                     centre.setMajor(antipus.maj());
//                     centre.setMinor(antipus.min());
//                     centre.setX(this->getXPeak());
//                     centre.setY(this->getYPeak());
// 		    centre.setPeak(this->getPeakFlux());
// 		    cmpntlist.push_back(centre);
// 		}

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
//D1.1.13       long dim[2]; dim[0] = this->boxXsize(); dim[1] = this->boxYsize();
                size_t dim[2]; dim[0] = this->boxXsize(); dim[1] = this->boxYsize();
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
//D1.1.13       long dim[2]; dim[0] = this->boxXsize(); dim[1] = this->boxYsize();
                size_t dim[2]; dim[0] = this->boxXsize(); dim[1] = this->boxYsize();
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

                        if (failed) ASKAPTHROW(AskapError, "RadioSource flux allocation failed on voxel (" << x << "," << y << "," << z << ")");

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

//D1.1.13   bool RadioSource::fitGauss(float *fluxArray, long *dimArray, FittingParameters &baseFitter)
            bool RadioSource::fitGauss(float *fluxArray, size_t *dimArray, FittingParameters &baseFitter)
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
 //D1.1.13              int j = x + y * dimArray[0];
                        size_t j = x + y * dimArray[0];

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

	      ASKAPLOG_INFO_STR(logger, "Fitting source " << this->name << " at RA=" << this->raS << ", Dec=" << this->decS 
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
		ASKAPLOG_DEBUG_STR(logger, "numSubThresh="<<this->itsFitParams.numSubThresholds());

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

			std::vector<SubComponent> cmpntListCopy(cmpntList);
                        if (*type == "psf") {
                            for (size_t i = 0; i < cmpntList.size(); i++) {
			      //cmpntList[i].setMajor(this->itsHeader.beam().area());
			      //cmpntList[i].setMinor(this->itsHeader.beam().area());			      
			      //cmpntList[i].setPA(0.);
                                cmpntListCopy[i].setMajor(this->itsHeader.beam().maj());
                                cmpntListCopy[i].setMinor(this->itsHeader.beam().min());			      
                                cmpntListCopy[i].setPA(this->itsHeader.beam().pa()*M_PI/180.);
                            }
                        }

                        int ctr = 0;
                        Fitter fit[this->itsFitParams.maxNumGauss()];
                        bool fitIsGood = false;
                        int bestFit = 0;
                        float bestRChisq = 9999.;

                        int maxGauss = std::min(this->itsFitParams.maxNumGauss(), int(f.size()));

			bool fitPossible=true;
			bool stopNow=false;
                        for (int g = 1; g <= maxGauss && fitPossible && !stopNow; g++) {
			    ASKAPLOG_DEBUG_STR(logger, "Number of Gaussian components = " << g);

                            fit[ctr].setParams(this->itsFitParams);
                            fit[ctr].setNumGauss(g);
                            fit[ctr].setEstimates(cmpntListCopy, this->itsHeader);
                            fit[ctr].setRetries();
                            fit[ctr].setMasks();
			    fitPossible = fit[ctr].fit(pos, f, sigma);
			    bool acceptable = fit[ctr].acceptable();

                            if (fitPossible && acceptable) {
                                if ((ctr == 0) || (fit[ctr].redChisq() < bestRChisq)) {
                                    fitIsGood = true;
                                    bestFit = ctr;
                                    bestRChisq = fit[ctr].redChisq();
                                }
                            }
			    stopNow = this->itsFitParams.stopAfterFirstGoodFit() && acceptable;
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
		    if(this->itsFitParams.useGuessIfBad()){
		      this->itsBestFitType = "guess";
		      // set the components to be at least as big as the beam
		      for (size_t i = 0; i < cmpntList.size(); i++) {
			casa::Gaussian2D<casa::Double> gauss=cmpntList[i].asGauss();
			if(cmpntList[i].maj()<this->itsHeader.beam().maj()){
			  cmpntList[i].setMajor(this->itsHeader.beam().maj());
			  cmpntList[i].setMinor(this->itsHeader.beam().min());			      
			  cmpntList[i].setPA(this->itsHeader.beam().pa()*M_PI/180.);
			}
			else cmpntList[i].setMinor(std::max(cmpntList[i].min(),double(this->itsHeader.beam().min())));
			// cmpntList[i].setFlux(gauss.flux());
		      }
		      FitResults guess;
		      guess.saveGuess(cmpntList);
		      this->itsBestFitMap["guess"] = guess;
		      this->itsBestFitMap["best"] = guess;
		      ASKAPLOG_INFO_STR(logger, "No good fit found, so saving initial guess as the fit result");
		      this->itsBestFitMap["best"].logIt("INFO");
		    }
                    else ASKAPLOG_INFO_STR(logger, "No good fit found.");
                }

                ASKAPLOG_INFO_STR(logger, "-----------------------");
                return this->hasFit;
            }

            //**************************************************************//

	  void RadioSource::findSpectralTerm(std::string imageName, int term, bool doCalc)
            {
                /// @details This function finds the value of the spectral
                /// index or spectral curvature for each Gaussian component fitted to the zeroth
                /// Taylor term image. The procedure is:
                /// @li Find the Taylor 1/2 image from the provided image
                /// name (must be of format *.taylor.0*)
                /// @li Extract pixel values within the source's box
                /// @li For each Gaussian component of the source, and for
                /// each fit type, fit the same shape & location Gaussian
                /// (that is, only fit the height of the Gaussian).
                /// @li Calculate the spectral index or curvature using the appropriate formulae
                /// @li Store the spectral index value in a map indexed by fit type.
                /// Note that if the imageName provided is not of the correct format, nothing is done.
                /// @param imageName The name of the image in which sources have been found
	        /// @param term Which Taylor term to do - either 1 or 2, other values trigger an exception
                /// @param doCalc If true, do all the calculations. If false, fill the alpha & beta values to 0. for all fit types.

	      std::string termtype[3]={"","spectral index","spectral curvature"};

	        ASKAPCHECK(term==1 || term==2, 
			   "Term number ("<<term<<") must be either 1 (for spectral index) or 2 (for spectral curvature)");

                size_t pos = imageName.rfind(".taylor.0");

                if (!doCalc || pos == std::string::npos) {
                    // image provided is not a Taylor series term - notify and do nothing
                    if (doCalc)
                        ASKAPLOG_WARN_STR(logger, "radioSource::findSpectralTerm : Image name provided ("
                                              << imageName << ") is not a Taylor term. Cannot find spectral information.");

                    std::vector<std::string>::iterator type;
                    std::vector<std::string> typelist = availableFitTypes;
		    typelist.push_back("best");

                    for (type = typelist.begin(); type < typelist.end(); type++) {
		      if(term==1)     this->itsAlphaMap[*type] = std::vector<float>(this->itsBestFitMap[*type].numFits(), 0.);
		      else if(term==2) this->itsBetaMap[*type] = std::vector<float>(this->itsBestFitMap[*type].numFits(), 0.);
		      
                    }

                } else {

		  ASKAPLOG_DEBUG_STR(logger, "About to find the "<<termtype[term]<<", for image " << imageName);

                    // Get Taylor-term image name
		  std::stringstream ss;
		  ss << ".taylor."<<term;
		  std::string taylorName = imageName.replace(pos, 9, ss.str());

		  ASKAPLOG_DEBUG_STR(logger, "Using Taylor "<<term<<" image " << taylorName);

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
		    
		    Slice xrange=casa::Slice(this->boxXmin()+this->getXOffset(),this->boxXmax()-this->boxXmin()+1,1);
		    Slice yrange=casa::Slice(this->boxYmin()+this->getYOffset(),this->boxYmax()-this->boxYmin()+1,1);
		    Slicer theBox=casa::Slicer(xrange, yrange);
                    casa::Vector<casa::Double> f = getPixelsInBox(taylorName, theBox);

                    ASKAPLOG_DEBUG_STR(logger, "Preparing the fit for the taylor "<<term<<" term");

                    // Set up fit with same parameters and do the fit
                    std::vector<std::string>::iterator type;
                    std::vector<std::string> typelist = availableFitTypes;

                    for (type = typelist.begin(); type < typelist.end(); type++) {
                        std::vector<float> termValues(this->itsBestFitMap[*type].numGauss(), 0.);

                        if (this->itsBestFitMap[*type].isGood()){
			    ASKAPLOG_DEBUG_STR(logger, "Finding "<<termtype[term]<<" values for fit type \"" << *type << "\", with " << this->itsBestFitMap[*type].numGauss() << " components ");
                            Fitter fit;
                            fit.setParams(this->itsFitParams);
                            fit.rparams().setFlagFitThisParam("height");
                            fit.rparams().setNegativeFluxPossible(true);
                            fit.setNumGauss(this->itsBestFitMap[*type].numGauss());
// 			    ASKAPLOG_DEBUG_STR(logger, "Setting estimate with the following:");
// 			    this->itsBestFitMap[*type].logIt("DEBUG");
                            fit.setEstimates(this->itsBestFitMap[*type].getCmpntList(), this->itsHeader);
                            fit.setRetries();
                            fit.setMasks();
                            bool fitPossible = fit.fit(pos, f, sigma);

                            // Calculate taylor term value

                            if (fitPossible && fit.passConverged() && fit.passChisq()) { // the fit is OK
			      ASKAPLOG_DEBUG_STR(logger, "Values for "<<termtype[term]<<" follow (" 
						 << this->itsBestFitMap[*type].numGauss() << " of them):");

			      for (unsigned int i = 0; i < this->itsBestFitMap[*type].numGauss(); i++) {
				    float Iref=this->itsBestFitMap[*type].gaussian(i).flux();
                                    if(term==1){
				      termValues[i] = fit.gaussian(i).flux() / Iref;
				    }
				    else if(term==2){
				      float alpha = this->itsAlphaMap[*type][i];
				      termValues[i] = fit.gaussian(i).flux() / Iref - 0.5 * alpha * (alpha - 1.);
				    }
				      ASKAPLOG_DEBUG_STR(logger, "   Component " << i << ": " << termValues[i] << ", calculated with fitted flux of " << fit.gaussian(i).flux()<<", peaking at "<<fit.gaussian(i).height()<<", best fit taylor0 flux of " << Iref);
                                }
                            }

                        }

			if(term==1) this->itsAlphaMap[*type] = termValues;
			else if(term==2) this->itsBetaMap[*type] = termValues;
                    }

                    ASKAPLOG_DEBUG_STR(logger, "Finished finding the "<<termtype[term]<<" values");

                }
		
		if(term==1)       this->itsAlphaMap["best"] = this->itsAlphaMap[this->itsBestFitType];
		else if(term==2)  this->itsBetaMap["best"] = this->itsBetaMap[this->itsBestFitType];

            }


            //**************************************************************//

	  duchamp::Catalogues::CatalogueSpecification fullCatalogue(duchamp::Catalogues::CatalogueSpecification inputSpec, duchamp::FitsHeader &header)
	  {

	    /// @todo Make this a more obvious parameter to change
	    const int fluxPrec = 8;
	    const int fluxWidth = fluxPrec + 12;

	    duchamp::Catalogues::CatalogueSpecification newSpec;
	    newSpec.addColumn(inputSpec.column("NUM"));
	    newSpec.addColumn(inputSpec.column("NAME"));
	    newSpec.addColumn(inputSpec.column("RAJD"));
	    newSpec.addColumn(inputSpec.column("DECJD"));
	    newSpec.addColumn(inputSpec.column("FINT"));
	    newSpec.addColumn(inputSpec.column("FPEAK"));
	    
	    newSpec.column("FINT").changePrec(fluxPrec);
	    newSpec.column("FPEAK").changePrec(fluxPrec);
	    newSpec.column("NUM").setName("ID");
	    newSpec.column("NUM").setDatatype("char");
	    // new columns
	    newSpec.addColumn( duchamp::Catalogues::Column("FINTFIT","F_int(fit)", header.getIntFluxUnits(), fluxWidth, fluxPrec,"phot.flux","float","col_fint_fit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("FPEAKFIT","F_pk(fit)", header.getFluxUnits(), fluxWidth, fluxPrec,"phot.flux","float","col_fint_fit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("MAJFIT","Maj(fit)", "", 10, 3,"","float","col_maj_fit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("MINFIT","Min(fit)", "", 10, 3,"","float","col_min_fit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("PAFIT","P.A.(fit)", "", 10, 2,"","float","col_pa_fit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("MAJDECONV","Maj(fit_deconv.)", "", 17, 3,"","float","col_maj_deconv","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("MINDECONV","Min(fit_deconv.)", "", 17, 3,"","float","col_min_deconv","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("PADECONV","P.A.(fit_deconv.)", "", 18, 2,"","float","col_pa_deconv","") ); 
	    newSpec.addColumn( duchamp::Catalogues::Column("ALPHA","Alpha", "", 11, 5,"","float","col_alpha","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("BETA","Beta", "", 11, 5,"","float","col_beta","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("CHISQFIT","Chisq(fit)", "", 27, 9,"","float","col_chisqfit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("RMSIMAGE","RMS(image)", header.getFluxUnits(), fluxWidth, fluxPrec,"","float","col_rmsimage","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("RMSFIT","RMS(fit)", "", fluxWidth, fluxPrec,"","float","col_rmsfit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("NFREEFIT","Nfree(fit)", "", 11, 0,"","int","col_nfreefit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("NDOFFIT","NDoF(fit)", "", 10, 0,"","int","col_ndoffit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("NPIXFIT","NPix(fit)", "", 10, 0,"","int","col_npixfit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("NPIXOBJ","NPix(obj)", "", 10, 0,"","int","col_npixobj","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("GUESS","Guess?","",7,0,"","int","col_guess","") );
    
	    return newSpec;
	  }


            void RadioSource::printSummary(std::ostream &stream, duchamp::Catalogues::CatalogueSpecification columns,
                                           std::string fittype, bool doHeader)
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

		duchamp::Catalogues::CatalogueSpecification newSpec;
		newSpec.addColumn(columns.column("NUM"));
		newSpec.addColumn(columns.column("NAME"));
		newSpec.addColumn(columns.column("RAJD"));
		newSpec.addColumn(columns.column("DECJD"));
		newSpec.addColumn(columns.column("FINT"));
		newSpec.addColumn(columns.column("FPEAK"));

                newSpec.column("FINT").changePrec(fluxPrec);
                newSpec.column("FPEAK").changePrec(fluxPrec);
                newSpec.column("NUM").setName("ID");
                // new columns
                newSpec.addColumn( duchamp::Catalogues::Column("FINTFIT","F_int(fit)", "["+this->itsHeader.getIntFluxUnits()+"]", fluxWidth, fluxPrec,"phot.flux","float","col_fint_fit","") );
                newSpec.addColumn( duchamp::Catalogues::Column("FPEAKFIT","F_pk(fit)", "["+this->itsHeader.getFluxUnits()+"]", fluxWidth, fluxPrec,"phot.flux","float","col_fint_fit","") );
                newSpec.addColumn( duchamp::Catalogues::Column("MAJFIT","Maj(fit)", "", 10, 3,"","float","col_","") );
                newSpec.addColumn( duchamp::Catalogues::Column("MINFIT","Min(fit)", "", 10, 3,"","float","col_","") );
                newSpec.addColumn( duchamp::Catalogues::Column("PAFIT","P.A.(fit)", "", 10, 2,"","float","col_","") );
                newSpec.addColumn( duchamp::Catalogues::Column("MAJDECONV","Maj(fit_deconv.)", "", 17, 3,"","float","col_","") );
                newSpec.addColumn( duchamp::Catalogues::Column("MINDECONV","Min(fit_deconv.)", "", 17, 3,"","float","col_","") );
                newSpec.addColumn( duchamp::Catalogues::Column("PADECONV","P.A.(fit_deconv.)", "", 18, 2,"","float","col_","") );		
		newSpec.addColumn( duchamp::Catalogues::Column("ALPHA","Alpha", "", 11, 3,"","float","col_","") );
		newSpec.addColumn( duchamp::Catalogues::Column("BETA","Beta", "", 11, 3,"","float","col_","") );
                newSpec.addColumn( duchamp::Catalogues::Column("CHISQFIT","Chisq(fit)", "", 27, 9,"","float","col_","") );
                newSpec.addColumn( duchamp::Catalogues::Column("RMSIMAGE","RMS(image)", "", fluxWidth, fluxPrec,"","float","col_","") );
                newSpec.addColumn( duchamp::Catalogues::Column("RMSFIT","RMS(fit)", "", fluxWidth, fluxPrec,"","float","col_","") );
                newSpec.addColumn( duchamp::Catalogues::Column("NFREEFIT","Nfree(fit)", "", 11, 0,"","int","col_","") );
                newSpec.addColumn( duchamp::Catalogues::Column("NDOFFIT","NDoF(fit)", "", 10, 0,"","int","col_","") );
                newSpec.addColumn( duchamp::Catalogues::Column("NPIXFIT","NPix(fit)", "", 10, 0,"","int","col_","") );
                newSpec.addColumn( duchamp::Catalogues::Column("NPIXOBJ","NPix(obj)", "", 10, 0,"","int","col_","") );
		newSpec.addColumn( duchamp::Catalogues::Column("GUESS","Guess?","",7,0,"","int","col_","") );

                if (doHeader) {
                    stream << "#";
		    for(size_t i=0;i<newSpec.size();i++) newSpec.column(i).printTitle(stream);
                    stream << "\n";
		    int width=0;
		    for(size_t i=0;i<newSpec.size();i++) width += newSpec.column(i).getWidth();
                    stream << "#" << std::setfill('-') << std::setw(width) << '-' << "\n";
                }

                columns.column("NUM").widen(); // to account for the # characters at the start of the title lines

                if (!results.isGood() && !results.fitIsGuess()) { //if no fits were made, and we haven't got a guess stored
                    float zero = 0.;
                    newSpec.column("NUM").printEntry(stream, this->getID());
		    newSpec.column("NAME").printEntry(stream, this->getName());
                    newSpec.column("RAJD").printEntry(stream, this->getRA());
                    newSpec.column("DECJD").printEntry(stream, this->getDec());
                    newSpec.column("FINT").printEntry(stream, this->getIntegFlux());
                    newSpec.column("FPEAK").printEntry(stream, this->getPeakFlux());
                    newSpec.column("FINTFIT").printEntry(stream, zero);
                    newSpec.column("FPEAKFIT").printEntry(stream, zero);
                    newSpec.column("MAJFIT").printEntry(stream, zero);
                    newSpec.column("MINFIT").printEntry(stream, zero);
                    newSpec.column("PAFIT").printEntry(stream, zero);
                    newSpec.column("MAJDECONV").printEntry(stream, zero);
                    newSpec.column("MINDECONV").printEntry(stream, zero);
                    newSpec.column("PADECONV").printEntry(stream, zero);
		    newSpec.column("ALPHA").printEntry(stream, zero);
		    newSpec.column("BETA").printEntry(stream, zero);
                    newSpec.column("CHISQFIT").printEntry(stream, zero);
                    newSpec.column("RMSIMAGE").printEntry(stream, this->itsNoiseLevel);
                    newSpec.column("RMSFIT").printEntry(stream, zero);
                    newSpec.column("NFREEFIT").printEntry(stream, zero);
                    newSpec.column("NDOFFIT").printEntry(stream, zero);
                    newSpec.column("NPIXFIT").printEntry(stream, zero);
                    newSpec.column("NPIXOBJ").printEntry(stream, this->getSize());
		    newSpec.column("GUESS").printEntry(stream, zero);
                    stream << "\n";
                } else {
                    std::vector<casa::Gaussian2D<Double> > fitSet = results.fitSet();
                    std::vector<casa::Gaussian2D<Double> >::iterator fit = fitSet.begin();
                    std::vector<float>::iterator alphaIter = this->itsAlphaMap[fittype].begin();
                    std::vector<float>::iterator betaIter = this->itsBetaMap[fittype].begin();
                    ASKAPCHECK(fitSet.size() == this->itsAlphaMap[fittype].size(),
                               "Sizes of fitSet (" << fitSet.size() << ") and alpha Set (" << this->itsAlphaMap[fittype].size() << ") for fittype '" << fittype << "' don't match!");
                    ASKAPCHECK(fitSet.size() == this->itsBetaMap[fittype].size(),
                               "Sizes of fitSet (" << fitSet.size() << ") and beta Set (" << this->itsBetaMap[fittype].size() << ") for fittype '" << fittype << "' don't match!");

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

                        newSpec.column("NUM").printEntry(stream, id.str());
			newSpec.column("NAME").printEntry(stream, this->getName());
                        newSpec.column("RAJD").printEntry(stream, thisRA);
                        newSpec.column("DECJD").printEntry(stream, thisDec);
                        newSpec.column("FINT").printEntry(stream, this->getIntegFlux());
                        newSpec.column("FPEAK").printEntry(stream, this->getPeakFlux());
                        newSpec.column("FINTFIT").printEntry(stream, intfluxfit);
                        newSpec.column("FPEAKFIT").printEntry(stream, fit->height());
                        newSpec.column("MAJFIT").printEntry(stream, fit->majorAxis()*this->itsHeader.getAvPixScale()*3600.); // convert from pixels to arcsec
                        newSpec.column("MINFIT").printEntry(stream, fit->minorAxis()*this->itsHeader.getAvPixScale()*3600.);
                        newSpec.column("PAFIT").printEntry(stream, fit->PA()*180. / M_PI);
                        newSpec.column("MAJDECONV").printEntry(stream, deconv[0]*this->itsHeader.getAvPixScale()*3600.); // convert from pixels to arcsec
                        newSpec.column("MINDECONV").printEntry(stream, deconv[1]*this->itsHeader.getAvPixScale()*3600.);
                        newSpec.column("PADECONV").printEntry(stream, deconv[2]*180. / M_PI);
			newSpec.column("ALPHA").printEntry(stream, *alphaIter);
			newSpec.column("BETA").printEntry(stream, *betaIter);
                        newSpec.column("CHISQFIT").printEntry(stream, results.chisq());
                        newSpec.column("RMSIMAGE").printEntry(stream, this->itsNoiseLevel);
                        newSpec.column("RMSFIT").printEntry(stream, results.RMS());
                        newSpec.column("NFREEFIT").printEntry(stream, results.numFreeParam());
                        newSpec.column("NDOFFIT").printEntry(stream, results.ndof());
                        newSpec.column("NPIXFIT").printEntry(stream, results.numPix());
                        newSpec.column("NPIXOBJ").printEntry(stream, this->getSize());
			newSpec.column("GUESS").printEntry(stream, results.fitIsGuess() ? 1 : 0);
                        stream << "\n";
                    }
                }
            }

            //**************************************************************//

	  void RadioSource::printTableRow(std::ostream &stream, duchamp::Catalogues::CatalogueSpecification columns, size_t fitNum, std::string fitType)
	  {
	    /// @details
	    ///  Print a row of values for the current Detection into an output
	    ///  table. Columns are printed according to the tableType string,
	    ///  using the Column::doCol() function as a determinant.
	    /// \param stream Where the output is written
	    /// \param columns The vector list of Column objects
	    /// \param tableType A Catalogues::DESTINATION label saying what format to use: one of
	    /// FILE, LOG, SCREEN or VOTABLE (although the latter
	    /// shouldn't be used with this function).
	    
	    stream.setf(std::ios::fixed);  
	    for(size_t i=0;i<columns.size();i++) this->printTableEntry(stream, columns.column(i), fitNum, fitType);
	    stream << "\n";
	    
	  }

	  void RadioSource::printTableEntry(std::ostream &stream, duchamp::Catalogues::Column column, size_t fitNum, std::string fitType)
	  {
	    /// @details
	    ///  Print a single value into an output table. The Detection's
	    ///  correct value is extracted according to the Catalogues::COLNAME
	    ///  key in the column given.
	    /// \param stream Where the output is written
	    /// \param column The Column object defining the formatting.
	    
	    ASKAPCHECK(fitNum<this->itsBestFitMap[fitType].numFits(),"fitNum="<<fitNum<<", but source "<<
		       this->getID()<<" only has "<<this->itsBestFitMap[fitType].numFits()<<" fits for type " << fitType);
	    FitResults results = this->itsBestFitMap[fitType];
	    casa::Gaussian2D<Double> gauss = this->itsBestFitMap[fitType].gaussian(fitNum);
	    char firstSuffix = 'a';
	    std::stringstream id;
	    id << this->getID() << char(firstSuffix + fitNum);
	    std::vector<Double> deconv = deconvolveGaussian(gauss,this->itsHeader.getBeam());
	    double *pix = new double[3];
	    pix[0] = gauss.xCenter();
	    pix[1] = gauss.yCenter();
	    pix[2] = this->getZcentre();
	    double *wld = new double[3];
	    this->itsHeader.pixToWCS(pix, wld);
	    double thisRA = wld[0];
	    double thisDec = wld[1];
	    delete [] pix;
	    delete [] wld;
	    float intfluxfit = gauss.flux();
	    if (this->itsHeader.needBeamSize())
	      intfluxfit /= this->itsHeader.beam().area(); // Convert from Jy/beam to Jy

	    std::string type=column.type();	    
	    if(type=="NUM")  column.printEntry(stream, id.str());
	    else if(type=="NAME")  column.printEntry(stream, this->getName());
	    else if(type=="RAJD")  column.printEntry(stream, thisRA);
	    else if(type=="DECJD")  column.printEntry(stream, thisDec);
	    else if(type=="FINT")  column.printEntry(stream, this->getIntegFlux());
	    else if(type=="FPEAK")  column.printEntry(stream, this->getPeakFlux());
	    else if(type=="FINTFIT")  column.printEntry(stream, intfluxfit);
	    else if(type=="FPEAKFIT")  column.printEntry(stream, results.gaussian(fitNum).height());
	    else if(type=="MAJFIT")  column.printEntry(stream, gauss.majorAxis()*this->itsHeader.getAvPixScale()*3600.); // convert from pixels to arcsec
	    else if(type=="MINFIT")  column.printEntry(stream, gauss.minorAxis()*this->itsHeader.getAvPixScale()*3600.);
	    else if(type=="PAFIT")  column.printEntry(stream, gauss.PA()*180. / M_PI);
	    else if(type=="MAJDECONV")  column.printEntry(stream, deconv[0]*this->itsHeader.getAvPixScale()*3600.); // convert from pixels to arcsec
	    else if(type=="MINDECONV")  column.printEntry(stream, deconv[1]*this->itsHeader.getAvPixScale()*3600.);
	    else if(type=="PADECONV")  column.printEntry(stream, deconv[2]*180. / M_PI);
	    else if(type=="ALPHA")  column.printEntry(stream, this->itsAlphaMap[fitType][fitNum]);
	    else if(type=="BETA")  column.printEntry(stream, this->itsBetaMap[fitType][fitNum]);
	    else if(type=="CHISQFIT")  column.printEntry(stream, results.chisq());
	    else if(type=="RMSIMAGE")  column.printEntry(stream, this->itsNoiseLevel);
	    else if(type=="RMSFIT")  column.printEntry(stream, results.RMS());
	    else if(type=="NFREEFIT")  column.printEntry(stream, results.numFreeParam());
	    else if(type=="NDOFFIT")  column.printEntry(stream, results.ndof());
	    else if(type=="NPIXFIT")  column.printEntry(stream, results.numPix());
	    else if(type=="NPIXOBJ")  column.printEntry(stream, this->getSize());
	    else if(type=="GUESS")  column.printEntry(stream, results.fitIsGuess() ? 1 : 0);
	    else this->duchamp::Detection::printTableEntry(stream,column); // handles anything covered by duchamp code. If different column, use the following.
	  }

	  void RadioSource::writeFitToAnnotationFile(duchamp::AnnotationWriter *writer, bool doEllipse, bool doBox)
	  {
	                   double *pix = new double[12];
                double *world = new double[12];

                for (int i = 0; i < 4; i++) pix[i*3+2] = 0;

                std::vector<casa::Gaussian2D<Double> > fitSet = this->itsBestFitMap["best"].fitSet();
                std::vector<casa::Gaussian2D<Double> >::iterator fit;

                for (fit = fitSet.begin(); fit < fitSet.end(); fit++) {
                    pix[0] = fit->xCenter();
                    pix[1] = fit->yCenter();
                    this->itsHeader.pixToWCS(pix, world);

                    if (doEllipse) {
		      writer->ellipse(world[0],
				     world[1],
				     fit->majorAxis() * this->itsHeader.getAvPixScale() / 2.,
				     fit->minorAxis() * this->itsHeader.getAvPixScale() / 2.,
				     fit->PA() * 180. / M_PI);
                    }
                }
		
		pix[0] = pix[9] = this->getXmin() - this->itsFitParams.boxPadSize() - 0.5;
                pix[1] = pix[4] = this->getYmin() - this->itsFitParams.boxPadSize() - 0.5;
                pix[3] = pix[6] = this->getXmax() + this->itsFitParams.boxPadSize() + 0.5;
                pix[7] = pix[10] = this->getYmax() + this->itsFitParams.boxPadSize() + 0.5;
                this->itsHeader.pixToWCS(pix, world, 4);

                if (doBox) {
		  std::vector<double> x,y;
		  for(int i=0;i<=4;i++){
		    x.push_back(world[(i%4)*3]);
		    y.push_back(world[(i%4)*3+1]);
		  }
		  writer->joinTheDots(x,y);
                }

                delete [] pix;
                delete [] world;

	  }

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





	  void SortDetections(std::vector<RadioSource> &sourcelist, std::string parameter)
	    {
	      /// @details This function sorts a vector list of
	      /// RadioSource objects, using the same functionality as
	      /// the duchamp library's SortDetections function. The
	      /// objects are sorted as duchamp::Detection objects,
	      /// keeping track of their individual identities by
	      /// using the ID field. The original ID field is
	      /// replaced at the end.

	      std::vector<duchamp::Detection> detlist(sourcelist.size());
	      int ID=0;
	      std::map<string,int> posMap;
	      for(std::vector<RadioSource>::iterator src=sourcelist.begin();src<sourcelist.end();src++){

		std::stringstream ss;
		ss << src->getXPeak() << "_"<<src->getYPeak()<< "_"<<src->getZPeak();
		posMap.insert(std::pair<string,int>(ss.str(),ID));
		detlist[ID] = duchamp::Detection(*src);
		ID++;

	      }

	      duchamp::SortDetections(detlist,parameter);
	      
	      std::vector<RadioSource> newSourcelist(sourcelist.size());

	      for(size_t i=0;i<detlist.size();i++){
		std::stringstream ss;
		ss << detlist[i].getXPeak() << "_"<<detlist[i].getYPeak()<< "_"<<detlist[i].getZPeak();
		ID=posMap[ss.str()];
		newSourcelist[i] = sourcelist[ID];
	      }

	      sourcelist = newSourcelist;
	      

	    }




        }

    }

}
