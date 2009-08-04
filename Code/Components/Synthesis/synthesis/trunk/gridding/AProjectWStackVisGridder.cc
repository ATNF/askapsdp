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

#include <gridding/AProjectWStackVisGridder.h>

#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/ArrayMath.h>
#include <measures/Measures/MDirection.h>
#include <casa/Quanta/MVDirection.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVTime.h>

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".gridding");

#include <askap/AskapError.h>
#include <askap/AskapUtil.h>

#include <fft/FFTWrapper.h>
#include <gridding/IBasicIllumination.h>

#include <utils/PaddingUtils.h>

using namespace askap;

namespace askap {
namespace synthesis {

AProjectWStackVisGridder::AProjectWStackVisGridder(const boost::shared_ptr<IBasicIllumination const> &illum,
        const double wmax, const int nwplanes,
        const int overSample, const int maxSupport, const int limitSupport, 
        const int maxFeeds,
        const int maxFields, const int maxAnts,
        const double pointingTol, const double paTol,
        const bool frequencyDependent, const std::string& name) :
    WStackVisGridder(wmax, nwplanes), itsReferenceFrequency(0.0),
    itsIllumination(illum),
    itsMaxFeeds(maxFeeds), itsMaxFields(maxFields), itsMaxAnts(maxAnts),
    itsPointingTolerance(pointingTol),       itsParallacticAngleTolerance(paTol),
    itsFreqDep(frequencyDependent), itsIndicesValid(false),
    itsNumberOfCFGenerations(0),
    itsNumberOfIterations(0), itsNumberOfCFGenerationsDueToPA(0)
{	
    ASKAPCHECK(maxFeeds>0, "Maximum number of feeds must be one or more");
    ASKAPCHECK(maxFields>0, "Maximum number of fields must be one or more");
    ASKAPCHECK(maxAnts>0, "Maximum number of antennas must be one or more");
    ASKAPCHECK(overSample>0, "Oversampling must be greater than 0");
    ASKAPCHECK(maxSupport>0, "Maximum support must be greater than 0")
    ASKAPCHECK(pointingTol>0.0, "Pointing tolerance must be greater than 0.0");
    ASKAPLOG_INFO_STR(logger, "Maximum number of antennas allowed = " << maxAnts);
    ASKAPDEBUGASSERT(itsIllumination);
    itsSupport=0;
    itsOverSample=overSample;
    itsMaxSupport=maxSupport;
    itsLimitSupport=limitSupport;
    itsName=name;

    itsSlopes.resize(2, itsMaxFeeds, itsMaxFields);
    itsSlopes.set(0.0);
    itsDone.resize(itsMaxFeeds, itsMaxFields);
    itsDone.set(false);
    itsPointings.resize(itsMaxFeeds, itsMaxFields);
    itsPointings.set(casa::MVDirection());
    itsLastField=-1;
    itsCurrentField=0;
}

/// @brief copy constructor
/// @details It is required to decouple internal arrays between input object
/// and this copy.
/// @param[in] other input object
/// @note illumination pattern is copied as a shared pointer, hence referencing
/// the same model

AProjectWStackVisGridder::AProjectWStackVisGridder(const AProjectWStackVisGridder &other) :
    WStackVisGridder(other), itsReferenceFrequency(other.itsReferenceFrequency),
    itsIllumination(other.itsIllumination), itsMaxFeeds(other.itsMaxFeeds),
    itsMaxFields(other.itsMaxFields), itsMaxAnts(other.itsMaxAnts),
    itsPointingTolerance(other.itsPointingTolerance),
    itsParallacticAngleTolerance(other.itsParallacticAngleTolerance),
    itsLastField(other.itsLastField), itsCurrentField(other.itsCurrentField),
    itsFreqDep(other.itsFreqDep), itsMaxSupport(other.itsMaxSupport),
    itsLimitSupport(other.itsLimitSupport),
    itsCMap(other.itsCMap.copy()), itsSlopes(other.itsSlopes.copy()),
    itsDone(other.itsDone.copy()), itsPointings(other.itsPointings.copy()), 
    itsIndicesValid(other.itsIndicesValid),
    itsNumberOfCFGenerations(other.itsNumberOfCFGenerations),
    itsNumberOfIterations(other.itsNumberOfIterations),
    itsNumberOfCFGenerationsDueToPA(other.itsNumberOfCFGenerationsDueToPA), 
    itsCFParallacticAngles(other.itsCFParallacticAngles)
{
    if (other.itsPattern) {
        itsPattern.reset(new UVPattern(*(other.itsPattern)));
    }
}

AProjectWStackVisGridder::~AProjectWStackVisGridder() {
    size_t nUsed = 0;
    for (casa::uInt feed = 0; feed<itsDone.nrow(); ++feed) {
        for (casa::uInt field = 0; field<itsDone.ncolumn(); ++field) {
            if (itsDone(feed,field)) {
                ++nUsed;
            }
        }
    }
    if (itsDone.nelements()) {
        ASKAPLOG_INFO_STR(logger, "AProjectWStackVisGridder CF cache memory utilisation: "<<
                double(nUsed)/double(itsDone.nrow()*itsDone.ncolumn())*100<<"% of maxfeed*maxfield");
    }
    if (itsNumberOfIterations != 0) {
        ASKAPLOG_INFO_STR(logger, "AProjectWStackVisGridder cache was rebuild "<<
                itsNumberOfCFGenerations<<" times for "<<itsNumberOfIterations<<" iterations");
        if (itsNumberOfCFGenerations != 0) {
            ASKAPLOG_INFO_STR(logger, "Parallactic angle change caused "<<
                    itsNumberOfCFGenerationsDueToPA<<" of those rebuilds ("<<
                    double(itsNumberOfCFGenerationsDueToPA)/double(itsNumberOfCFGenerations)*100<<
                    " %)");
        }   
        ASKAPLOG_INFO_STR(logger, "CF cache utilisation is "<<
                (1.-double(itsNumberOfCFGenerations)/double(itsNumberOfIterations))*100.<<" %");
    }
}

/// Clone a copy of this Gridder
IVisGridder::ShPtr AProjectWStackVisGridder::clone() {
    return IVisGridder::ShPtr(new AProjectWStackVisGridder(*this));
}

/// @brief initialise sum of weights
/// @details We keep track the number of times each convolution function is used per
/// channel and polarisation (sum of weights). This method is made virtual to be able
/// to do gridder specific initialisation without overriding initialiseGrid.
/// This method accepts no parameters as itsShape, itsNWPlanes, etc should have already
/// been initialised by the time this method is called.
void AProjectWStackVisGridder::initialiseSumOfWeights()
{
    // this method is hopefully just a temporary stub until we figure out a better way of
    // managing a cache of convolution functions. It skips initialisation if itsSupport is
    // not zero, which means that some initialisation has been done before. 
    // Note, it is not a very good way of doing things!
    if (itsSupport == 0) {
        WStackVisGridder::initialiseSumOfWeights();
    }
}

/// Initialize the indices into the cube.
void AProjectWStackVisGridder::initIndices(const IConstDataAccessor& acc) {

    // Validate cache using first row only
    bool newField=true;
    ASKAPDEBUGASSERT(acc.nRow()>0);

    int firstFeed=acc.feed1()(0);
    ASKAPCHECK(firstFeed<itsMaxFeeds, "Too many feeds: increase maxfeeds");
    casa::MVDirection firstPointing=acc.pointingDir1()(0);

    for (int field=itsLastField; field>-1; --field) {
        if (firstPointing.separation(itsPointings(firstFeed, field))
                <itsPointingTolerance) {
            itsCurrentField=field;
            newField=false;
            break;
        }
    }
    if (newField) {
        itsLastField++;
        itsCurrentField=itsLastField;
        ASKAPCHECK(itsCurrentField<itsMaxFields,
                "Too many fields: increase maxfields " << itsMaxFields);
        itsPointings(firstFeed, itsCurrentField)=firstPointing;
        ASKAPLOG_INFO_STR(logger, "Found new field " << itsCurrentField<<" at "<<
                printDirection(firstPointing));
    }

    const int nSamples = acc.nRow();
    const int maxNSamples = itsMaxFeeds*itsMaxFields*itsMaxAnts*(itsMaxAnts+1)/2;
    const int nChan = acc.nChannel();      
    const int nPol = acc.nPol();

    // Given above are checks for maxFeeds and maxFields, this assert should
    // really only fail if maxAnt is not high enough.
    ASKAPCHECK(nSamples<maxNSamples, "Number of samples " << nSamples
            << " exceeds expected maximum " << maxNSamples);

    if (itsCMap.shape() != casa::IPosition(3,maxNSamples,nPol,nChan)) {
        itsIndicesValid = false;      
        ASKAPLOG_INFO_STR(logger, "Resizing convolution function map: new " << maxNSamples
                << " old " << itsCMap.shape()(0) << " samples"); 
        itsCMap.resize(maxNSamples, nPol, nChan);
        itsCMap.set(0);
    }
    ASKAPDEBUGASSERT(itsCMap.shape() == casa::IPosition(3,maxNSamples,nPol,nChan));

    /// @todo Select max feeds more carefully

    itsGMap.resize(nSamples, nPol, nChan);
    itsGMap.set(0);

    const int cenw=(itsNWPlanes-1)/2;

    const casa::Vector<casa::RigidVector<double, 3> > &rotatedUVW = acc.rotatedUVW(getTangentPoint());

    for (int i=0; i<nSamples; ++i) {
        const int feed=acc.feed1()(i);
        ASKAPCHECK(feed<itsMaxFeeds, "Exceeded specified maximum number of feeds");
        ASKAPCHECK(feed>-1, "Illegal negative feed number");

        const double w=(rotatedUVW(i)(2))/(casa::C::c);

        for (int chan=0; chan<nChan; ++chan) {
            const double freq=acc.frequency()[chan];
            for (int pol=0; pol<nPol; pol++) {
                int index = -1;
                /// Order is (chan, feed)
                if(itsFreqDep) {
                    index = chan+nChan*(feed+itsMaxFeeds*itsCurrentField);
                    ASKAPCHECK(index<itsMaxFields*itsMaxFeeds*nChan, "CMap index too large");
                } else {
                    index = (feed+itsMaxFeeds*itsCurrentField);
                    ASKAPCHECK(index < itsMaxFields*itsMaxFeeds, "CMap index too large");
                }

                ASKAPCHECK(index>-1, "CMap index less than zero");

                if (itsIndicesValid) {
                    if (itsCMap(i, pol, chan) != index) {
                        itsIndicesValid = false;
                        itsCMap(i, pol, chan) = index;
                    }
                } else {
                    itsCMap(i, pol, chan) = index;
                }
                /// Calculate the index into the grids
                if (itsNWPlanes>1) {
                    itsGMap(i, pol, chan)=cenw+nint(w*freq/itsWScale);
                } else {
                    itsGMap(i, pol, chan)=0;
                }
                ASKAPCHECK(itsGMap(i, pol, chan)<itsNWPlanes,
                        "W scaling error: recommend allowing larger range of w, you have w="<<w*freq<<" wavelengths");
                ASKAPCHECK(itsGMap(i, pol, chan)>-1,
                        "W scaling error: recommend allowing larger range of w, you have w="<<w*freq<<" wavelengths");
            }
        }
    }
    if (!itsIndicesValid) {
        ASKAPLOG_INFO_STR(logger, "Convolution function map was incorrect - invalidating CMap");
    }
}
/// @brief Initialise the gridding
/// @param axes axes specifications
/// @param shape Shape of output image: u,v,pol,chan
/// @param dopsf Make the psf?
void AProjectWStackVisGridder::initialiseGrid(const scimath::Axes& axes,  const casa::IPosition& shape, const bool dopsf)
{
    WStackVisGridder::initialiseGrid(axes,shape,dopsf);

    /// Limit the size of the convolution function since
    /// we don't need it finely sampled in image space. This
    /// will reduce the time taken to calculate it.
    const casa::uInt nx=std::min(itsMaxSupport, itsShape(0));
    const casa::uInt ny=std::min(itsMaxSupport, itsShape(1));

    ASKAPLOG_INFO_STR(logger, "Shape for calculating gridding convolution function = "
            << nx << " by " << ny << " pixels");

    // this is just a buffer in the uv-space
    itsPattern.reset(new UVPattern(nx,ny, itsUVCellSize(0),itsUVCellSize(1),itsOverSample));

    // this invalidates the cache of CFs      
    itsIndicesValid = false;
    if (!itsIndicesValid) {
        ASKAPLOG_INFO_STR(logger, "Initializing grid - invalidating CMap");
    }
}

/// @brief Initialise the degridding
/// @param axes axes specifications
/// @param image Input image: cube: u,v,pol,chan
void AProjectWStackVisGridder::initialiseDegrid(const scimath::Axes& axes,
        const casa::Array<double>& image)
{
    WStackVisGridder::initialiseDegrid(axes,image);      
    /// Limit the size of the convolution function since
    /// we don't need it finely sampled in image space. This
    /// will reduce the time taken to calculate it.
    const casa::uInt nx=std::min(itsMaxSupport, itsShape(0));
    const casa::uInt ny=std::min(itsMaxSupport, itsShape(1));

    ASKAPLOG_INFO_STR(logger, "Shape for calculating degridding convolution function = "
            << nx << " by " << ny << " pixels");

    // this is just a buffer in the uv-space
    itsPattern.reset(new UVPattern(nx,ny, itsUVCellSize(0),itsUVCellSize(1),itsOverSample));      

    // this invalidates the cache of CFs      
    itsIndicesValid = false;
    if (!itsIndicesValid) {
        ASKAPLOG_INFO_STR(logger, "Initializing degrid - invalidating CMap");
    }
}

/// Initialize the convolution function into the cube. If necessary this
/// could be optimized by using symmetries.
/// @todo Make initConvolutionFunction more robust
void AProjectWStackVisGridder::initConvolutionFunction(const IConstDataAccessor& acc) {

    ASKAPDEBUGASSERT(itsIllumination);
    ASKAPDEBUGASSERT(itsPattern);
    // just to avoid a repeated call to a virtual function from inside the loop
    const bool hasSymmetricIllumination = itsIllumination->isSymmetric();
    const int nSamples = acc.nRow();

    if (itsIndicesValid && !hasSymmetricIllumination) {
        // need to check parallactic angles here
        //          ASKAPDEBUGASSERT(itsCFParallacticAngles.nelements() == casa::uInt(nSamples));
        const casa::Vector<casa::Float> &feed1PAs = acc.feed1PA();
        //          ASKAPDEBUGASSERT(feed1PAs.nelements() == casa::uInt(nSamples));
        for (int row = 0; row<nSamples; ++row) {
            if (fabs(feed1PAs[row] - itsCFParallacticAngles[row])<itsParallacticAngleTolerance) {
                itsIndicesValid = false;
                ++itsNumberOfCFGenerationsDueToPA;
                break;
            }
        }
        if(!itsIndicesValid) {
            //	    ASKAPLOG_INFO_STR(logger, "Parallactic angle change on non-symmetric beam - invalidating CMap and CFs");
            itsDone.set(false);
            ++itsNumberOfCFGenerations;
        }
    }

    ++itsNumberOfIterations;

    casa::MVDirection out = getImageCentre();

    /// We have to calculate the lookup function converting from
    /// row and channel to plane of the w-dependent convolution
    /// function
    const int nChan = itsFreqDep ? acc.nChannel() : 1;

    if(itsSupport==0) {
        ASKAPLOG_INFO_STR(logger, "Resizing convolution function to "
                << itsOverSample << "*" << itsOverSample << "*" << itsMaxFeeds << "*" << itsMaxFields << "*" << nChan << " entries");
        itsConvFunc.resize(itsOverSample*itsOverSample*itsMaxFeeds*itsMaxFields*nChan);

        ASKAPLOG_INFO_STR(logger, "Resizing sum of weights to " << itsMaxFeeds << "*" << itsMaxFields << "*" << nChan << " entries");
        itsSumWeights.resize(itsMaxFeeds*itsMaxFields*nChan, itsShape(2), itsShape(3));
        itsSumWeights.set(0.0);
    }

    UVPattern &pattern = *itsPattern;
    const casa::uInt nx = pattern.uSize();
    const casa::uInt ny = pattern.vSize();


    int nDone=0;
    for (int row=0; row<nSamples; ++row) {
        const int feed=acc.feed1()(row);

        if (!itsDone(feed, itsCurrentField)) {
            itsDone(feed, itsCurrentField)=true;
            nDone++;
            casa::MVDirection offset(acc.pointingDir1()(row).getAngle());
            itsSlopes(0, feed, itsCurrentField) = isPSFGridder() ? 0. : sin(offset.getLong()
                    -out.getLong()) *cos(offset.getLat());
            itsSlopes(1, feed, itsCurrentField)= isPSFGridder() ? 0. : sin(offset.getLat())
                *cos(out.getLat()) - cos(offset.getLat())*sin(out.getLat())
                *cos(offset.getLong()-out.getLong());

            const double parallacticAngle = hasSymmetricIllumination ? 0. : acc.feed1PA()(row);

            for (int chan=0; chan<nChan; chan++) {
                /// Extract illumination pattern for this channel
                itsIllumination->getPattern(acc.frequency()[chan], pattern,
                        itsSlopes(0, feed, itsCurrentField),
                        itsSlopes(1, feed, itsCurrentField), 
                        parallacticAngle);

                /// Now convolve the disk with itself using an FFT
                scimath::fft2d(pattern.pattern(), false);

                double peak=0.0;
                for (casa::uInt ix=0; ix<nx; ++ix) {
                    for (casa::uInt iy=0; iy<ny; ++iy) {
                        pattern(ix, iy)=pattern(ix,iy)*conj(pattern(ix,iy));
                        if(casa::abs(pattern(ix,iy))>peak) peak=casa::abs(pattern(ix,iy));
                    }
                }
                if(peak>0.0) {
                    pattern.pattern()*=casa::Complex(1.0/peak);
                }
                // The maximum will be 1.0
                //	    ASKAPLOG_INFO_STR(logger, "Max of FT of convolution function = " << casa::max(pattern.pattern()));
                scimath::fft2d(pattern.pattern(), true);	
                // Now correct for normalization of FFT
                pattern.pattern()*=casa::Complex(1.0/(double(nx)*double(ny)));

                if (itsSupport==0) {
                    // we probably need a proper support search here
                    // it can be encapsulated in a method of the UVPattern class
                    itsSupport = pattern.maxSupport();
                    ASKAPCHECK(itsSupport>0,
                            "Unable to determine support of convolution function");
                    ASKAPCHECK(itsSupport*itsOverSample<int(nx)/2,
                            "Overflowing convolution function - increase maxSupport or decrease overSample");
                    if (itsLimitSupport > 0  &&  itsSupport > itsLimitSupport) {
                        ASKAPLOG_INFO_STR(logger, "Convolution function support = "
                                << itsSupport << " pixels exceeds upper support limit; "
                                << "set to limit = " << itsLimitSupport << " pixels");
                        itsSupport = itsLimitSupport;
                    }

                    itsCSize=2*itsSupport+1;
                    // just for logging
                    const double cell = std::abs(pattern.uCellSize())*(casa::C::c/acc.frequency()[chan]);
                    ASKAPLOG_INFO_STR(logger, "Convolution function support = "
                            << itsSupport << " pixels, size = " << itsCSize
                            << " pixels");
                    ASKAPLOG_INFO_STR(logger, "Maximum extent = "<< itsSupport
                            *cell << " (m) sampled at "<< cell
                            << " (m)");
                    ASKAPLOG_INFO_STR(logger, "Number of planes in convolution function = "
                            << itsConvFunc.size());
                } // if itsSupport uninitialized
                int zIndex=chan+nChan*(feed+itsMaxFeeds*itsCurrentField);

                // Since we are decimating, we need to rescale by the
                // decimation factor
                float rescale=float(itsOverSample*itsOverSample);
                for (int fracu=0; fracu<itsOverSample; fracu++) {
                    for (int fracv=0; fracv<itsOverSample; fracv++) {
                        int plane=fracu+itsOverSample*(fracv+itsOverSample*zIndex);
                        ASKAPDEBUGASSERT(plane>=0 && plane<int(itsConvFunc.size()));
                        itsConvFunc[plane].resize(itsCSize, itsCSize);
                        itsConvFunc[plane].set(0.0);
                        // Now cut out the inner part of the convolution function and
                        // insert it into the cache
                        for (int iy=-itsSupport; iy<itsSupport; iy++) {
                            for (int ix=-itsSupport; ix<itsSupport; ix++) {
                                itsConvFunc[plane](ix+itsSupport, iy+itsSupport)
                                    = rescale * pattern(itsOverSample*ix+fracu+nx/2,
                                            itsOverSample*iy+fracv+ny/2);
                            } // for ix
                        } // for iy
                        //
                        //ASKAPLOG_INFO_STR(logger, "convolution function for channel "<<chan<<
                        //   " plane="<<plane<<" has an integral of "<<sum(itsConvFunc[plane]));						
                        //
                    } // for fracv
                } // for fracu								
            } // for chan
        } // if !isDone
    } // for row

    ASKAPCHECK(itsSupport>0, "Support not calculated correctly");
    if (!itsIndicesValid && !hasSymmetricIllumination) {
        itsCFParallacticAngles.assign(acc.feed1PA().copy());
    }
    itsIndicesValid = true;
}

// To finalize the transform of the weights, we use the following steps:
// 1. For each plane of the convolution function, transform to image plane
// and multiply by conjugate to get abs value squared.
// 2. Sum all planes weighted by the weight for that convolution function.
void AProjectWStackVisGridder::finaliseWeights(casa::Array<double>& out) {

    ASKAPLOG_INFO_STR(logger, "Calculating sum of weights image");
    ASKAPDEBUGASSERT(itsShape.nelements()>=3);

    const int nx=itsShape(0);
    const int ny=itsShape(1);
    const int nPol=itsShape(2);
    const int nChan=itsShape(3);

    const int nZ=itsSumWeights.shape()(0);

    /// We must pad the convolution function to full size, reverse transform
    /// square, and sum multiplied by the corresponding weight
    const int cnx=std::min(itsMaxSupport, nx);
    const int cny=std::min(itsMaxSupport, ny);
    const int ccenx = cnx/2;
    const int cceny = cny/2;

    /// This is the output array before sinc padding
    casa::Array<double> cOut(casa::IPosition(4, cnx, cny, nPol, nChan));
    cOut.set(0.0);

    // for debugging
    double totSumWt = 0.;

    /// itsSumWeights has one element for each separate data plane (feed, field, chan)
    /// itsConvFunc has overSampling**2 planes for each separate data plane (feed, field, chan)
    /// We choose the convolution function at zero fractional offset in u,v 
    for (int iz=0; iz<nZ; ++iz) {
        const int plane=itsOverSample*itsOverSample*iz;

        bool hasData=false;
        for (int chan=0; chan<nChan; ++chan) {
            for (int pol=0; pol<nPol; ++pol) {
                const double wt = itsSumWeights(iz, pol, chan);
                if (wt > 0.0) {
                    hasData=true;
                    totSumWt += wt;
                    //break;
                }
            }
        }
        if(hasData) {

            // Now fill the inner part of the uv plane with the convolution function
            // and transform to obtain the image. The uv sampling is fixed here
            // so the total field of view is itsOverSample times larger than the
            // original field of view.
            /// Work space
            casa::Matrix<casa::Complex> thisPlane(cnx, cny);
            thisPlane.set(0.0);
            for (int iy=-itsSupport; iy<+itsSupport; ++iy) {
                for (int ix=-itsSupport; ix<+itsSupport; ++ix) {
                    thisPlane(ix+ccenx, iy+cceny)=itsConvFunc[plane](ix+itsSupport, iy+itsSupport);
                }
            }

            //	  	  ASKAPLOG_INFO_STR(logger, "Convolution function["<< iz << "] peak = "<< peak);
            scimath::fft2d(thisPlane, false);
            thisPlane*=casa::Complex(nx*ny);
            float peak=real(casa::max(casa::abs(thisPlane)));
            //	  ASKAPLOG_INFO_STR(logger, "Transform of convolution function["<< iz
            //			    << "] peak = "<< peak);

            if(peak>0.0) {
                thisPlane*=casa::Complex(1.0/peak);
            }

            // Now we need to cut out only the part inside the field of view
            for (int chan=0; chan<nChan; ++chan) {
                for (int pol=0; pol<nPol; ++pol) {
                    casa::IPosition ip(4, 0, 0, pol, chan);
                    const double wt=itsSumWeights(iz, pol, chan);
                    for (int ix=0; ix<cnx; ++ix) {
                        ip(0)=ix;
                        for (int iy=0; iy<cny; ++iy) {
                            ip(1)=iy;
                            cOut(ip)+=wt*casa::real(thisPlane(ix, iy)*conj(thisPlane(ix, iy)));
                        }
                    }
                }
            }
        } // if has data
    } // loop over convolution functions
    scimath::PaddingUtils::fftPad(cOut, out, paddingFactor());
    ASKAPLOG_INFO_STR(logger, 
            "Finished finalising the weights, the sum over all convolution functions is "<<totSumWt);	
}

int AProjectWStackVisGridder::cIndex(int row, int pol, int chan) {
    return itsCMap(row, pol, chan);
}

void AProjectWStackVisGridder::correctConvolution(casa::Array<double>& grid) {
}

}
}
