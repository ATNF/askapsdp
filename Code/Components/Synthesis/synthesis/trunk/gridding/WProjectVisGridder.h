/// @file
///
/// WProjectVisGridder: W projection gridding

/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef CONRAD_SYNTHESIS_WPROJECTVISGRIDDER_H_
#define CONRAD_SYNTHESIS_WPROJECTVISGRIDDER_H_

#include <gridding/SphFuncVisGridder.h>

namespace conrad
{
	namespace synthesis
	{
		/// @brief Visibility gridder using W projection
		/// @ingroup gridding
		class WProjectVisGridder : public SphFuncVisGridder
		{
			public:

				/// @brief Construct a gridder for W projection
				/// @param wmax Maximum baseline (wavelengths)
				/// @param nwplanes Number of w planes
				/// @param cutoff Cutoff in determining support e.g. 10^-3 of the peak
				/// @param overSample Oversampling (currently limited to <=1)
				/// @param maxSupport Maximum support to be allowed
				//
				WProjectVisGridder(const double wmax, const int nwplanes,
				    const double cutoff, const int overSample, const int maxSupport);

				virtual ~WProjectVisGridder();

				/// Form the sum of the convolution function squared, multiplied by the weights for each
				/// different convolution function. This is used in the evaluation of the second derivative.
				/// @param sumwt Sum of weights (offset, pol)
				/// @param out Output double precision grid
				virtual void finaliseReverseWeights(casa::Matrix<double>& sumWeights,
				    casa::Cube<double>& out);

			protected:
				/// Offset into convolution function
				/// @param row Row number
				/// @param chan Channel number
				virtual int cOffset(int row, int chan);
				/// Initialize convolution function
				/// @param idi Data access iterator
				/// @param uvw Input uvw (may be rotated so we cannot use the iterator version)
				/// @param cellSize Cell size in wavelengths
				/// @param shape Shape of grid
				virtual void initConvolutionFunction(IDataSharedIter& idi,
				    const conrad::scimath::Axes& axes,
				    casa::Vector<casa::RigidVector<double, 3> >& uvw,
				    const casa::Vector<double>& cellSize, const casa::IPosition& shape);
				/// Scaling
				double itsWScale;
				/// Number of w planes
				int itsNWPlanes;
				/// Threshold for cutoff of convolution function
				double itsCutoff;
				/// Mapping from row and channel to planes
				casa::Matrix<int> itsCMap;
				/// Maximum support
				int itsMaxSupport;
		};
	}
}
#endif
