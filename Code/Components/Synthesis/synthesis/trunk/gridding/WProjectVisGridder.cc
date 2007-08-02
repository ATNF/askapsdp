#include <gridding/WProjectVisGridder.h>

#include <conrad/ConradError.h>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>

#include <casa/BasicSL/Constants.h>
#include <scimath/Mathematics/FFTServer.h>

using namespace conrad;

#include <cmath>

namespace conrad
{
namespace synthesis
{

WProjectVisGridder::WProjectVisGridder(const double wmax, const int nwplanes,
		const double cutoff, const int overSample, const int maxSupport)
{
	CONRADCHECK(wmax>0.0, "Baseline length must be greater than zero");
	CONRADCHECK(nwplanes>0, "Number of w planes must be greater than zero");
	CONRADCHECK(overSample>0, "Oversampling must be greater than 0");
	CONRADCHECK(cutoff>0.0, "Cutoff must be positive");
	CONRADCHECK(cutoff<1.0, "Cutoff must be less than 1.0");
	CONRADCHECK(maxSupport>0, "Maximum support must be greater than 0")
	itsSupport=0;
	itsNWPlanes=2*nwplanes+1;
	itsWScale=wmax/double(nwplanes);
	itsOverSample=overSample;
	itsCutoff=cutoff;
	itsMaxSupport=maxSupport;
}

WProjectVisGridder::~WProjectVisGridder()
{
}

/// Initialize the convolution function into the cube. If necessary this
/// could be optimized by using symmetries.
void WProjectVisGridder::initConvolutionFunction(IDataSharedIter& idi,
		const conrad::scimath::Axes& axes,
		casa::Vector<casa::RigidVector<double, 3> >& uvw,
		const casa::Vector<double>& cellSize, const casa::IPosition& shape)
{
	/// We have to calculate the lookup function converting from
	/// row and channel to plane of the w-dependent convolution
	/// function
	const int nSamples = uvw.size();
	const int nChan = idi->frequency().size();
	itsCMap.resize(nSamples, nChan);
	itsCMap.set(0);

	int cenw=(itsNWPlanes-1)/2;
	for (int i=0; i<nSamples; i++)
	{
		double w=(uvw(i)(2))/(casa::C::c);
		for (int chan=0; chan<nChan; chan++)
		{
			double freq=idi->frequency()[chan];
			itsCMap(i, chan)=cenw+nint(w*freq/itsWScale);
		}
	}
	if (itsSupport!=0)
		return;

	casa::FFTServer<casa::Float,casa::Complex> ffts;
	itsSupport=0;

	/// Limit the size of the convolution function since
	/// we don't need it finely sampled in image space. This
	/// will reduce the time taken to calculate it.
	int nx=std::min(itsMaxSupport, shape(0)/itsOverSample);
	int ny=std::min(itsMaxSupport, shape(1)/itsOverSample);
	int cenx=nx/2;
	int ceny=ny/2;

	casa::Vector<float> ccfx(nx);
	casa::Vector<float> ccfy(ny);
	for (int ix=0; ix<nx; ix++)
	{
		float nux=std::abs(float(ix-cenx))/float(nx/2);
		ccfx(ix)=grdsf(nux)/float(nx);
	}
	for (int iy=0; iy<ny; iy++)
	{
		float nuy=std::abs(float(iy-ceny))/float(ny/2);
		ccfy(iy)=grdsf(nuy)/float(ny);
	}

	// Now we step through the w planes, starting the furthest
	// out. We calculate the support for that plane and use it
	// for all the others.
	// We pad here to do sinc interpolation of the convolution
	// function in uv space
	casa::Matrix<casa::Complex> thisPlane(nx*itsOverSample, ny*itsOverSample);

	float cellx=1.0/(float(nx)*cellSize(0));
	float celly=1.0/(float(ny)*cellSize(1));

	for (int iw=0; iw<=cenw; iw++)
	{

		thisPlane.set(0.0);

		float w=2.0f*casa::C::pi*float(iw-cenw)*itsWScale;
		for (int iy=ceny-ny/2; iy<ceny+ny/2; iy++)
		{
			float y2=float(iy-ceny)*celly;
			y2*=y2;
			for (int ix=cenx-nx/2; ix<cenx+nx/2; ix++)
			{
				float x2=float(ix-cenx)*cellx;
				x2*=x2;
				float r2=x2+y2;
				float phase=w*(1.0-sqrt(1.0-r2));
				float wt=ccfx(iy-ceny+ny/2)*ccfy(ix-cenx+nx/2);
				thisPlane(ix, iy)=casa::Complex(wt*cos(phase), -wt*sin(phase));
			}
		}
		// Now we have to calculate the Fourier transform to get the
		// convolution function in uv space
		// @todo Replace ffts by smarter versions - real/complex and FFTW
		for (int iy=0; iy<itsOverSample*ny; iy++)
		{
			casa::Array<casa::Complex> vec(thisPlane.column(iy));
			ffts.fft(vec, true);
		}
		for (int ix=0; ix<itsOverSample*nx; ix++)
		{
			casa::Array<casa::Complex> vec(thisPlane.row(ix));
			ffts.fft(vec, true);
		}
		// If the support is not yet set, find it and size the
		// convolution function appropriately
		if (itsSupport==0)
		{
			// Find the support by starting from the edge and
			// working in
			for (int ix=0; ix<itsOverSample*cenx; ix++)
			{
				if (abs(thisPlane(ix, itsOverSample*ceny))>itsCutoff)
				{
					itsSupport=abs(ix-itsOverSample*cenx)/itsOverSample;
					break;
				}
			}
			itsSupport=(itsSupport<nx/2) ? itsSupport : nx/2;
			itsCSize=2*(itsSupport+1)*itsOverSample;
			std::cout << "W support = "<< itsSupport
					<< " pixels, convolution function size = "<< itsCSize
					<< " pixels"<< std::endl;
			itsCCenter=itsCSize/2-1;
			itsC.resize(itsCSize, itsCSize, itsNWPlanes);
			itsC.set(0.0);
		}
		// Now cut out the inner part of the convolution function
		for (int iy=-itsSupport*itsOverSample; iy<+itsOverSample*itsSupport; iy++)
		{
			for (int ix=-itsOverSample*itsSupport; ix<+itsOverSample*itsSupport; ix++)
			{
				itsC(ix+itsCCenter, iy+itsCCenter, iw)=thisPlane(ix
						+itsOverSample*cenx, iy+itsOverSample*ceny);
			}
		}
		itsC.xyPlane(itsNWPlanes-1-iw)=casa::conj(itsC.xyPlane(iw));
	}
	std::cout << "Shape of convolution function = "<< itsC.shape() << std::endl;
}

void WProjectVisGridder::finaliseReverseWeights(casa::Matrix<double>& sumWeights,
		casa::Cube<double>& out)
{
	casa::FFTServer<casa::Float,casa::Complex> ffts;

	int nx=out.shape()(0);
	int ny=out.shape()(1);
	int nPol=out.shape()(2);

	int cenx=nx/2;
	int ceny=ny/2;
	/// We must pad the convolution function to full size, reverse transform
	/// square, and sum multiplied by the corresponding weight
	casa::Matrix<casa::Complex> thisPlane(nx*itsOverSample, ny*itsOverSample);

	int nZ=sumWeights.shape()(0);

	CONRADCHECK(sumWeights.shape()(1)!=nPol, "Number of polarizations do not match");

	out.set(0.0);

	for (int iz=0; iz<nZ; iz++)
	{
		thisPlane.set(0.0);

		// Now fill the inner part of the uv plane with the convolution function
		// and transform to obtain the full size image
		for (int iy=-itsSupport*itsOverSample; iy<+itsOverSample*itsSupport; iy++)
		{
			for (int ix=-itsOverSample*itsSupport; ix<+itsOverSample*itsSupport; ix++)
			{
				thisPlane(ix+itsOverSample*cenx, iy+itsOverSample*ceny)=itsC(ix
						+itsCCenter, iy+itsCCenter, iz);
			}
		}

		// Now we have to calculate the Fourier transform to get the
		// convolution function in uv space
		// @todo Replace ffts by smarter versions - real/complex and FFTW
		for (int iy=0; iy<itsOverSample*ny; iy++)
		{
			casa::Array<casa::Complex> vec(thisPlane.column(iy));
			ffts.fft(vec, false);
		}
		for (int ix=0; ix<itsOverSample*nx; ix++)
		{
			casa::Array<casa::Complex> vec(thisPlane.row(ix));
			ffts.fft(vec, false);
		}
		for (int pol=0; pol<nPol; pol++)
		{
			double weight=sumWeights(iz, pol);
			for (int ix=0;ix<nx;ix++) {
				for (int iy=0;iy<ny;iy++) {
					out(ix,iy,pol)=weight*real(thisPlane(ix,iy)*conj(thisPlane(ix,iy)));
				}
			}
		}
	}

}

int WProjectVisGridder::cOffset(int row, int chan)
{
	return itsCMap(row, chan);
}

}
}
