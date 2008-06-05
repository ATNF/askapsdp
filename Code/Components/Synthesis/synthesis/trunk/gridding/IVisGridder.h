/// @file
///
/// IVisGridder: Interface definition for visibility gridders
///
/// (c) 2007 ASKAP, All Rights Reserved.
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
#include <dataaccess/IDataIterator.h>
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

			/// Grid the visibility data.
			/// @param idi DataIterator
			virtual void grid(IDataSharedIter& idi) = 0;

			/// Form the final output image
			/// @param out Output double precision image
			virtual void finaliseGrid(casa::Array<double>& out) = 0;

			/// Form the final output image
			/// @param out Output double precision PSF
			virtual void finalisePSF(casa::Array<double>& out) = 0;

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

			/// Degrid the visibility data.
			/// @param idi DataIterator
			virtual void degrid(IDataSharedIter& idi) = 0;

			/// @brief Finalise
			virtual void finaliseDegrid() = 0;

		};
	}
}
#endif                                            /*IVISGRIDDER_H_*/
