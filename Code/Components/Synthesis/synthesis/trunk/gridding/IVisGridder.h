/// @file
///
/// IVisGridder: Interface definition for visibility gridders
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
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef ASKAP_SYNTHESIS_IVISGRIDDER_H_
#define ASKAP_SYNTHESIS_IVISGRIDDER_H_

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>

#include <fitting/Axes.h>

#include <boost/shared_ptr.hpp>
#include <dataaccess/IDataAccessor.h>
#include <dataaccess/SharedIter.h>

#include <boost/shared_ptr.hpp>

#include <gridding/IVisWeights.h>

namespace askap
{
	namespace synthesis
	{

		/// @brief Abstract Base Class for all gridders.
		/// A gridder puts the synthesis data onto a grid and transforms
		/// as necessary. To allow all the important possibilities, the
		/// Fourier transforms are performed here rather than externally.
		/// 
		/// There is a separate class for degridding.
		/// @ingroup gridding
		class IVisGridder
		{
			public:

			/// Shared pointer definition
			typedef boost::shared_ptr<IVisGridder> ShPtr;

			IVisGridder();
			virtual ~IVisGridder();
			
			/// Clone a copy of this Gridder
			virtual ShPtr clone() = 0;

			/// @brief Initialise the gridding
			/// @param axes axes specifications
			/// @param shape Shape of output image: cube: u,v,pol,chan
			/// @param dopsf Make the psf?
			virtual void initialiseGrid(const scimath::Axes& axes,
					const casa::IPosition& shape, const bool dopsf=true) = 0;

            /// @brief Grid the visibility data.
            /// @param acc const data accessor to work with
            virtual void grid(IConstDataAccessor& acc) = 0;

			/// Form the final output image
			/// @param out Output double precision image or PSF
			virtual void finaliseGrid(casa::Array<double>& out) = 0;

			/// Form the sum of the convolution function squared, multiplied by the weights for each
			/// different convolution function. This is used in the evaluation of the second derivative.
			/// @param out Output double precision sum of weights images
			virtual void finaliseWeights(casa::Array<double>& out) = 0;

			/// @brief Initialise the degridding
			/// @param axes axes specifications
			/// @param image Input image: cube: u,v,pol,chan
			virtual void initialiseDegrid(const scimath::Axes& axes,
					const casa::Array<double>& image) = 0;

			/// @brief Make context-dependant changes to the gridder behaviour
			/// @param[in] context context description
			virtual void customiseForContext(casa::String context) = 0;
			
			/// @brief set visibility weights
			/// @param[in] viswt shared pointer to visibility weights
			virtual void initVisWeights(IVisWeights::ShPtr viswt) = 0;

            /// @brief Degrid the visibility data.
            /// @param[in] acc non-const data accessor to work with  
            virtual void degrid(IDataAccessor& acc) = 0;

			/// @brief Finalise
			virtual void finaliseDegrid() = 0;

		};
	}
}
#endif                                            /*IVISGRIDDER_H_*/
