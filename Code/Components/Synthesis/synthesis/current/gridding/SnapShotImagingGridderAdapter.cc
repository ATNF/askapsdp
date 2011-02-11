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

#include <gridding/SnapShotImagingGridderAdapter.h>
#include <askap/AskapError.h>

using namespace askap;
using namespace askap::synthesis;

/// @brief initialise the adapter
/// @details
/// @param[in] gridder a shared pointer to the gridder to be wrapped by this adapter
SnapShotImagingGridderAdapter::SnapShotImagingGridderAdapter(const boost::shared_ptr<IVisGridder> &gridder) :
     itsGridder(gridder) 
{
  ASKAPCHECK(itsGridder, "SnapShotImagingGridderAdapter should only be initialised with a valid gridder");
}

/// @brief copy constructor
/// @details We need this because the gridder doing actual work is held by a shared pointer,
/// which is a non-trivial type
/// @param[in] other an object to copy from
SnapShotImagingGridderAdapter::SnapShotImagingGridderAdapter(const SnapShotImagingGridderAdapter &other) :
    IVisGridder(other)
{
  ASKAPCHECK(other.itsGridder, 
       "copy constructor of SnapShotImagingGridderAdapter got an object somehow set up with an empty gridder");
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
  ASKAPDEBUGASSERT(itsGridder);
  itsGridder->initialiseGrid(axes,shape,dopsf);
}

/// @brief grid the visibility data.
/// @param[in] acc const data accessor to work with
void SnapShotImagingGridderAdapter::grid(IConstDataAccessor& acc)
{
  ASKAPDEBUGASSERT(itsGridder);
  itsGridder->grid(acc);
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


