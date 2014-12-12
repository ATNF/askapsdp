/// @file
///
/// @brief Utility functions to support use of LOFAR Blobs in the analysis software
/// @details
/// These functions are not part of any classes, but provide
/// ways for existing objects to be passed over LOFAR Blobs.
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

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/RadioSource.h>
#include <sourcefitting/FittingParameters.h>
#include <sourcefitting/FitResults.h>

#include <string>
#include <vector>
#include <utility>

#include <duchamp/Detection/detection.hh>
#include <duchamp/PixelMap/Voxel.hh>

#include <Common/LofarTypedefs.h>
using namespace LOFAR::TYPES;
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Common/Exceptions.h>

#include <scimath/Functionals/Gaussian2D.h>
#include <casa/namespace.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slicer.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".parallelanalysis");

namespace askap {
    namespace analysis {

        namespace sourcefitting {

            LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream &blob, FittingParameters& par)
            {
                /// @brief This function provides a mechanism for passing the
                /// entire contents of a FittingParameters object into a
                /// LOFAR::BlobOStream stream
	        blob << par.itsFlagDoFit;
		blob << par.itsBoxPadSize;
                blob << par.itsMaxRMS;
                blob << par.itsMaxNumGauss;
                blob << par.itsChisqConfidence;
                blob << par.itsMaxReducedChisq;
                blob << par.itsNoiseBoxSize;
                blob << par.itsMinFitSize;
                blob << par.itsBoxFlux;
		blob << par.itsFlagFitJustDetection;
                blob << par.itsSrcPeak;
                blob << par.itsDetectThresh;
                blob << par.itsNumSubThresholds;
		blob << par.itsFlagLogarithmicIncrements;
		blob << par.itsFlagUseCurvature;
		blob << par.itsSigmaCurv;
		blob << par.itsCurvatureImage;
		blob << par.itsFlagNumGaussFromGuess;
                blob << par.itsBeamSize;
                blob << par.itsMaxRetries;
                blob << par.itsCriterium;
                blob << par.itsMaxIter;
                blob << par.itsUseNoise;
		blob << par.itsNoiseLevel;
		blob << par.itsNegativeFluxPossible;
		blob << par.itsStopAfterFirstGoodFit;
		blob << par.itsUseGuessIfBad;
                blob << par.itsXmin;
                blob << par.itsXmax;
                blob << par.itsYmin;
                blob << par.itsYmax;
                uint32 size = par.itsFlagFitThisParam.size();
                blob << size;

                for (uint32 i = 0; i < size; i++)
                    blob << par.itsFlagFitThisParam[i];

                size = par.itsFitTypes.size();
                blob << size;

                for (uint32 i = 0; i < size; i++)
                    blob << par.itsFitTypes[i];

                return blob;
            }

            LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream &blob, FittingParameters& par)
            {
                /// @brief This function provides a mechanism for receiving the
                /// entire contents of a FittingParameters object from a
                /// LOFAR::BlobIStream stream
	        blob >> par.itsFlagDoFit;
                blob >> par.itsBoxPadSize;
                blob >> par.itsMaxRMS;
                blob >> par.itsMaxNumGauss;
                blob >> par.itsChisqConfidence;
                blob >> par.itsMaxReducedChisq;
                blob >> par.itsNoiseBoxSize;
                blob >> par.itsMinFitSize;
                blob >> par.itsBoxFlux;
		blob >> par.itsFlagFitJustDetection;
                blob >> par.itsSrcPeak;
                blob >> par.itsDetectThresh;
                blob >> par.itsNumSubThresholds;
		blob >> par.itsFlagLogarithmicIncrements;
		blob >> par.itsFlagUseCurvature;
		blob >> par.itsSigmaCurv;
		blob >> par.itsCurvatureImage;
		blob >> par.itsFlagNumGaussFromGuess;
                blob >> par.itsBeamSize;
                blob >> par.itsMaxRetries;
                blob >> par.itsCriterium;
                blob >> par.itsMaxIter;
                blob >> par.itsUseNoise;
		blob >> par.itsNoiseLevel;
		blob >> par.itsNegativeFluxPossible;
		blob >> par.itsStopAfterFirstGoodFit;
		blob >> par.itsUseGuessIfBad;
                blob >> par.itsXmin;
                blob >> par.itsXmax;
                blob >> par.itsYmin;
                blob >> par.itsYmax;
                int32 size;
                bool flag;
                blob >> size;
                par.itsFlagFitThisParam = std::vector<bool>(size);

                for (int i = 0; i < size; i++) {
                    blob >> flag;
                    par.itsFlagFitThisParam[i] = flag;
                }

                blob >> size;
                par.itsFitTypes = std::vector<std::string>(size);
                std::string type;

                for (int i = 0; i < size; i++) {
                    blob >> type;
                    par.itsFitTypes[i] = type;
                }

                return blob;
            }


            LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream &blob, FitResults& result)
            {
                /// @brief This function provides a mechanism for passing the
                /// entire contents of a FitResults object into a
                /// LOFAR::BlobOStream stream
                blob << result.itsFitIsGood;
                blob << result.itsChisq;
                blob << result.itsRedChisq;
                blob << result.itsRMS;
                blob << result.itsNumDegOfFreedom;
                blob << result.itsNumFreeParam;
                blob << result.itsNumPix;
                blob << result.itsNumGauss;
		blob << result.itsFlagFitIsGuess;
                uint32 i = result.itsGaussFitSet.size(); blob << i;
                std::vector<casa::Gaussian2D<Double> >::iterator fit = result.itsGaussFitSet.begin();

                for (; fit < result.itsGaussFitSet.end(); fit++) {
                    blob << fit->height();
                    blob << fit->xCenter();
                    blob << fit->yCenter();
                    blob << fit->majorAxis();
                    blob << fit->axialRatio();
                    blob << fit->PA();
                }

                return blob;
            }


            LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream &blob, FitResults& result)
            {
                /// @brief This function provides a mechanism for receiving the
                /// entire contents of a FitResults object from a
                /// LOFAR::BlobIStream stream
                blob >> result.itsFitIsGood;
                blob >> result.itsChisq;
                blob >> result.itsRedChisq;
                blob >> result.itsRMS;
                blob >> result.itsNumDegOfFreedom;
                blob >> result.itsNumFreeParam;
                blob >> result.itsNumPix;
                blob >> result.itsNumGauss;
 		blob >> result.itsFlagFitIsGuess;
               uint32 i, size;
                blob >> size;
                result.itsGaussFitSet.clear();

                for (i = 0; i < size; i++) {
                    Double d1, d2, d3, d4, d5, d6;
                    blob >> d1;
                    blob >> d2;
                    blob >> d3;
                    blob >> d4;
                    blob >> d5;
                    blob >> d6;
                    casa::Gaussian2D<Double> fit(d1, d2, d3, d4, d5, d6);
                    result.itsGaussFitSet.push_back(fit);
                }

                return blob;
            }




            LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream& blob, RadioSource& src)
            {
                /// @brief This function provides a mechanism for passing the
                /// entire contents of a RadioSource object into a
                /// LOFAR::BlobOStream stream
                int32 l;
                int i;
                float f;
		double d;
                std::string s;
                bool b;
                int size = src.getSize();
                blob << size;
                std::vector<PixelInfo::Voxel> pixelSet = src.getPixelSet();

                for (i = 0; i < size; i++) {
                    l = pixelSet[i].getX(); blob << l;
                    l = pixelSet[i].getY(); blob << l;
                    l = pixelSet[i].getZ(); blob << l;
                }

                l = src.xSubOffset; blob << l;
                l = src.ySubOffset; blob << l;
                l = src.zSubOffset; blob << l;
                b = src.haveParams; blob << b;
                f = src.totalFlux;  blob << f;
                f = src.intFlux;    blob << f;
                f = src.peakFlux;   blob << f;
                l = src.xpeak;      blob << l;
                l = src.ypeak;      blob << l;
                l = src.zpeak;      blob << l;
                f = src.peakSNR;    blob << f;
                f = src.xCentroid;  blob << f;
                f = src.yCentroid;  blob << f;
                f = src.zCentroid;  blob << f;
                s = src.centreType; blob << s;
                b = src.negSource;  blob << b;
                s = src.flagText;   blob << s;
                i = src.id;         blob << i;
                s = src.name;       blob << s;
                b = src.flagWCS;    blob << b;
                s = src.raS;        blob << s;
                s = src.decS;       blob << s;
                d = src.ra;         blob << d;
                d = src.dec;        blob << d;
                d = src.raWidth;    blob << d;
                d = src.decWidth;   blob << d;
                d = src.majorAxis;  blob << d;
                d = src.minorAxis;  blob << d;
                d = src.posang;     blob << d;
		b = src.specOK;     blob << b;
		s = src.specUnits;  blob << s;
                s = src.specType;   blob << s;
                s = src.fluxUnits;  blob << s;
                s = src.intFluxUnits; blob << s;
                s = src.lngtype;    blob << s;
                s = src.lattype;    blob << s;
                d = src.vel;        blob << d;
                d = src.velWidth;   blob << d;
                d = src.velMin;     blob << d;
                d = src.velMax;     blob << d;
		d = src.v20min;     blob << d;
                d = src.v20max;     blob << d;
		d = src.w20;        blob << d;
                d = src.v50min;     blob << d;
                d = src.v50max;     blob << d;
                d = src.w50;        blob << d;
                i = src.posPrec;    blob << i;
                i = src.xyzPrec;    blob << i;
                i = src.fintPrec;   blob << i;
                i = src.fpeakPrec;  blob << i;
                i = src.velPrec;    blob << i;
                i = src.snrPrec;    blob << i;
                b = src.hasFit;     blob << b;
                b = src.atEdge;     blob << b;
                f = src.itsDetectionThreshold; blob << f;
                f = src.itsNoiseLevel; blob << f;
                blob << src.itsFitParams;
                size = src.itsBestFitMap.size();
                blob << size;
                std::map<std::string, FitResults>::iterator fit;

                for (fit = src.itsBestFitMap.begin(); fit != src.itsBestFitMap.end(); fit++) {
                    blob << fit->first;
                    blob << fit->second;
                }

                size = src.itsAlphaMap.size();
                blob << size;
                std::map<std::string, std::vector<float> >::iterator val;

                for (val = src.itsAlphaMap.begin(); val != src.itsAlphaMap.end(); val++) {
                    blob << val->first;
                    size = val->second.size();
                    blob << size;

                    for (int i = 0; i < size; i++) blob << val->second[i];
                }

                size = src.itsBetaMap.size();
                blob << size;

                for (val = src.itsBetaMap.begin(); val != src.itsBetaMap.end(); val++) {
                    blob << val->first;
                    size = val->second.size();
                    blob << size;

                    for (int i = 0; i < size; i++) blob << val->second[i];
                }

		i = src.box().ndim(); blob << i;
		i = src.box().start()[0]; blob << i;
		i = src.box().start()[1]; blob << i;
		if(src.box().ndim()>2){
		    i = src.box().start()[2]; blob << i;
		}
		i = src.box().end()[0]; blob << i;
		i = src.box().end()[1]; blob << i;
		if(src.box().ndim()>2){
		    i = src.box().end()[2]; blob << i;
		}

		//		ASKAPLOG_DEBUG_STR(logger, "Sending box: start="<<src.box().start()<<", end="<<src.box().end());

                return blob;
            }

            LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream &blob, RadioSource& src)
            {
                /// @brief Receive a RadioSource object from a Blob
                /// @details This function provides a mechanism for receiving the
                /// entire contents of a RadioSource object from a
                /// LOFAR::BlobIStream stream
                int i;
                int32 l;
                bool b;
                float f;
		double d;
                std::string s;
                int32 size;
                blob >> size;

                for (i = 0; i < size; i++) {
                    int32 x, y, z;
                    blob >> x;
                    blob >> y;
                    blob >> z;
                    src.addPixel(x, y, z);
                }

                blob >> l; src.xSubOffset = l;
                blob >> l; src.ySubOffset = l;
                blob >> l; src.zSubOffset = l;
                blob >> b; src.haveParams = b;
                blob >> f; src.totalFlux = f;
                blob >> f; src.intFlux = f;
                blob >> f; src.peakFlux = f;
                blob >> l; src.xpeak = l;
                blob >> l; src.ypeak = l;
                blob >> l; src.zpeak = l;
                blob >> f; src.peakSNR = f;
                blob >> f; src.xCentroid = f;
                blob >> f; src.yCentroid = f;
                blob >> f; src.zCentroid = f;
                blob >> s; src.centreType = s;
                blob >> b; src.negSource = b;
                blob >> s; src.flagText = s;
                blob >> i; src.id = i;
                blob >> s; src.name = s;
                blob >> b; src.flagWCS = b;
                blob >> s; src.raS = s;
                blob >> s; src.decS = s;
                blob >> d; src.ra = d;
                blob >> d; src.dec = d;
                blob >> d; src.raWidth = d;
                blob >> d; src.decWidth = d;
                blob >> d; src.majorAxis = d;
                blob >> d; src.minorAxis = d;
                blob >> d; src.posang = d;
		blob >> b; src.specOK = b;
		blob >> s; src.specUnits = s;
                blob >> s; src.specType = s;
                blob >> s; src.fluxUnits = s;
                blob >> s; src.intFluxUnits = s;
                blob >> s; src.lngtype = s;
                blob >> s; src.lattype = s;
                blob >> d; src.vel = d;
                blob >> d; src.velWidth = d;
                blob >> d; src.velMin = d;
                blob >> d; src.velMax = d;
                blob >> d; src.v20min = d;
                blob >> d; src.v20max = d;
                blob >> d; src.w20 = d;
                blob >> d; src.v50min = d;
                blob >> d; src.v50max = d;
                blob >> d; src.w50 = d;
                blob >> i; src.posPrec = i;
                blob >> i; src.xyzPrec = i;
                blob >> i; src.fintPrec = i;
                blob >> i; src.fpeakPrec = i;
                blob >> i; src.velPrec = i;
                blob >> i; src.snrPrec = i;
                blob >> b; src.hasFit = b;
                blob >> b; src.atEdge = b;
                blob >> f; src.itsDetectionThreshold = f;
                blob >> f; src.itsNoiseLevel = f;
                blob >> src.itsFitParams;
                blob >> size;

                for (int i = 0; i < size; i++) {
                    FitResults res;
                    blob >> s >> res;
                    src.itsBestFitMap[s] = res;
                }

                blob >> size;

                for (int i = 0; i < size; i++) {
                    int32 vecsize;
                    blob >> s >> vecsize;
                    std::vector<float> vec(vecsize);

                    for (int i = 0; i < vecsize; i++) blob >> vec[i];

                    src.itsAlphaMap[s] = vec;
                }

                blob >> size;

                for (int i = 0; i < size; i++) {
                    int32 vecsize;
                    blob >> s >> vecsize;
                    std::vector<float> vec(vecsize);

                    for (int i = 0; i < vecsize; i++) blob >> vec[i];

                    src.itsBetaMap[s] = vec;
                }

		int ndim,x1,y1,z1,x2,y2,z2;
		blob >> ndim >> x1 >> y1;
		if(ndim>2) blob >> z1;
		blob >> x2 >> y2;
		if(ndim>2) blob >> z2;
		casa::IPosition start(ndim),end(ndim),stride(ndim,1);
		start(0)=x1; start(1)=y1; 
		end(0)=x2; end(1)=y2; 
		if(ndim>2){
		    start(2)=z1;
		    end(2)=z2;
		}
		ASKAPCHECK(end>=start, "Slicer in blob transfer of RadioSource - start "<< start << " > end " << end);
		Slicer box(start, end, stride, Slicer::endIsLast);;
		src.setBox(box);
		
		//		ASKAPLOG_DEBUG_STR(logger, "Received box: start="<<start<<", end="<<end);

                return blob;
            }

        }

    }

}
