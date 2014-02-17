/// @file linmos.cc
///
/// @brief combine a number of images as a linear mosaic
/// @details This is a standalone utility to merge images into
///     a mosaic. Some code/functionality can later be moved into cimager,
///     but for now it is handy to have it separate. 
///
/// @copyright (c) 2012 CSIRO
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>
/// @author Daniel Mitchell <daniel.mitchell@csiro.au> (2014)

// Package level header file
#include "askap_synthesis.h"

// System includes
#include <sstream>
#include <typeinfo>

// other 3rd party
#include <Common/ParameterSet.h>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <casa/Arrays/Array.h>
#include <images/Images/ImageRegrid.h>

// ASKAPsoft includes
#include <askap/Application.h>
#include <askap/AskapError.h>
#include <askap/AskapLogging.h>
#include <askap/AskapUtil.h>
#include <fitting/Params.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <utils/MultiDimArrayPlaneIter.h>
#include <scimath/Mathematics/Interpolate2D.h>

ASKAP_LOGGER(logger, ".linmos");

using namespace casa;
using namespace askap;
using namespace askap::synthesis;

enum weight_types {FROM_WEIGHT_IMAGES=0, FROM_BP_MODEL};
// FROM_WEIGHT_IMAGES   Obtain pixel weights from weight images (parset "weights" entries)
// FROM_BP_MODEL        Generate pixel weights using a Gaussian primary-beam model
enum weight_states {CORRECTED=0, INHERENT, WEIGHTED};
// CORRECTED            Direction-dependent beams/weights have been divided out of input images
// INHERENT             Input images retain the natural primary-beam weighting of the visibilities
// WEIGHTED             Input images have full primary-beam-squared weighting

class LinmosAccumulator {

  public:

    LinmosAccumulator();

    /// @brief check parset parameters and set any dependent options
    /// @param[in] const string& weightTypeName: value given for parset key 'weighttype'
    /// @param[in] const string& weightStateName: value given for parset key 'weightstate'
    /// @return bool true=success, false=fail
    bool checkParset(const string& weightTypeName, const string& weightStateName);

    /// @brief test whether the output buffers are empty and need initialising
    /// @return bool
    bool outputBufferSetupRequired(void);

    /// @brief set the input coordinate system and shape
    /// @param[in] const string& inImgName: name of the input image
    /// @param[in] const accessors::IImageAccess& iac
    void setInputParameters(const string& inImgName, const accessors::IImageAccess& iacc);

    /// @brief set the output coordinate system and shape, based on the overlap of input images
    /// @details This method is based on the SynthesisParamsHelper::add and
    ///     SynthesisParamsHelper::facetSlicer. It has been reimplemented here
    ///     so that images can be read into memory separately.
    /// @param[in] const vector<string>& inImgNames: names of the input images (those given for parset key 'names')
    /// @param[in] const accessors::IImageAccess& iac
    void setOutputParameters(const vector<string>& inImgNames, const accessors::IImageAccess& iacc);

    /// @brief set up any 2D temporary output image buffers required for regridding
    void initialiseOutputBuffers(void);

    /// @brief set up any 2D temporary input image buffers required for regridding
    void initialiseInputBuffers(void);

    /// @brief set up regridder
    /// @param[in] const String& method: ImageRegrid::regrid input option
    void initialiseRegridder(const String& method="linear");

    /// @brief load the temporary image buffers with the current plane of the current input image
    /// @param[in] const scimath::MultiDimArrayPlaneIter& planeIter: current plane id
    /// @param[in] Array<float>& inPix: image buffer
    /// @param[in] Array<float>& inWgtPix: weight image buffer
    void loadInputBuffers(const scimath::MultiDimArrayPlaneIter& planeIter,
                          Array<float>& inPix, Array<float>& inWgtPix);

    /// @brief call the regridder for the buffered plane
    /// @param[in] const Int decimate: ImageRegrid::regrid input option
    /// @param[in] const Bool replicate: ImageRegrid::regrid input option
    /// @param[in] const Bool force: ImageRegrid::regrid input option
    void regrid(const Int decimate=3, const Bool replicate=False, const Bool force=False);

    /// @brief add the current plane to the accumulation arrays
    /// @details This method adds from the regridded buffers
    /// @param[out] Array<float>& outPix: accumulated weighted image pixels
    /// @param[out] Array<float>& outWgtPix: accumulated weight pixels
    /// @param[in] const IPosition& curpos: indices of the current plane
    void accumulatePlane(Array<float>& outPix, Array<float>& outWgtPix, const IPosition& curpos);

    /// @brief add the current plane to the accumulation arrays
    /// @details This method adds directly from the input arrays
    /// @param[out] Array<float>& outPix: accumulated weighted image pixels
    /// @param[out] Array<float>& outWgtPix: accumulated weight pixels
    /// @param[in] const Array<float>& inPix: input image pixels
    /// @param[in] const Array<float>& inWgtPix: input weight pixels
    /// @param[in] const IPosition& curpos: indices of the current plane
    void accumulatePlane(Array<float>& outPix, Array<float>& outWgtPix,
                         const Array<float>& inPix, const Array<float>& inWgtPix, const IPosition& curpos);

    /// @brief divide the weighted pixels by the weights for the current plane
    /// @param[in,out] Array<float>& outPix: accumulated deweighted image pixels
    /// @param[in] const Array<float>& outWgtPix: accumulated weight pixels
    /// @param[in] const IPosition& curpos: indices of the current plane
    /// @param[in] const float cutoff: threshold to stop division by small numbers
    void deweightPlane(Array<float>& outPix, const Array<float>& outWgtPix, const IPosition& curpos,
                       const float cutoff=1e-6);

    /// @brief check to see if the input and output coordinate grids are equal
    /// @return bool: true if they are equal
    bool CoordinatesAreEqual(void);

    // return metadata for the current input image
    IPosition inShape(void) {return itsInShape;}
    CoordinateSystem inCoordSys(void) {return itsInCoordSys;}

    // return metadata for the output image
    IPosition outShape(void) {return itsOutShape;}
    CoordinateSystem outCoordSys(void) {return itsOutCoordSys;}

  private:

    /// @brief convert the current input shape and coordinate system to the reference (output) system
    /// @param[in] const DirectionCoordinate& refDC: reference direction coordinate
    /// @return IPosition vector containing BLC and TRC of the current input image, relative to another coord. system
    Vector<IPosition> convertImageCornersToRef(const DirectionCoordinate& refDC);

    /// @brief check to see if the input coordinate system is consistent enough with the reference system to merge
    /// @param[in] const CoordinateSystem& refCoordSys: reference coordinate system
    /// @return bool: true if they are consistent
    bool CoordinatesAreConsistent(const CoordinateSystem& refCoordSys);

    // regridding options
    ImageRegrid<float> itsRegridder;
    IPosition itsAxes;
    Interpolate2D::Method itsEmethod;
    // regridding buffers
    TempImage<float> itsInBuffer, itsInWgtBuffer;
    TempImage<float> itsOutBuffer, itsOutWgtBuffer;
    // metadata objects
    IPosition itsInShape;
    CoordinateSystem itsInCoordSys;
    IPosition itsOutShape;
    CoordinateSystem itsOutCoordSys;
    // options
    int itsWeightType;
    int itsWeightState;

};

LinmosAccumulator::LinmosAccumulator() : itsWeightType(-1), itsWeightState(-1) {}

bool LinmosAccumulator::checkParset(const string& weightTypeName, const string& weightStateName) {

    // Check weighting options. One of the following must be set:
    //  - get weights from input weight images
    //    * the number of weight images and their shapes must match the input images

    if (boost::iequals(weightTypeName, "FromWeightImages")) {
        itsWeightType = FROM_WEIGHT_IMAGES;
    } else if (boost::iequals(weightTypeName, "FromPrimaryBeamModel")) {
        itsWeightType = FROM_BP_MODEL;
        ASKAPLOG_ERROR_STR(logger, "weighttype '" << weightTypeName << "' not yet supported");
        return false;
    } else {
        ASKAPLOG_ERROR_STR(logger, "Unknown weighttype " << weightTypeName);
        return false;
    }

    if (boost::iequals(weightStateName, "Corrected")) {
        itsWeightState = CORRECTED;
    } else if (boost::iequals(weightStateName, "Inherent")) {
        itsWeightState = INHERENT;
    } else if (boost::iequals(weightStateName, "Weighted")) {
        itsWeightState = WEIGHTED;
    } else {
        ASKAPLOG_ERROR_STR(logger, "Unknown weightstyle " << weightStateName);
        return false;
    }

    return true;

}

bool LinmosAccumulator::outputBufferSetupRequired(void) {
    return ( itsOutBuffer.shape().nelements() == 0 );
}

void LinmosAccumulator::setInputParameters(const string& inImgName, const accessors::IImageAccess& iacc) {
    // set the input coordinate system and shape
    itsInCoordSys = iacc.coordSys(inImgName);
    itsInShape = iacc.shape(inImgName);
}

void LinmosAccumulator::setOutputParameters(const vector<string>& inImgNames, const accessors::IImageAccess& iacc) {

    // test that there are some input image names ...

    const IPosition refShape = iacc.shape(inImgNames[0]);
    ASKAPDEBUGASSERT(refShape.nelements() >= 2);
    const CoordinateSystem refCS = iacc.coordSys(inImgNames[0]);
    const int dcPos = refCS.findCoordinate(Coordinate::DIRECTION,-1);
    const DirectionCoordinate refDC = refCS.directionCoordinate(dcPos);
    IPosition refBLC(refShape.nelements(),0);
    IPosition refTRC(refShape);       
    for (uInt dim=0; dim<refShape.nelements(); ++dim) {
        --refTRC(dim); // these are added back later. Is this just to deal with degenerate axes?
    }
    ASKAPDEBUGASSERT(refBLC.nelements() >= 2);
    ASKAPDEBUGASSERT(refTRC.nelements() >= 2);
 
    IPosition tempBLC = refBLC;
    IPosition tempTRC = refTRC;

    // Loop over input images, converting their image bounds to the ref system 
    // and expanding the new overlapping image bounds where appropriate.
    for (uint img = 1; img < inImgNames.size(); ++img ) {

        // short cuts
        const string inImgName = inImgNames[img];

        // 
        itsInShape = iacc.shape(inImgName);
        itsInCoordSys = iacc.coordSys(inImgName);

        // test to see if the loaded coordinate system is close enough to the reference system for merging
        ASKAPCHECK(CoordinatesAreConsistent(refCS), "Input images have inconsistent coordinate systems");
        // could also test whether they are equal and set a regrid tag to false if all of them are

        Vector<IPosition> corners = convertImageCornersToRef(refDC);

        const IPosition newBLC = corners[0];
        const IPosition newTRC = corners[1];
        ASKAPDEBUGASSERT(newBLC.nelements() >= 2);
        ASKAPDEBUGASSERT(newTRC.nelements() >= 2);
        for (casa::uInt dim=0; dim<2; ++dim) {
            if (newBLC(dim) < tempBLC(dim)) {
                tempBLC(dim) = newBLC(dim);
            }
            if (newTRC(dim) > tempTRC(dim)) {
                tempTRC(dim) = newTRC(dim);
            }
        }
 
    }

    itsOutShape = refShape;
    itsOutShape(0) = tempTRC(0) - tempBLC(0) + 1;
    itsOutShape(1) = tempTRC(1) - tempBLC(1) + 1;
    ASKAPDEBUGASSERT(itsOutShape(0) > 0);
    ASKAPDEBUGASSERT(itsOutShape(1) > 0);       
    Vector<Double> refPix = refDC.referencePixel();
    refPix[0] -= Double(tempBLC(0) - refBLC(0));
    refPix[1] -= Double(tempBLC(1) - refBLC(1));
    DirectionCoordinate newDC(refDC);
    newDC.setReferencePixel(refPix);

    // set up a coord system for the merged images
    itsOutCoordSys = refCS;
    itsOutCoordSys.replaceCoordinate(newDC, dcPos);

}

void LinmosAccumulator::initialiseOutputBuffers(void) {
    // set up temporary images needed for regridding (which is done on a plane-by-plane basis so ignore other dims)

// DAM: do we need to test that the direction axes are 0 and 1 for the iterator?

    // set up the coord. sys.
    int dcPos = itsOutCoordSys.findCoordinate(Coordinate::DIRECTION,-1);
    ASKAPCHECK(dcPos>=0, "Cannot find the directionCoordinate");
    const DirectionCoordinate dcTmp = itsOutCoordSys.directionCoordinate(dcPos);
    CoordinateSystem cSysTmp;
    cSysTmp.addCoordinate(dcTmp);

    // set up the shape
    Vector<Int> shapePos = itsOutCoordSys.pixelAxes(dcPos);
    // check that the length is equal to 2 and the both elements are >= 0

    IPosition shape = IPosition(2,itsOutShape(shapePos[0]),itsOutShape(shapePos[1]));

    // apparently the +100 forces it to use the memory
    double maxMemoryInMB = double(shape.product()*sizeof(float))/1024./1024.+100;
    itsOutBuffer = TempImage<float>(shape, cSysTmp, maxMemoryInMB);
    itsOutWgtBuffer = TempImage<float>(shape, cSysTmp, maxMemoryInMB);

}

void LinmosAccumulator::initialiseInputBuffers() {
    // set up temporary images needed for regridding (which is done on a plane-by-plane basis so ignore other dims)

    // set up a coord. sys. the planes
    int dcPos = itsInCoordSys.findCoordinate(Coordinate::DIRECTION,-1);
    ASKAPCHECK(dcPos>=0, "Cannot find the directionCoordinate");
    const DirectionCoordinate dc = itsInCoordSys.directionCoordinate(dcPos);
    CoordinateSystem cSys;
    cSys.addCoordinate(dc);

    // set up the shape
    Vector<Int> shapePos = itsInCoordSys.pixelAxes(dcPos);
    // check that the length is equal to 2 and the both elements are >= 0

    IPosition shape = IPosition(2,itsInShape(shapePos[0]),itsInShape(shapePos[1]));

    double maxMemoryInMB = double(shape.product()*sizeof(float))/1024./1024.+100;
    itsInBuffer = TempImage<float>(shape,cSys,maxMemoryInMB);
    itsInWgtBuffer = TempImage<float>(shape,cSys,maxMemoryInMB);       

}

void LinmosAccumulator::initialiseRegridder(const String& method) {

    // die if itsOutBuffer isn't set

    itsAxes = IPosition::makeAxisPath(itsOutBuffer.shape().nelements());
    itsEmethod = Interpolate2D::stringToMethod(method);

}

void LinmosAccumulator::loadInputBuffers(const scimath::MultiDimArrayPlaneIter& planeIter,
                                         Array<float>& inPix, Array<float>& inWgtPix) {
    itsInBuffer.put(planeIter.getPlane(inPix));
    itsInWgtBuffer.put(planeIter.getPlane(inWgtPix));
}

void LinmosAccumulator::regrid(const Int decimate, const Bool replicate, const Bool force) {
    // 
    itsRegridder.regrid(itsOutBuffer, itsEmethod, itsAxes, itsInBuffer, replicate, decimate, False, force);
    itsRegridder.regrid(itsOutWgtBuffer, itsEmethod, itsAxes, itsInWgtBuffer, replicate, decimate, False, force);
}

void LinmosAccumulator::accumulatePlane(Array<float>& outPix, Array<float>& outWgtPix, const IPosition& curpos) {

    // copy the pixel iterator containing all dimensions
    IPosition fullpos(curpos);

    // set a pixel iterator that does not have the higher dimensions
    IPosition planepos(2);

    // Accumulate the pixels of this slice.
    // Could restrict it (and the regrid) to a smaller region of interest.
    if (itsWeightState == CORRECTED) {
        for (int x=0; x<outPix.shape()[0];++x) {
            for (int y=0; y<outPix.shape()[1];++y) {
                fullpos[0] = x;
                fullpos[1] = y;
                planepos[0] = x;
                planepos[1] = y;
                //the restore seems to be unweighting the images. Need to know this...
                outPix(fullpos) = outPix(fullpos) + itsOutBuffer.getAt(planepos) * itsOutWgtBuffer.getAt(planepos);
                outWgtPix(fullpos) = outWgtPix(fullpos) + itsOutWgtBuffer.getAt(planepos);
            }
        }
    } else if (itsWeightState == INHERENT) {
        for (int x=0; x<outPix.shape()[0];++x) {
            for (int y=0; y<outPix.shape()[1];++y) {
                fullpos[0] = x;
                fullpos[1] = y;
                planepos[0] = x;
                planepos[1] = y;
                //the restore seems to be unweighting the images. Need to know this...
                outPix(fullpos) = outPix(fullpos) + itsOutBuffer.getAt(planepos) * sqrt(itsOutWgtBuffer.getAt(planepos));
                outWgtPix(fullpos) = outWgtPix(fullpos) + itsOutWgtBuffer.getAt(planepos);
            }
        }
    } else if (itsWeightState == WEIGHTED) {
        for (int x=0; x<outPix.shape()[0];++x) {
            for (int y=0; y<outPix.shape()[1];++y) {
                fullpos[0] = x;
                fullpos[1] = y;
                planepos[0] = x;
                planepos[1] = y;
                //the restore seems to be unweighting the images. Need to know this...
                outPix(fullpos) = outPix(fullpos) + itsOutBuffer.getAt(planepos);
                outWgtPix(fullpos) = outWgtPix(fullpos) + itsOutWgtBuffer.getAt(planepos);
            }
        }
    }

}

void LinmosAccumulator::accumulatePlane(Array<float>& outPix, Array<float>& outWgtPix,
                                        const Array<float>& inPix, const Array<float>& inWgtPix,
                                        const IPosition& curpos) {

    ASKAPASSERT(inPix.shape() == outPix.shape());

    // copy the pixel iterator containing all dimensions
    IPosition fullpos(curpos);

    // Update the accululation arrays for this plane.
    if (itsWeightState == CORRECTED) {
        for (int x=0; x<outPix.shape()[0];++x) {
            for (int y=0; y<outPix.shape()[1];++y) {
                fullpos[0] = x;
                fullpos[1] = y;
                outPix(fullpos) = outPix(fullpos) + inPix(fullpos) * inWgtPix(fullpos);
                outWgtPix(fullpos) = outWgtPix(fullpos) + inWgtPix(fullpos);
            }
        }
    } else if (itsWeightState == INHERENT) {
        for (int x=0; x<outPix.shape()[0];++x) {
            for (int y=0; y<outPix.shape()[1];++y) {
                fullpos[0] = x;
                fullpos[1] = y;
                outPix(fullpos) = outPix(fullpos) + inPix(fullpos) * sqrt(inWgtPix(fullpos));
                outWgtPix(fullpos) = outWgtPix(fullpos) + inWgtPix(fullpos);
            }
        }
    } else if (itsWeightState == WEIGHTED) {
        for (int x=0; x<outPix.shape()[0];++x) {
            for (int y=0; y<outPix.shape()[1];++y) {
                fullpos[0] = x;
                fullpos[1] = y;
                outPix(fullpos) = outPix(fullpos) + inPix(fullpos);
                outWgtPix(fullpos) = outWgtPix(fullpos) + inWgtPix(fullpos);
            }
        }
    }

}

void LinmosAccumulator::deweightPlane(Array<float>& outPix, const Array<float>& outWgtPix, const IPosition& curpos,
                                      const float cutoff) {

    // copy the pixel iterator containing all dimensions
    IPosition fullpos(curpos);

    for (int x=0; x<outPix.shape()[0];++x) {
        for (int y=0; y<outPix.shape()[1];++y) {
            fullpos[0] = x;
            fullpos[1] = y;
            if (sqrt(outWgtPix(fullpos))<cutoff) {
                outPix(fullpos) = 0.0;
            } else {
                outPix(fullpos) = outPix(fullpos) / outWgtPix(fullpos);
            }
        }
    }

}

Vector<IPosition> LinmosAccumulator::convertImageCornersToRef(const DirectionCoordinate& refDC) {
    // based on SynthesisParamsHelper::facetSlicer, but don't want to load every input image into a scimath::Param

    ASKAPDEBUGASSERT(itsInShape.nelements() >= 2);
    // add more checks

    const int coordPos = itsInCoordSys.findCoordinate(Coordinate::DIRECTION,-1);
    const DirectionCoordinate inDC = itsInCoordSys.directionCoordinate(coordPos);

    IPosition blc(itsInShape.nelements(),0);
    IPosition trc(itsInShape);
    for (uInt dim=0; dim<itsInShape.nelements(); ++dim) {
         --trc(dim); // these are added back later. Is this just to deal with degenerate axes?
    }
    // currently blc,trc describe the whole input image; convert coordinates
    Vector<Double> pix(2);
    
    // first process BLC
    pix[0] = Double(blc[0]);
    pix[1] = Double(blc[1]);
    MDirection tempDir;
    Bool success = inDC.toWorld(tempDir, pix);
    ASKAPCHECK(success, "Pixel to world coordinate conversion failed for input BLC: "<<inDC.errorMessage());
    success = refDC.toPixel(pix,tempDir);
    ASKAPCHECK(success, "World to pixel coordinate conversion failed for output BLC: "<<refDC.errorMessage());
    blc[0] = casa::Int(round(pix[0]));
    blc[1] = casa::Int(round(pix[1]));
 
    // now process TRC
    pix[0] = Double(trc[0]);
    pix[1] = Double(trc[1]);
    success = inDC.toWorld(tempDir, pix);
    ASKAPCHECK(success, "Pixel to world coordinate conversion failed for input TRC: "<<inDC.errorMessage());
    success = refDC.toPixel(pix,tempDir);
    ASKAPCHECK(success, "World to pixel coordinate conversion failed for output TRC: "<<refDC.errorMessage());
    trc[0] = casa::Int(round(pix[0]));
    trc[1] = casa::Int(round(pix[1]));

    Vector<IPosition> corners(2);
    corners[0] = blc;
    corners[1] = trc;

    return corners;

}

bool LinmosAccumulator::CoordinatesAreConsistent(const CoordinateSystem& refCoordSys) {
    // Check to see if it makes sense to combine images with these coordinate systems.
    // Could get more tricky, but right now make sure any extra dimensions, such as frequency
    // and polarisation, are equal in the two systems.
    if ( itsInCoordSys.nCoordinates() != refCoordSys.nCoordinates() ) {
        //ASKAPLOG_INFO_STR(logger, "Coordinates are not consistent: shape mismatch");
        return false;
    }
    if (!allEQ(itsInCoordSys.worldAxisNames(), refCoordSys.worldAxisNames())) {
        //ASKAPLOG_INFO_STR(logger, "Coordinates are not consistent: axis name mismatch");
        return false;
    }
    if (!allEQ(itsInCoordSys.worldAxisUnits(), refCoordSys.worldAxisUnits())) {
        //ASKAPLOG_INFO_STR(logger, "Coordinates are not consistent: axis unit mismatch");
        return false;
    }
    return true;
}

bool LinmosAccumulator::CoordinatesAreEqual(void) {
    // Check to see if regridding is required. If they are equal there is no need.

    // Test the these things are set up...

    // Does something better already exist?
    double thresh = 1.0e-12;

    // Check that the input dimensionality is the same as that of the output
    if ( !CoordinatesAreConsistent(itsOutCoordSys) ) return false;

    // Also check that the size and centre of each dimension is the same.
    if ( itsInShape != itsOutShape ) {
        //ASKAPLOG_INFO_STR(logger, "Input and output coordinates are not equal: shape mismatch");
        return false;
    }
    // test that the grid properties of each dimension are equal
    for (casa::uInt dim=0; dim<itsInCoordSys.nCoordinates(); ++dim) {

        if ( (itsInCoordSys.referencePixel()[dim] != itsOutCoordSys.referencePixel()[dim]) ||
             (fabs(itsInCoordSys.increment()[dim] - itsOutCoordSys.increment()[dim]) > thresh) ||
             (fabs(itsInCoordSys.referenceValue()[dim] - itsOutCoordSys.referenceValue()[dim]) > thresh) ) {
            //ASKAPLOG_INFO_STR(logger, "Coordinates are not equal: coord system mismatch for dim " << dim);
            return false;
        }
    }
    return true;
}

/// @brief do the merge
/// @param[in] parset subset with parameters
static void merge(const LOFAR::ParameterSet &parset) {

    // initialise an image accumulator
    LinmosAccumulator accumulator;

    // load the parset
    bool expandable = true;
    const vector<string> inImgNames = parset.getStringVector("names", expandable);
    const vector<string> inWgtNames = parset.getStringVector("weights", vector<string>(), expandable);
    const string outImgName = parset.getString("outname");
    const string outWgtName = parset.getString("outweight");
    const string weightTypeName = parset.getString("weighttype");
    const string weightStateName = parset.getString("weightstate", "Corrected");
    // extra input:
    // the ability to also merge sensitivity and other images?
    // regridding options

    if ( !accumulator.checkParset(weightTypeName, weightStateName) ) return;

    // check for conflicts
    for (uint img = 0; img < inImgNames.size(); ++img ) {
        // check for conflicts
        ASKAPCHECK(inImgNames[img]!=outImgName,
                   "Output image, "<<outImgName<<", is present among the inputs");
        ASKAPCHECK(inWgtNames[img]!=outWgtName,
                   "Output weight image, "<<outWgtName<<", is present among the inputs");
    }

    // initialise an image accessor
    accessors::IImageAccess& iacc = SynthesisParamsHelper::imageHandler();

    // set the output coordinate system and shape, based on the overlap of input images
    accumulator.setOutputParameters(inImgNames, iacc);

    // set up the output pixel arrays
    Array<float> outPix(accumulator.outShape(),0.);
    Array<float> outWgtPix(accumulator.outShape(),0.);

    // set up an indexing vector for the arrays
    IPosition curpos(outPix.shape());
    ASKAPASSERT(curpos.nelements()>=2);
    for (uInt dim=0; dim<curpos.nelements(); ++dim) {
        curpos[dim] = 0;
    }

    // loop over the input images, reading each in an adding to the output pixel arrays
    for (uInt img = 0; img < inImgNames.size(); ++img ) {

        // short cuts
        string inImgName = inImgNames[img];
        ASKAPLOG_INFO_STR(logger, "Processing input image " << inImgName);
        string inWgtName = inWgtNames[img];
        ASKAPLOG_INFO_STR(logger, " - and input weight image " << inWgtName);

        // set the input coordinate system and shape
        accumulator.setInputParameters(inImgName, iacc);

        Array<float> inPix;
        Array<float> inWgtPix;
        inPix    = iacc.read(inImgName);
        inWgtPix = iacc.read(inWgtName);
        ASKAPASSERT(inPix.shape() == inWgtPix.shape());

        // set up an iterator for all directionCoordinate planes in the input image
        scimath::MultiDimArrayPlaneIter planeIter(accumulator.inShape());

        // test whether to simply add weighted pixels, or whether a regrid is required
        bool regridRequired = !accumulator.CoordinatesAreEqual();

        // if regridding is required, set up buffer some images
        if ( regridRequired ) {

            // currently all output planes have full-size, so only initialise once
            // would be faster if this was reduced to the size of the current input image
            if ( accumulator.outputBufferSetupRequired() ) {
                // set up temp images required for regridding
                accumulator.initialiseOutputBuffers();
                // set up regridder
                accumulator.initialiseRegridder();
            }

            // set up temp images required for regridding
            // are those of the previous iteration correctly freed?
            accumulator.initialiseInputBuffers();

        }

        // iterator over planes (e.g. freq & polarisation), regridding and accumulating weights and weighted images
        for (; planeIter.hasMore(); planeIter.next()) {

            // set the indices of any higher-order dimensions for this slice
            curpos = planeIter.position();

            if ( regridRequired ) {

                ASKAPLOG_INFO_STR(logger, " - regridding. Input pixel grid is different from the output.");
                // load input buffer for the current plane
                accumulator.loadInputBuffers(planeIter, inPix, inWgtPix);
                // call regrid for any buffered images
                accumulator.regrid();
                // update the accululation arrays for this plane
                accumulator.accumulatePlane(outPix, outWgtPix, curpos);

            } else {

                ASKAPLOG_INFO_STR(logger, " - not regridding. Input pixel grid is the same as the output.");
                // Update the accululation arrays for this plane.
                accumulator.accumulatePlane(outPix, outWgtPix, inPix, inWgtPix, curpos);

            }

        }

    } // loop over input images

    // deweight the image pixels
    // use another iterator to loop over planes
    ASKAPLOG_INFO_STR(logger, "Deweighting accumulated images");
    scimath::MultiDimArrayPlaneIter deweightIter(accumulator.outShape());
    for (; deweightIter.hasMore(); deweightIter.next()) {
        curpos = deweightIter.position();
        accumulator.deweightPlane(outPix, outWgtPix, curpos);
    }

    // write result
    ASKAPLOG_INFO_STR(logger, "Writing accumulated image to " << outImgName);
    iacc.create(outImgName, accumulator.outShape(), accumulator.outCoordSys());
    iacc.write(outImgName,outPix);
    ASKAPLOG_INFO_STR(logger, "Writing accumulated weight image to " << outWgtName);
    iacc.create(outWgtName, accumulator.outShape(), accumulator.outCoordSys());
    iacc.write(outWgtName,outWgtPix);

};

class LinmosApp : public askap::Application
{
    public:
        virtual int run(int argc, char* argv[])
        {
            LOFAR::ParameterSet subset(config().makeSubset("linmos."));
            SynthesisParamsHelper::setUpImageHandler(subset);
            merge(subset);
            return 0;
        }
};

int main(int argc, char *argv[])
{
    LinmosApp app;
    return app.main(argc, argv);
}
