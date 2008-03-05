/// @file
///
/// WStackVisGridder: W projection gridding

/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef ASKAP_SYNTHESIS_WSTACKVISGRIDDER_H_
#define ASKAP_SYNTHESIS_WSTACKVISGRIDDER_H_

#include <gridding/SphFuncVisGridder.h>

namespace askap
{
	namespace synthesis
	{
		/// @brief Visibility gridder using W stacking
		/// @details The visibilities are gridded using a convolution
		/// function of compact support - actually a spheroidal function.
		/// To correct for the w term in the full synthesis measurement equation 
		/// the data are first partitioned in w and then gridded onto separate
		/// planes. At the end, all planes are Fourier transformed and stacked
		/// after multiplication by the w-dependent complex phasor image.
		///
		/// The scaling is fast in data points, slow in w planes.
		///
		/// @ingroup gridding
		class WStackVisGridder : public SphFuncVisGridder
		{
			public:

				/// @brief Construct a gridder for W stacking
				/// @param wmax Maximum baseline (wavelengths)
				/// @param nwplanes Number of w planes
				WStackVisGridder(const double wmax, const int nwplanes);

				virtual ~WStackVisGridder();

				/// @brief Initialise the gridding
				/// @param axes axes specifications
				/// @param shape Shape of output image: u,v,pol,chan
				/// @param dopsf Make the psf?
				virtual void initialiseGrid(const scimath::Axes& axes,
				    const casa::IPosition& shape, const bool dopsf=true);
				
				/// Form the final output image
				/// @param out Output double precision image
				virtual void finaliseGrid(casa::Array<double>& out);

				/// Form the final output image
				/// @param out Output double precision PSF
				virtual void finalisePSF(casa::Array<double>& out);

				/// @brief Initialise the degridding
				/// @param axes axes specifications
				/// @param image Input image: cube: u,v,pol,chan
				virtual void initialiseDegrid(const scimath::Axes& axes,
				    const casa::Array<double>& image);

				/// Clone a copy of this Gridder
				virtual IVisGridder::ShPtr clone();

			protected:
				/// @brief Initialise the indices
				virtual void initIndices(IDataSharedIter& idi);

				/// Offset into grid
				/// @param row Row number
				/// @param pol Polarisation
				/// @param chan Channel number
				virtual int gIndex(int row, int pol, int chan);

				/// Multiply by the phase screen
				/// @param scratch To be multiplied
				/// @param i Index
				void multiply(casa::Array<casa::Complex>& scratch, int i);
				
				/// Scaling
				double itsWScale;
				/// Number of w planes
				int itsNWPlanes;

				/// Mapping from row, pol, and channel to planes of grid
				casa::Cube<int> itsGMap;
		};
	}
}
#endif
