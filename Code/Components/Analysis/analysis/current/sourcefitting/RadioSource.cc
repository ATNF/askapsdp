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
#include <analysisparallel/SubimageDef.h>
#include <casainterface/CasaInterface.h>

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
#include <duchamp/Detection/finders.hh>

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
#include <string>
#include <algorithm>
#include <utility>
#include <math.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".radioSource");

using namespace duchamp;
using namespace askap::analysisutilities;

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

            RadioSource& RadioSource::operator= (const duchamp::Detection& det)
            {
	      ((duchamp::Detection &) *this) = det;
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
	      return *this;
            }

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

	    void RadioSource::addOffsets(long xoff, long yoff, long zoff)
	    {
		/// @details Reimplementing the addOffsets function
		/// from Detection. It runs the Detection version,
		/// then adds the offsets to each of the component
		/// positions.

		this->Detection::addOffsets(xoff, yoff, zoff);
		
                std::map<std::string, FitResults>::iterator fit;
                for (fit = this->itsBestFitMap.begin(); fit != this->itsBestFitMap.end(); fit++) {
		    std::vector<casa::Gaussian2D<Double> >::iterator gauss;
		    for(gauss=fit->second.fits().begin();gauss!=fit->second.fits().end(); gauss++){
			gauss->setXcenter(gauss->xCenter() + xoff);
			gauss->setYcenter(gauss->yCenter() + yoff); 
		    }
                }
		
	    }

            //**************************************************************//

            void RadioSource::defineBox(duchamp::Section &sec, FittingParameters &fitParams, int spectralAxis)
            {
                /// @details Defines the maximum and minimum points of the box
                /// in each axis direction. The size of the image array is
                /// taken into account, using the axes array, so that the box
                /// does not go outside the allowed pixel area.

		int ndim=(spectralAxis>=0) ? 3 : 2;
	        casa::IPosition start(ndim, 0), end(ndim, 0), stride(ndim, 1);
                start(0) = std::max(long(sec.getStart(0) - this->xSubOffset), this->getXmin() - fitParams.boxPadSize());
                end(0)   = std::min(long(sec.getEnd(0) - this->xSubOffset), this->getXmax() + fitParams.boxPadSize());
                start(1) = std::max(long(sec.getStart(1) - this->ySubOffset), this->getYmin() - fitParams.boxPadSize());
                end(1)   = std::min(long(sec.getEnd(1) - this->ySubOffset), this->getYmax() + fitParams.boxPadSize());
		if(spectralAxis>=0){
		  start(2) = std::max(long(sec.getStart(spectralAxis) - this->zSubOffset), this->getZmin() - fitParams.boxPadSize());
		  end(2)   = std::min(long(sec.getEnd(spectralAxis) - this->zSubOffset), this->getZmax() + fitParams.boxPadSize());
		}
		// ASKAPLOG_DEBUG_STR(logger, "DefineBox: start = " << start << " end="<<end << " section="<<sec.getSection() 
		// 		   << ", sec.start: " << sec.getStart(0) << " " << sec.getStart(1) << " " << sec.getStart(spectralAxis)
		// 		   << ", sec.end: " << sec.getEnd(0) << " " << sec.getEnd(1) << " " << sec.getEnd(spectralAxis)
		// 		     <<", offsets: "<<this->xSubOffset << " " << this->ySubOffset << " " << this->zSubOffset
		// 		     <<", mins: " <<this->getXmin() << " " << this->getYmin() << " " << this->getZmin()
		// 		     <<", maxs: " <<this->getXmax() << " " << this->getYmax() << " " << this->getZmax()
		// 		     <<", boxpadsize: " << fitParams.boxPadSize());
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

	    std::string RadioSource::boundingSubsection(std::vector<size_t> dim, duchamp::FitsHeader *header, FittingParameters *fitparam, bool fullSpectralRange)
	    {
		/// @details This function returns a subsection string
		/// that shows the bounding box for the object. This
		/// will be in a suitable format for use with the
		/// subsection string in the input parameter set. It
		/// uses the FitsHeader object to know which axis
		/// belongs where.

		const int lng=header->getWCS()->lng;
		const int lat=header->getWCS()->lat;
		const int spec=header->getWCS()->spec;
		std::vector<std::string> sectionlist(dim.size(),"1:1");
		long min,max;
		for(int ax=0;ax<int(dim.size());ax++){
		    std::stringstream ss;
		    if (ax==spec){
			if (fullSpectralRange) ss << "1:"<<dim[ax]+1;
			else ss << std::max(1L,this->zmin-fitparam->boxPadSize()+1)<<":"<<std::min(long(dim[ax]),this->zmax+fitparam->boxPadSize()+1);
		    }
		    else if(ax==lng){
			min = this->xmin-fitparam->boxPadSize()+1;
			max = this->xmax+fitparam->boxPadSize()+1;
			if(fitparam->useNoise()){
			    min = std::min(min, this->xpeak-fitparam->noiseBoxSize()/2);
			    max = std::max(max, this->xpeak+fitparam->noiseBoxSize()/2);
			}
			ss << std::max(1L,min)<<":"<<std::min(long(dim[ax]),max);
		    }
		    else if (ax==lat){
			min = this->ymin-fitparam->boxPadSize()+1;
			max = this->ymax+fitparam->boxPadSize()+1;
			if(fitparam->useNoise()){
			    min = std::min(min, this->ypeak-fitparam->noiseBoxSize()/2);
			    max = std::max(max, this->ypeak+fitparam->noiseBoxSize()/2);
			}
			ss << std::max(1L,min)<<":"<<std::min(long(dim[ax]),max);
		    }
		    else
			ss << "1:1";
		    sectionlist[ax]=ss.str();
		}
		std::stringstream secstr;
		secstr << "[ " << sectionlist[0];
		for(size_t i=1;i<dim.size();i++) secstr << "," << sectionlist[i];
		secstr << "]";

		return secstr.str();
	    }


            //**************************************************************//

            void RadioSource::setAtEdge(duchamp::Cube &cube, analysisutilities::SubimageDef &subimage, int workerNum)
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
                    madfm += localArray[size/2-1];
                    madfm /= 2.;
                }

                this->itsNoiseLevel = Statistics::madfmToSigma(madfm);
		//		ASKAPLOG_DEBUG_STR(logger, "Setting noise level to " << this->itsNoiseLevel);

                delete [] localArray;
            }


            //**************************************************************//

	    void RadioSource::setDetectionThreshold(duchamp::Cube &cube, bool flagMedianSearch, std::string snrImage)
	  {
	    

	    if (flagMedianSearch) {

	      std::vector<PixelInfo::Voxel> voxSet = this->getPixelSet();

	      casa::IPosition globalOffset(this->itsBox.start().size(),0);
	      globalOffset[0] = cube.pars().getXOffset();
	      globalOffset[1] = cube.pars().getYOffset();
	      casa::Slicer fullImageBox(this->itsBox.start()+globalOffset,this->itsBox.length(),Slicer::endIsLength);
	      casa::Array<float> snrArray = analysisutilities::getPixelsInBox(snrImage,fullImageBox,false);

	      std::vector<PixelInfo::Voxel>::iterator vox = voxSet.begin();
	      this->itsDetectionThreshold = cube.getPixValue(vox->getX(), vox->getY(), vox->getZ());
	      int loc = (vox->getX() - this->boxXmin()) + this->boxXsize() * (vox->getY() - this->boxYmin());
	      this->peakSNR = snrArray.data()[loc];
	      
	      for (; vox < voxSet.end(); vox++) {
		  loc = (vox->getX() - this->boxXmin()) + this->boxXsize() * (vox->getY() - this->boxYmin());
		  this->peakSNR = std::max(this->peakSNR,snrArray.data()[loc]);
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
                /// using the principal axes and position angle calculated in
                /// the duchamp::PixelInfo code. This is done by using the
                /// array of flux values given by f, thresholding at half the
                /// object's peak flux value, and averaging the x- and
                /// y-widths that the Duchamp code gives.
                ///
                /// It may be that the thresholding returns more than one
                /// object. In this case, we only look at the one with the
                /// same peak location as the base object.

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
                        std::pair<double, double> axes = o->getPrincipalAxes();
                        maj = std::max(axes.first, axes.second);
                        min = std::min(axes.first, axes.second);
                    }

                }

                delete smlIm;
            }

            //**************************************************************//

            std::vector<SubComponent> RadioSource::getSubComponentList(casa::Matrix<casa::Double> pos, casa::Vector<casa::Double> &f)
            {
		std::vector<SubComponent> cmpntlist;
		if(this->itsFitParams.useCurvature()){

		    // 1. get array of curvature from curvature map
		    // 2. define bool array of correct size
		    // 3. value of this is = (isInObject) && (curvature < -sigmaCurv)
		    // 4. run lutz_detect to get list of objects
		    // 5. for each object, define a subcomponent of zero size with correct peak & position

		    casa::IPosition globalOffset(this->itsBox.start().size(),0);
		    globalOffset[0] = this->xSubOffset;
		    globalOffset[1] = this->ySubOffset;
		    //ASKAPLOG_DEBUG_STR(logger, "Defining slicer with offset " << globalOffset << " applied to box start " << this->itsBox.start() <<" (giving " << this->itsBox.start()+globalOffset << ") and length " << this->itsBox.length());
		    casa::Slicer fullImageBox(this->itsBox.start()+globalOffset,this->itsBox.length(),Slicer::endIsLength);
		    //ASKAPLOG_DEBUG_STR(logger, "Have slicer " << fullImageBox);

		    casa::Array<float> curvArray = analysisutilities::getPixelsInBox(this->itsFitParams.curvatureImage(),fullImageBox,false);

		    PixelInfo::Object2D spatMap = this->getSpatialMap();
		    size_t *dim = new size_t[2]; dim[0]=fullImageBox.length()[0]; dim[1]=fullImageBox.length()[1];
		    float *fluxArray = new float[fullImageBox.length().product()];
		    for (size_t i = 0; i < this->boxSize(); i++) fluxArray[i] = 0.;
		    std::vector<bool> summitMap(fullImageBox.length().product(),false);

		    //ASKAPLOG_DEBUG_STR(logger, "xmin="<< fullImageBox.start()[0] << " xsize=" << fullImageBox.length()[0] << " ymin=" << fullImageBox.start()[1] << " ysize=" << fullImageBox.length()[1] << " f.size="<<f.size());
		    for (size_t i = 0; i < f.size(); i++) {
			int x = int(pos(i, 0));
			int y = int(pos(i, 1));
			if (spatMap.isInObject(x, y)) {
			    int loc = (x - this->boxXmin()) + this->boxXsize() * (y - this->boxYmin());
			    fluxArray[loc] = float(f(i));
			    summitMap[loc] = (curvArray.data()[loc] < -1.*this->itsFitParams.sigmaCurv());
//			    ASKAPLOG_DEBUG_STR(logger, x << " " << y << " " << loc << " " << fluxArray[loc] << " "  << curvArray.data()[loc]<< " " << " " << this->itsFitParams.sigmaCurv()<< " " << summitMap[loc]);
			}
		    }

		    std::vector<Object2D> summitList = duchamp::lutz_detect(summitMap, this->boxXsize(), this->boxYsize(), 1);
		    ASKAPLOG_DEBUG_STR(logger, "Found " << summitList.size() << " summits");

		    duchamp::Param par;
		    par.setXOffset(fullImageBox.start()[0]);
		    par.setYOffset(fullImageBox.start()[1]);
		    for(std::vector<Object2D>::iterator obj=summitList.begin();obj<summitList.end();obj++){
			duchamp::Detection det;
			det.addChannel(0,*obj);
			det.calcFluxes(fluxArray,dim);
			det.setOffsets(par);
			det.addOffsets();
			SubComponent cmpnt;
			cmpnt.setPeak(det.getPeakFlux());
			// Need to correct the positions to put them in the current worker frame
			cmpnt.setX(det.getXPeak()-globalOffset[0]);
			cmpnt.setY(det.getYPeak()-globalOffset[1]);
			cmpnt.setPA(0.);
			cmpnt.setMajor(0.);
			cmpnt.setMinor(0.);
			cmpntlist.push_back(cmpnt);
			ASKAPLOG_DEBUG_STR(logger, "Found subcomponent " << cmpnt);
		    }
		    
		    delete dim;
		    delete fluxArray;

		}
		else{
		    SubThresholder subThresh;
		    subThresh.define(this, pos,f);
		    cmpntlist = subThresh.find();
		}

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
                size_t dim[2]; dim[0] = this->boxXsize(); dim[1] = this->boxYsize();
                duchamp::Image *smlIm = new duchamp::Image(dim);
                smlIm->saveArray(fluxarray, this->boxSize());
                smlIm->setMinSize(1);
                SubComponent base;
                base.setPeak(this->peakFlux);
                base.setX(this->xpeak);
                base.setY(this->ypeak);
                double a=0., b=0., c=0.;

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

                        for (size_t i = 0; i < this->boxSize(); i++) {
                            size_t xbox = i % this->boxXsize();
                            size_t ybox = i / this->boxXsize();
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
                size_t dim[2]; dim[0] = this->boxXsize(); dim[1] = this->boxYsize();
                duchamp::Image smlIm(dim);
                float *fluxarray = new float[this->boxSize()];

                for (size_t i = 0; i < this->boxSize(); i++) fluxarray[i] = f(i);

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

	  void RadioSource::prepareForFit(duchamp::Cube &cube, bool useArray)
	  {
	    
	    // Set up parameters for fitting.
	    if(useArray) this->setNoiseLevel(cube, this->itsFitParams);
	    else {
	      // if need to use the surrounding noise, we have to go extract it from the image
	      if (this->itsFitParams.useNoise() // && !cube->itsCube.pars().getFlagUserThreshold()
		  ) {
		float noise = findSurroundingNoise(cube.pars().getImageFile(), 
						   this->xpeak+this->xSubOffset, this->ypeak+this->ySubOffset, 
						   this->itsFitParams.noiseBoxSize());
		this->setNoiseLevel(noise);
	      } else this->setNoiseLevel(1);
	    }
	    
	    this->setHeader(cube.pHeader());
	    this->setOffsets(cube.pars());
	    if(!this->itsFitParams.doFit()) this->itsFitParams.setBoxPadSize(1);
	    this->defineBox(cube.pars().section(), this->itsFitParams, cube.header().getWCS()->spec);
	    
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

                if ( this->getZmin() != this->getZmax()) {
                    ASKAPLOG_ERROR_STR(logger, "Can only do fitting for two-dimensional objects!: z-locations show a spread: zmin="<<this->getZmin()<<", zmax="<<this->getZmax());
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
                        size_t i = (x - this->boxXmin()) + (y - this->boxYmin()) * this->boxXsize();
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

                for (long x = this->boxXmin(); x <= this->boxXmax(); x++) {
                    for (long y = this->boxYmin(); y <= this->boxYmax(); y++) {
                        size_t i = (x - this->boxXmin()) + (y - this->boxYmin()) * this->boxXsize();
                        size_t j = x + y * dimArray[0];

                        if (j < dimArray[0]*dimArray[1]) f(i) = fluxArray[j];
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
                                cmpntListCopy[i].setMajor(this->itsHeader->beam().maj());
                                cmpntListCopy[i].setMinor(this->itsHeader->beam().min());			      
                                cmpntListCopy[i].setPA(this->itsHeader->beam().pa()*M_PI/180.);
                            }
                        }

                        int ctr = 0;
			std::vector<Fitter> fit(this->itsFitParams.maxNumGauss());
                        bool fitIsGood = false;
                        int bestFit = 0;
                        float bestRChisq = 9999.;

			int minGauss = this->itsFitParams.numGaussFromGuess() ? cmpntListCopy.size() : 1;
                        int maxGauss = this->itsFitParams.numGaussFromGuess() ? cmpntListCopy.size() : std::min(this->itsFitParams.maxNumGauss(), int(f.size()));

			bool fitPossible=true;
			bool stopNow=false;
                        for (int g = minGauss; g <= maxGauss && fitPossible && !stopNow; g++) {
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
			if (!this->itsFitParams.numGaussFromGuess()){
			    cmpntList.resize( std::min(this->itsFitParams.maxNumGauss(), int(f.size())) );
			}
		      this->itsBestFitType = "guess";
		      // set the components to be at least as big as the beam
		      for (size_t i = 0; i < cmpntList.size(); i++) {
			casa::Gaussian2D<casa::Double> gauss=cmpntList[i].asGauss();
			if(cmpntList[i].maj()<this->itsHeader->beam().maj()){
			  cmpntList[i].setMajor(this->itsHeader->beam().maj());
			  cmpntList[i].setMinor(this->itsHeader->beam().min());			      
			  cmpntList[i].setPA(this->itsHeader->beam().pa()*M_PI/180.);
			}
			else cmpntList[i].setMinor(std::max(cmpntList[i].min(),double(this->itsHeader->beam().min())));
			// cmpntList[i].setFlux(gauss.flux());
		      }
		      FitResults guess;
		      guess.saveGuess(cmpntList);
		      this->itsBestFitMap["guess"] = guess;
		      this->itsBestFitMap["best"] = guess;
		      for (type = typelist.begin(); type < typelist.end(); type++) {
			  if (this->itsFitParams.hasType(*type)) this->itsBestFitMap[*type] = guess;
		      }
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
                /// @param imageName The name of the image from which to extract the spectral information
	        /// @param term Which Taylor term to do - either 1 or 2, other values trigger an exception
                /// @param doCalc If true, do all the calculations. If false, fill the alpha & beta values to 0. for all fit types.

	      std::string termtype[3]={"","spectral index","spectral curvature"};

	      ASKAPCHECK(term==1 || term==2, 
			 "Term number ("<<term<<") must be either 1 (for spectral index) or 2 (for spectral curvature)");


	      if (!doCalc) {
		// initialise arrays to zero and do nothing else

		std::vector<std::string>::iterator type;
		std::vector<std::string> typelist = availableFitTypes;
		typelist.push_back("best");

		for (type = typelist.begin(); type < typelist.end(); type++) {
		  if(term==1)     this->itsAlphaMap[*type] = std::vector<float>(this->itsBestFitMap[*type].numFits(), 0.);
		  else if(term==2) this->itsBetaMap[*type] = std::vector<float>(this->itsBestFitMap[*type].numFits(), 0.);
		      
		}

	      } 
	      else {

		ASKAPLOG_DEBUG_STR(logger, "About to find the "<<termtype[term]<<", for image " << imageName);

		// Get taylor1 values for box, and define positions
		Slice xrange=casa::Slice(this->boxXmin()+this->getXOffset(),this->boxXmax()-this->boxXmin()+1,1);
		Slice yrange=casa::Slice(this->boxYmin()+this->getYOffset(),this->boxYmax()-this->boxYmin()+1,1);
		Slicer theBox=casa::Slicer(xrange, yrange);
		// casa::Vector<casa::Double> flux_all = getPixelsInBox(imageName, theBox);
		casa::Array<casa::Float> flux_all = getPixelsInBox(imageName, theBox);
		ASKAPLOG_DEBUG_STR(logger, "Read flux array in a box with " << flux_all.size() << " pixels");

		std::vector<double> fluxvec;
		for (size_t i=0;i<flux_all.size();i++) {
		    // if(!isnan(flux_all(i))){
			// fluxvec.push_back(flux_all(i));
		    if(!isnan(flux_all.data()[i])){
			fluxvec.push_back(flux_all.data()[i]);
		    }
		}
		casa::Matrix<casa::Double> pos;
		casa::Vector<casa::Double> sigma;
		pos.resize(fluxvec.size(), 2);
		sigma.resize(fluxvec.size());
		casa::Vector<casa::Double> curpos(2);
		curpos = 0;

		// The following checks for pixels that have been blanked, and ignore them
		int counter=0;
		for (size_t i=0;i<flux_all.size();i++) {
		    // if(!isnan(flux_all(i))){
		    if(!isnan(flux_all.data()[i])){
			sigma(counter)=1;
			curpos(0) = i%this->boxXsize() + this->boxXmin();
			curpos(1) = i/this->boxXsize() + this->boxYmin();
			pos.row(counter) = curpos;
			counter++;
		    }
		}
		casa::Vector<casa::Double> f(fluxvec);
		ASKAPLOG_DEBUG_STR(logger, "About to use a flux array with " << f.size() << " pixels");

		// Set up fit with same parameters and do the fit
		std::vector<std::string>::iterator type;
		std::vector<std::string> typelist = availableFitTypes;

		for (type = typelist.begin(); type < typelist.end(); type++) {
		  std::vector<float> termValues(this->itsBestFitMap[*type].numGauss(), 0.);

		  if (this->itsBestFitMap[*type].isGood() || this->itsBestFitMap[*type].fitIsGuess()){
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

	    // /// @todo Make this a more obvious parameter to change
	    // const int fluxPrec = 8;
	    // const int fluxWidth = fluxPrec + 12;

	    duchamp::Catalogues::CatalogueSpecification newSpec;
	    newSpec.addColumn(inputSpec.column("NUM"));
	    newSpec.column("NUM").setName("ID");
	    newSpec.column("NUM").setUnits("--");
	    newSpec.column("NUM").setDatatype("char");
	    newSpec.addColumn(inputSpec.column("NAME"));
	    newSpec.column("NAME").setUnits("--");
	    newSpec.addColumn(inputSpec.column("RAJD"));
	    newSpec.addColumn(inputSpec.column("DECJD"));
	    // newSpec.addColumn(inputSpec.column("VEL"));
	    newSpec.addColumn(inputSpec.column("X"));
	    newSpec.column("X").setUnits("[pix]");
	    newSpec.addColumn(inputSpec.column("Y"));
	    newSpec.column("Y").setUnits("[pix]");
	    // newSpec.addColumn(inputSpec.column("Z"));
	    newSpec.addColumn(inputSpec.column("FINT"));
	    newSpec.addColumn(inputSpec.column("FPEAK"));
 
	    newSpec.column("FINT").setUCD("phot.flux.density.integrated");
	    // newSpec.column("FINT").changePrec(fluxPrec);
	    newSpec.column("FPEAK").setUCD("phot.flux.density.peak");
	    // newSpec.column("FPEAK").changePrec(fluxPrec);
	    // new columns
	    newSpec.addColumn( duchamp::Catalogues::Column("FINTFIT","F_int(fit)", "["+header.getIntFluxUnits()+"]",10,3,"phot.flux.density.integrated;stat.fit","float","col_fint_fit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("FPEAKFIT","F_pk(fit)", "["+header.getFluxUnits()+"]", 10, 3,"phot.flux.density.peak;stat.fit","float","col_fpeak_fit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("MAJFIT","Maj(fit)", "[arcsec]", 10, 3,"phys.angSize.smajAxis","float","col_maj_fit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("MINFIT","Min(fit)", "[arcsec]", 10, 3,"phys.angSize.sminAxis","float","col_min_fit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("PAFIT","P.A.(fit)", "[deg]", 10, 2,"phys.angSize;pos.posAng","float","col_pa_fit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("MAJDECONV","Maj(fit_deconv.)", "[arcsec]", 17, 3,"phys.angSize.smajAxis;meta.deconvolved","float","col_maj_deconv","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("MINDECONV","Min(fit_deconv.)", "[arcsec]", 17, 3,"phys.angSize.sminAxis;meta.deconvolved","float","col_min_deconv","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("PADECONV","P.A.(fit_deconv.)", "[deg]", 18, 2,"phys.angSize;pos.posAng;meta.deconvolved","float","col_pa_deconv","") ); 
	    newSpec.addColumn( duchamp::Catalogues::Column("ALPHA","Alpha", "--", 8, 3,"spect.index","float","col_alpha","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("BETA","Beta", "--", 8, 3,"spect.curvature","float","col_beta","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("CHISQFIT","Chisq(fit)", "--", 10,3,"stat.fit.chi2","float","col_chisqfit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("RMSIMAGE","RMS(image)", "["+header.getFluxUnits()+"]", 10,3,"stat.stdev;phot.flux.density","float","col_rmsimage","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("RMSFIT","RMS(fit)", "["+header.getFluxUnits()+"]", 10, 3,"stat.stdev;stat.fit","float","col_rmsfit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("NFREEFIT","Nfree(fit)", "--", 11, 0,"meta.number;stat.fit.param;stat.fit","int","col_nfreefit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("NDOFFIT","NDoF(fit)", "--", 10, 0,"stat.fit.dof","int","col_ndoffit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("NPIXFIT","NPix(fit)", "--", 10, 0,"meta.number;instr.pixel","int","col_npixfit","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("NPIXOBJ","NPix(obj)", "--", 10, 0,"meta.number;instr.pixel;stat.fit","int","col_npixobj","") );
	    newSpec.addColumn( duchamp::Catalogues::Column("GUESS","Guess?","--",7,0,"meta.flag","int","col_guess","") );
    
	    return newSpec;
	  }

	    void getResultsParams(casa::Gaussian2D<Double> &gauss, duchamp::FitsHeader *head, double zval, std::vector<Double> &deconvShape, double &ra, double &dec, double &intFluxFit)
	    {
		deconvShape = deconvolveGaussian(gauss,head->getBeam());
		double zworld;
		if(head->pixToWCS(gauss.xCenter(),gauss.yCenter(),zval,ra,dec,zworld)!=0)
		    ASKAPLOG_ERROR_STR(logger, "Error with pixToWCS conversion");
		intFluxFit = gauss.flux();
		if (head->needBeamSize())
		    intFluxFit /= head->beam().area(); // Convert from Jy/beam to Jy
	    }


	    void setupCols(duchamp::Catalogues::CatalogueSpecification &spec, std::vector<sourcefitting::RadioSource> &srclist, std::string fitType)
	    {

		std::vector<sourcefitting::RadioSource>::iterator src;
		for(src=srclist.begin();src!=srclist.end();src++){
		    FitResults results=src->fitResults(fitType);
		    for(unsigned int n=0;n<results.numFits();n++){
			casa::Gaussian2D<Double> gauss=results.gaussian(n);
			std::vector<Double> deconvShape;
			double ra,dec,intFluxFit;
			getResultsParams(gauss,src->header(), src->getZcentre(), deconvShape, ra, dec, intFluxFit);
			std::stringstream id;
			id << src->getID() << char('a'+n);
			spec.column("NUM").check(id.str());
			spec.column("NAME").check(src->getName());
			spec.column("RAJD").check(ra);
			spec.column("DECJD").check(dec);
			spec.column("X").check(gauss.xCenter());
			spec.column("Y").check(gauss.yCenter());
			spec.column("FINT").check(src->getIntegFlux());
			spec.column("FPEAK").check(src->getPeakFlux());
			spec.column("FINTFIT").check(intFluxFit);
			spec.column("FPEAKFIT").check(gauss.height());
			spec.column("MAJFIT").check(gauss.majorAxis()*src->header()->getAvPixScale()*3600.); // convert from pixels to arcsec
			spec.column("MINFIT").check(gauss.minorAxis()*src->header()->getAvPixScale()*3600.);
			spec.column("PAFIT").check(gauss.PA()*180. / M_PI,false);
			spec.column("MAJDECONV").check(deconvShape[0]*src->header()->getAvPixScale()*3600.); // convert from pixels to arcsec
			spec.column("MINDECONV").check(deconvShape[1]*src->header()->getAvPixScale()*3600.);
			spec.column("PADECONV").check(deconvShape[2]*180. / M_PI,false);
			spec.column("ALPHA").check(src->alphaValues(fitType)[n]);
			spec.column("BETA").check(src->betaValues(fitType)[n]);
			spec.column("CHISQFIT").check(results.chisq());
			spec.column("RMSIMAGE").check(src->noiseLevel());
			spec.column("RMSFIT").check(results.RMS());
			spec.column("NFREEFIT").check(results.numFreeParam());
			spec.column("NDOFFIT").check(results.ndof());
			spec.column("NPIXFIT").check(results.numPix());
			spec.column("NPIXOBJ").check(src->getSize());
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

	    std::string getSuffix(unsigned int num)
	    {
		/// @details Returns a string to uniquely identify a
		/// fit that is part of an island. The first 26
		/// numbers (zero-based), get a single letter
		/// a-z. After that, it becomes
		/// aa,ab,ac,...az,ba,bb,bc,...bz,ca,... If there are
		/// more than 702 (=26^2+26), we move to three
		/// characters: zy,zz,aaa,aab,aac,... And so on.

		char initialLetter='a';
		std::stringstream id;
		for(unsigned int c=0,count=0,factor=1; count <= num; factor*=26,c++,count+=factor){
		    int n = ((num-count)/factor)%26;
		    id << char(initialLetter+n);
		}
		std::string suff=id.str();
		std::reverse(suff.begin(),suff.end());
		    
		return suff;
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
	    std::stringstream id;
	    id << this->getID() << getSuffix(fitNum);
	    std::vector<Double> deconv = deconvolveGaussian(gauss,this->itsHeader->getBeam());
	    double thisRA,thisDec,zworld;
	    this->itsHeader->pixToWCS(gauss.xCenter(),gauss.yCenter(),this->getZcentre(),thisRA,thisDec,zworld);
	    float intfluxfit = gauss.flux();
	    if (this->itsHeader->needBeamSize())
	      intfluxfit /= this->itsHeader->beam().area(); // Convert from Jy/beam to Jy

	    std::string type=column.type();	    
	    if(type=="NUM")  column.printEntry(stream, id.str());
	    else if(type=="NAME")  column.printEntry(stream, this->getName());
	    else if(type=="RAJD")  column.printEntry(stream, thisRA);
	    else if(type=="DECJD")  column.printEntry(stream, thisDec);
	    else if(type=="X") column.printEntry(stream,gauss.xCenter());
	    else if(type=="Y") column.printEntry(stream,gauss.yCenter());
	    else if(type=="FINT")  column.printEntry(stream, this->getIntegFlux());
	    else if(type=="FPEAK")  column.printEntry(stream, this->getPeakFlux());
	    else if(type=="FINTFIT")  column.printEntry(stream, intfluxfit);
	    else if(type=="FPEAKFIT")  column.printEntry(stream, gauss.height());
	    else if(type=="MAJFIT")  column.printEntry(stream, gauss.majorAxis()*this->itsHeader->getAvPixScale()*3600.); // convert from pixels to arcsec
	    else if(type=="MINFIT")  column.printEntry(stream, gauss.minorAxis()*this->itsHeader->getAvPixScale()*3600.);
	    else if(type=="PAFIT")  column.printEntry(stream, gauss.PA()*180. / M_PI);
	    else if(type=="MAJDECONV")  column.printEntry(stream, deconv[0]*this->itsHeader->getAvPixScale()*3600.); // convert from pixels to arcsec
	    else if(type=="MINDECONV")  column.printEntry(stream, deconv[1]*this->itsHeader->getAvPixScale()*3600.);
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

	    void RadioSource::writeFitToAnnotationFile(duchamp::AnnotationWriter *writer, int sourceNum, bool doEllipse, bool doBox)
	  {
                /// @details
                ///
                /// This function writes the information about the
                /// fitted Gaussian components to a given annotation
                /// file. There are two different elements drawn for
                /// each RadioSource object - the half-maximum
                /// ellipse, and, optionally, the box given by
                /// FittingParameters::boxPadSize(). We use the
                /// Duchamp annotation file interface, to allow karma,
                /// DS9 and CASA formats to be written.
 
	      std::stringstream ss;
	      ss<< "# Source " << sourceNum << ":";
	      writer->writeCommentString(ss.str());
	      
	      double *pix = new double[12];
	      double *world = new double[12];

	      for (int i = 0; i < 4; i++) pix[i*3+2] = 0;

	      std::vector<casa::Gaussian2D<Double> > fitSet = this->itsBestFitMap["best"].fitSet();
	      std::vector<casa::Gaussian2D<Double> >::iterator fit;

	      if (doEllipse) {
		  for (fit = fitSet.begin(); fit < fitSet.end(); fit++) {
		      pix[0] = fit->xCenter();
		      pix[1] = fit->yCenter();
		      this->itsHeader->pixToWCS(pix, world);
		      
		      writer->ellipse(world[0],
				      world[1],
				      fit->majorAxis() * this->itsHeader->getAvPixScale() / 2.,
				      fit->minorAxis() * this->itsHeader->getAvPixScale() / 2.,
				      fit->PA() * 180. / M_PI);
		  }
	      }
		
	      if (doBox) {
		  pix[0] = pix[9] = this->getXmin() - this->itsFitParams.boxPadSize() - 0.5;
		  pix[1] = pix[4] = this->getYmin() - this->itsFitParams.boxPadSize() - 0.5;
		  pix[3] = pix[6] = this->getXmax() + this->itsFitParams.boxPadSize() + 0.5;
		  pix[7] = pix[10] = this->getYmax() + this->itsFitParams.boxPadSize() + 0.5;
		  this->itsHeader->pixToWCS(pix, world, 4);

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

	  void SortDetections(std::vector<RadioSource> &sourcelist, std::string parameter)
	    {
	      /// @details This function sorts a vector list of
	      /// RadioSource objects, using the same functionality as
	      /// the duchamp library's SortDetections function. The
	      /// objects are sorted as duchamp::Detection objects,
	      /// keeping track of their individual identities by
	      /// using the ID field. This will need to be reassigned
	      /// afterwards.

		size_t size=sourcelist.size();
		std::vector<duchamp::Detection> detlist(size);
		std::vector<RadioSource> newSourcelist(size);

		for(size_t i=0;i<size;i++){
		    sourcelist[i].setID(i);
		    detlist[i] = duchamp::Detection(sourcelist[i]);
		}

		duchamp::SortDetections(detlist,parameter);

		for(size_t i=0;i<size;i++){
		    newSourcelist[i] = sourcelist[detlist[i].getID()];
		}

		sourcelist = newSourcelist;
	      

	    }




        }

    }

}
