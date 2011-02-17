/// @file
///
/// @brief Gridder adapter to do snap-shot imaging 
/// @details We can handle non-coplanarity via snap-shot imaging. For an approximately co-planar 
///     array the effect of w-term at a short time interval is equivalent to a shift. This gridder
///     uses an accessor adapter to monitor changes of the best-fit plane in the u,v,w-spce. If the
///     departure from the previously fitted plane exceeds the tolerance, the image is regridded to
///     a proper coordinate system (taken the shift out). This is an adapter, which can work with
///     any ASKAPsoft gridder. The real gridder, passed as a parameter during construction, does all 
///     the gridding job, so the snap-shot imaging can be combined with w-projection or any other
///     algorithm. The main driver for snap-shot imaging is an attempt to decrease the support size
///     of convolution functions (largely caused by w-projection).  
///
///     See also Ord et al., 2011, PASA (in press); arXiv:1010.1733
///
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".gridding");

#include <gridding/SnapShotImagingGridderAdapter.h>
#include <askap/AskapError.h>
#include <utils/MultiDimArrayPlaneIter.h>

using namespace askap;
using namespace askap::synthesis;

/// @brief initialise the adapter
/// @details
/// @param[in] gridder a shared pointer to the gridder to be wrapped by this adapter
/// @param[in] tolerance w-term tolerance in wavelengths (a new fit is performed if the old
/// plane gives w-deviation exceeding this value)
SnapShotImagingGridderAdapter::SnapShotImagingGridderAdapter(const boost::shared_ptr<IVisGridder> &gridder,
                               const double tolerance) :
     itsGridder(gridder), itsAccessorAdapter(tolerance), itsDoPSF(false), itsCoeffA(0.), itsCoeffB(0.)
{
  ASKAPCHECK(itsGridder, "SnapShotImagingGridderAdapter should only be initialised with a valid gridder");
}

/// @brief copy constructor
/// @details We need this because the gridder doing actual work is held by a shared pointer,
/// which is a non-trivial type
/// @param[in] other an object to copy from
SnapShotImagingGridderAdapter::SnapShotImagingGridderAdapter(const SnapShotImagingGridderAdapter &other) :
    IVisGridder(other), itsAccessorAdapter(other.itsAccessorAdapter.tolerance()), itsDoPSF(other.itsDoPSF),
    itsAxes(other.itsAxes), itsImageBuffer(other.itsImageBuffer.copy()),
    itsWeightsBuffer(other.itsWeightsBuffer.copy()), itsCoeffA(other.itsCoeffA), itsCoeffB(other.itsCoeffB)
{
  ASKAPCHECK(other.itsGridder, 
       "copy constructor of SnapShotImagingGridderAdapter got an object somehow set up with an empty gridder");
  ASKAPCHECK(!other.itsAccessorAdapter.isAssociated(), 
     "An attempt to copy gridder adapter with the accessor adapter associated with some real data accessor. This shouldn't happen.");
  itsGridder = other.itsGridder->clone();  
}

/// @brief clone a copy of this gridder
/// @return shared pointer to the clone
boost::shared_ptr<IVisGridder> SnapShotImagingGridderAdapter::clone()
{
  boost::shared_ptr<SnapShotImagingGridderAdapter> newOne(new SnapShotImagingGridderAdapter(*this));
  return newOne;
}

/// @brief initialise the gridding
/// @details
/// @param[in] axes axes specifications
/// @param[in] shape Shape of output image: cube: u,v,pol,chan
/// @param[in] dopsf Make the psf?
void SnapShotImagingGridderAdapter::initialiseGrid(const scimath::Axes& axes,
                const casa::IPosition& shape, const bool dopsf)
{
  itsDoPSF = dopsf; // other fields are unused for the PSF gridder
  if (dopsf) {
      // do the standard initialisation for the PSF gridder
      ASKAPDEBUGASSERT(itsGridder);
      itsGridder->initialiseGrid(axes,shape,dopsf);
  } else {
      ASKAPDEBUGASSERT(shape.nelements() >= 2);
      itsAxes = axes;
      // initialise the buffers for final image and weight
      itsImageBuffer.resize(shape);
      itsWeightsBuffer.resize(shape);
      itsImageBuffer.set(0.);
      itsWeightsBuffer.set(0.);
  }
}

/// @brief grid the visibility data.
/// @param[in] acc const data accessor to work with
void SnapShotImagingGridderAdapter::grid(IConstDataAccessor& acc)
{
  ASKAPDEBUGASSERT(itsGridder);
  if (isPSFGridder()) {
      itsGridder->grid(acc);
  } else {
      itsAccessorAdapter.associate(acc);
      const scimath::ChangeMonitor cm = itsAccessorAdapter.planeChangeMonitor();
      // we need a way to do the check before the grid call here
      if (cm != itsAccessorAdapter.planeChangeMonitor()) {
          finaliseGriddingOfCurrentPlane();
          // update plane parameters
          itsCoeffA = itsAccessorAdapter.coeffA();
          itsCoeffB = itsAccessorAdapter.coeffB();          
      }
      itsGridder->grid(itsAccessorAdapter);
      // we don't really need this line
      itsAccessorAdapter.detach();
  }
}

/// @brief form the final output image
/// @param[in] out output double precision image or PSF
void SnapShotImagingGridderAdapter::finaliseGrid(casa::Array<double>& out)
{
  ASKAPDEBUGASSERT(itsGridder);
  itsGridder->finaliseGrid(out);
}

/// @brief finalise weights
/// @details Form the sum of the convolution function squared, multiplied by the weights for each
/// different convolution function. This is used in the evaluation of the second derivative.
/// @param[in] out output double precision sum of weights images
void SnapShotImagingGridderAdapter::finaliseWeights(casa::Array<double>& out)
{
  ASKAPDEBUGASSERT(itsGridder);
  itsGridder->finaliseWeights(out);
}

/// @brief initialise the degridding
/// @param[in] axes axes specifications
/// @param[in] image input image cube: u,v,pol,chan
void SnapShotImagingGridderAdapter::initialiseDegrid(const scimath::Axes& axes,
					const casa::Array<double>& image)
{
  ASKAPDEBUGASSERT(itsGridder);
  itsDoPSF = false;
  itsAxes = axes;
  itsGridder->initialiseDegrid(axes,image);
}					

/// @brief make context-dependant changes to the gridder behaviour
/// @param[in] context context description
void SnapShotImagingGridderAdapter::customiseForContext(const std::string &context)
{
  ASKAPDEBUGASSERT(itsGridder);
  itsGridder->customiseForContext(context);
}
			
/// @brief set visibility weights
/// @param[in] viswt shared pointer to visibility weights
void SnapShotImagingGridderAdapter::initVisWeights(const IVisWeights::ShPtr &viswt)
{
  ASKAPDEBUGASSERT(itsGridder);
  itsGridder->initVisWeights(viswt);
}

/// @brief degrid the visibility data.
/// @param[in] acc non-const data accessor to work with  
void SnapShotImagingGridderAdapter::degrid(IDataAccessor& acc)
{
  ASKAPDEBUGASSERT(itsGridder);
  itsGridder->degrid(acc);
}

/// @brief finalise degridding
void SnapShotImagingGridderAdapter::finaliseDegrid()
{
  ASKAPDEBUGASSERT(itsGridder);
  itsGridder->finaliseDegrid();
}

/// @brief finalise gridding for the current plane
/// @details We execute the gridder pointed by itsGridder
/// multiple times. Every time the best fitted plane changes
/// we have to finalise gridding with the wrapped gridder,
/// regrid the result into target frame and add it to buffers.
/// The same has to be done for both image and weights. This
/// method encapsulates all these operations.
void SnapShotImagingGridderAdapter::finaliseGriddingOfCurrentPlane()
{
  ASKAPDEBUGASSERT(itsGridder);
  casa::Array<double> scratch(itsImageBuffer.shape());
  itsGridder->finaliseGrid(scratch);
  imageRegrid(scratch, itsImageBuffer, true);
  itsGridder->finaliseWeights(scratch);
  imageRegrid(scratch, itsWeightsBuffer, true);  
}

/// @brief regrid images between frames
/// @details This method does the core regridding procedure. It iterates
/// over 2D planes of the input array, regrids them into the other frame
/// and either adds the result to the appropriate plane of the output array,
/// if the regridding is into the target frame or replaces the result if it is
/// from the target frame. 
/// @param[in] input input array to be regridded
/// @param[out] output output array
/// @param[in] toTarget true, if regridding is from the current frame into the 
/// target frame (for gridding); false if regridding is from the target frame 
/// into the current frame (for degridding)
/// @note The output and input arrays should have the same shape. The iteration
/// over 2D planes is perfromed explicitly to avoid initialising large scratch 
/// buffers. An exception is raised if input and output arrays have different 
/// shapes
void SnapShotImagingGridderAdapter::imageRegrid(const casa::Array<double> &input, 
           casa::Array<double> &output, bool toTarget) const
{
   if (toTarget) {
       ASKAPLOG_INFO_STR(logger, "Regridding image from the frame corresponding to the fitted plane w = u * "<<
              coeffA()<<" + v * "<<coeffB()<<", into the target frame");
   } else {
       ASKAPLOG_INFO_STR(logger, 
           "Regridding image from the input frame into a frame corresponding to the fitted plane w = u * "<<
              coeffA()<<" + v * "<<coeffB());       
   }
   ASKAPCHECK(input.shape() == output.shape(), 
           "The shape of input and output arrays should be identical, input.shape()="<<
              input.shape()<<", output.shape()="<<output.shape());
   ASKAPDEBUGASSERT(input.shape().nelements() >= 2);
   
   // constness is conceptual, we don't do any assignments to the input array
   // the following line doesn't copy the data (reference semantics)
   casa::Array<double> inRef(input);
               
   for (scimath::MultiDimArrayPlaneIter planeIter(input.shape()); planeIter.hasMore(); planeIter.next()) {
        // proper regridding comes here, now just do the copy to debug the rest of the logic
        casa::Array<double> outRef = planeIter.getPlane(output);
        if (toTarget) {
            outRef += planeIter.getPlane(inRef);
        } else {
            outRef = planeIter.getPlane(inRef);
        }
   }
}

