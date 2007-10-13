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
		/// @details The visibilities are gridded using a convolution
		/// function that implements a Fresnel transform. This corrects
		/// for the w term in the full synthesis measurement equation.
		///
		/// The convolution function is calculated straightforwardly
		/// by constructing an image of the complex w-dependent 
		/// phasor and Fourier transforming. The calculation is
		/// done using a coarse but large grid in image space so
		/// that it is sub-sampled in uv space.
		///
		/// The scaling is slow in data points, fast in w planes.
		///
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
				/// @param name Name of table to save convolution function into
				//
				WProjectVisGridder(const double wmax, const int nwplanes,
				    const double cutoff, const int overSample, const int maxSupport,
				    const std::string& name=std::string(""));

				virtual ~WProjectVisGridder();

				/// Clone a copy of this Gridder
				virtual IVisGridder::ShPtr clone();

			protected:
				/// @brief Initialise the indices
				virtual void initIndices(IDataSharedIter& idi);

				/// Offset into convolution function
				/// @param row Row number
				/// @param row Polarisation
				/// @param chan Channel number
				virtual int cIndex(int row, int pol, int chan);

				/// Initialize convolution function
				/// @param idi Data access iterator
				virtual void initConvolutionFunction(IDataSharedIter& idi);

				/// Scaling
				double itsWScale;
				/// Number of w planes
				int itsNWPlanes;
				/// Threshold for cutoff of convolution function
				double itsCutoff;
				/// Mapping from row, pol, and channel to planes of convolution function
				casa::Cube<int> itsCMap;
				/// Maximum support
				int itsMaxSupport;
		};
	}
}
#endif
