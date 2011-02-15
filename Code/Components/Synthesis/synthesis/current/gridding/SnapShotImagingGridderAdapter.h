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

#ifndef SNAP_SHOT_IMAGING_GRIDDER_ADAPTER_H
#define SNAP_SHOT_IMAGING_GRIDDER_ADAPTER_H

#include <gridding/IVisGridder.h>
#include <boost/shared_ptr.hpp>
#include <dataaccess/BestWPlaneDataAccessor.h>

namespace askap {

namespace synthesis {

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
/// @ingroup gridding
class SnapShotImagingGridderAdapter : public IVisGridder 
{
public:
   /// @brief initialise the adapter
   /// @details
   /// @param[in] gridder a shared pointer to the gridder to be wrapped by this adapter
   /// @param[in] tolerance w-term tolerance in wavelengths (a new fit is performed if the old
   /// plane gives w-deviation exceeding this value)
   SnapShotImagingGridderAdapter(const boost::shared_ptr<IVisGridder> &gridder, const double tolerance);


   /// @brief copy constructor
   /// @details We need this because the gridder doing actual work is held by a shared pointer,
   /// which is a non-trivial type
   /// @param[in] other an object to copy from
   SnapShotImagingGridderAdapter(const SnapShotImagingGridderAdapter &other);

   /// @brief clone a copy of this gridder
   /// @return shared pointer to the clone
   virtual boost::shared_ptr<IVisGridder> clone();

   /// @brief initialise the gridding
   /// @details
   /// @param[in] axes axes specifications
   /// @param[in] shape Shape of output image: cube: u,v,pol,chan
   /// @param[in] dopsf Make the psf?
   virtual void initialiseGrid(const scimath::Axes& axes,
                const casa::IPosition& shape, const bool dopsf = true);

   /// @brief grid the visibility data.
   /// @param[in] acc const data accessor to work with
   virtual void grid(IConstDataAccessor& acc);

   /// @brief form the final output image
   /// @param[in] out output double precision image or PSF
   virtual void finaliseGrid(casa::Array<double>& out);

   /// @brief finalise weights
   /// @details Form the sum of the convolution function squared, multiplied by the weights for each
   /// different convolution function. This is used in the evaluation of the second derivative.
   /// @param[in] out output double precision sum of weights images
   virtual void finaliseWeights(casa::Array<double>& out);

   /// @brief initialise the degridding
   /// @param[in] axes axes specifications
   /// @param[in] image input image cube: u,v,pol,chan
   virtual void initialiseDegrid(const scimath::Axes& axes,
					const casa::Array<double>& image);

   /// @brief make context-dependant changes to the gridder behaviour
   /// @param[in] context context description
   virtual void customiseForContext(const std::string &context);
			
   /// @brief set visibility weights
   /// @param[in] viswt shared pointer to visibility weights
   virtual void initVisWeights(const IVisWeights::ShPtr &viswt);

   /// @brief degrid the visibility data.
   /// @param[in] acc non-const data accessor to work with  
   virtual void degrid(IDataAccessor& acc);

   /// @brief finalise degridding
   virtual void finaliseDegrid();

private:
   
   /// @brief gridder doing actual job
   boost::shared_ptr<IVisGridder> itsGridder;
   
   /// @brief adapter dealing with plane fitting
   BestWPlaneDataAccessor itsAccessorAdapter;
};
   
} // namespace synthesis

} // namespace askap

#endif // #ifndef SNAP_SHOT_IMAGING_GRIDDER_ADAPTER_H



