#include <iostream>
#include <cmath>
#include <ctime>
#include <complex>

using std::cout;
using std::endl;
using std::complex;

// Typedefs for easy testing
typedef double Coord;
typedef float Real;
typedef std::complex<Real> Value;

// Perform standard data independent gridding
//
// u,v,w - components of spatial frequency
// data - values to be gridded
// nSamples - number of visibility samples
// freq - temporal frequency (inverse wavelengths)
// cellSize - size of one grid cell in wavelengths
// support - Total width of convolution function=2*support+1
// overSample - Oversampling factor for the convolution function
// 
int standard(const Coord* u, const Coord* v, const Value* data,
					const int nSamples, 
					const Coord* freq, const int nChan,
					const Coord cellSize,
					const int gSize, 
					const int support,
					const int overSample) {

		
	// Convolution function
	// We take this to be the product of two Gaussian. More often it 
	// is the product of two prolate spheroidal wave functions
	int cSize=(support+1)*overSample;

	Real* C=new Real[cSize*cSize];
	
	for (int i=0;i<cSize;i++) {
		double i2=std::pow(double(i)/double(overSample), 2);
		for (int j=0;j<cSize;j++) {
			double r2=i2+std::pow(double(j)/double(overSample), 2);
			C[i+cSize*j]=std::exp(-r2);
		}
	}

	// Grid
	Value* grid=new Value[gSize*gSize];
	for (int j=0;j<gSize*gSize;j++) {
		grid[j]=0.0;
	}

	cout << "Filling grid" << endl;
	
	clock_t start,finish;
	double time;

	Real sumwt=0.0;
	
	start = clock();
	// Loop over all samples adding them to the grid
	// First scale to the correct pixel location
	// Then find the fraction of a pixel to the nearest pixel
	// Loop over the entire support, calculating weights from
	// the convolution function and adding the scaled
	// visibility to the grid.
	for (int i=0;i<nSamples;i++) {
		for (int chan=0;chan<nChan;chan++) {
			
			Coord uScaled=freq[chan]*u[i]/cellSize;
			int iu=int(uScaled);
			int fracu=int(overSample*(uScaled-Coord(iu)));
			iu+=gSize/2;

			Coord vScaled=freq[chan]*v[i]/cellSize;
			int iv=int(vScaled);
			int fracv=int(overSample*(vScaled-Coord(iv)));
			iv+=gSize/2;

			if((iu<support)||(iu>gSize-support)||(iv<support)||(iv>gSize-support)){
				if(chan==0) {
					cout << "Row " << i << " is off grid - change scaling" << endl;
				}
			}
			else {
				for (int suppu=-support;suppu<+support;suppu++) {
					for (int suppv=-support;suppv<+support;suppv++) {
						Real wt=C[abs(fracu+suppu)+cSize*abs(fracv+suppv)];
						grid[(iu+suppu)+gSize*(iv+suppv)]+=wt*data[i];
						sumwt+=wt;
					}	
				}	
			}
		}
	}
	finish = clock();
	cout << "Total weight = " << sumwt << endl;
	time = (double(finish)-double(start))/CLOCKS_PER_SEC;
	cout << "Time " << time << " (s) " << endl;
	cout << "Time per visibility sample " << 1e6*time/double(nSamples) 
		<< " (us) " << endl;
	cout << "Time per visibility spectral sample " 
		<< 1e6*time/double(nSamples*nChan) << " (us) " << endl;
	cout << "Time per grid-addition " 
		<< 1e9*time/double(nSamples*nChan*(2*support)*(2*support+1)) 
		<< " (ns) " << endl;
	
	delete[] C;
	delete[] grid;
	
	return 0;
}

// Perform w projection (data dependent) gridding
//
// u,v,w - components of spatial frequency
// data - values to be gridded
// nSamples - number of visibility samples
// freq - temporal frequency (inverse wavelengths)
// cellSize - size of one grid cell in wavelengths
// gSize - size of grid in pixels (per axis)
// support - Total width of convolution function=2*support+1
// overSample - Oversampling factor for the convolution function
// wCellSize - size of one w grid cell in wavelengths
// wSize - Size of lookup table in w 
int wprojection(const Coord* u, const Coord* v, const Coord* w, 
					const Value* data,
					const int nSamples, 
					const Coord* freq, const int nChan,
					const Coord cellSize,
					const int gSize, 
					const int support,
					const int overSample,
					const Coord wCellSize,
					const int wSize) {

	// Convolution function. This should be the convolution of the
	// w projection kernel (the Fresnel term) with the convolution
	// function used in the standard case. The latter is needed to
	// suppress aliasing. In practice, we calculate entire function
	// by Fourier transformation. Here we take an approximation that
	// is good enough.
	int cSize=(support+1)*overSample;
	
	Real* C=new Real[cSize*cSize*wSize];
	
	for (int k=0;k<wSize;k++) {
		if(k!=wSize/2) {
			double w=double(k-wSize/2);
			double fScale=sqrt(abs(w)*wCellSize*freq[0])/cellSize;
			for (int j=0;j<cSize;j++) {
				double j2=std::pow(double(j)/double(overSample), 2);
				for (int i=0;i<cSize;i++) {
					double r2=j2+std::pow(double(i)/double(overSample), 2);
					long int cind=i+cSize*(j+cSize*k);
					C[cind]=std::cos(r2/(w*fScale));
				}
			}
		}
		else {
			for (int j=0;j<cSize;j++) {
				double j2=std::pow(double(j)/double(overSample), 2);
				for (int i=0;i<cSize;i++) {
					double r2=j2+std::pow(double(i)/double(overSample), 2);
					long int cind=i+cSize*(j+cSize*k);
					C[cind]=std::exp(-r2);
				}
			}
		}
	}
		
	// Grid
	Value* grid=new Value[gSize*gSize];
	for (int j=0;j<gSize*gSize;j++) {
		grid[j]=0.0;
	}

	cout << "Filling grid" << endl;
	
	clock_t start,finish;
	double time;

	Real sumwt=0.0;
	
	start = clock();
	// Loop over all samples adding them to the grid
	// First scale to the correct pixel location
	// Then find the fraction of a pixel to the nearest pixel
	// Loop over the entire support, calculating weights from
	// the convolution function and adding the scaled
	// visibility to the grid.
	for (int i=0;i<nSamples;i++) {
		for (int chan=0;chan<nChan;chan++) {

			Coord wScaled=freq[chan]*w[i]/wCellSize;		
			int k=wSize/2+int(wScaled);

			Coord uScaled=freq[chan]*u[i]/cellSize;
			int iu=int(uScaled);
			int fracu=int(overSample*(uScaled-Coord(iu)));
			iu+=gSize/2;

			Coord vScaled=freq[chan]*v[i]/cellSize;
			int iv=int(vScaled);
			int fracv=int(overSample*(vScaled-Coord(iv)));
			iv+=gSize/2;

			if((iu<support)||(iu>gSize-support)||(iv<support)||(iv>gSize-support)||
				(k<0)||(k>=wSize)){
				if(chan==0) {
					cout << "Row " << i << " is off grid - change scaling" << endl;
				}
			}
			else {
				for (int suppu=-support;suppu<+support;suppu++) {
					for (int suppv=-support;suppv<+support;suppv++) {
						long int cind=abs(fracu+suppu) + 
							cSize*(abs(fracv+suppv)+cSize*k);
						Real wt=C[cind];
						grid[(iu+suppu)+gSize*(iv+suppv)]+=wt*data[i];
						sumwt+=wt;
					}
				}					
			}
		}
	}
	finish = clock();
	// Report on timings
	cout << "Total weight = " << sumwt << endl;
	time = (double(finish)-double(start))/CLOCKS_PER_SEC;
	cout << "Time " << time << " (s) " << endl;
	cout << "Time per visibility sample " << 1e6*time/double(nSamples) 
		<< " (us) " << endl;
	cout << "Time per visibility spectral sample " 
		<< 1e6*time/double(nSamples*nChan) << " (us) " << endl;
	cout << "Time per grid-addition " 
		<< 1e9*time/double(nSamples*nChan*(2*support)*(2*support+1)) 
		<< " (ns) " << endl;

	delete[] C;
	delete[] grid;
	
	return 0;
}
int main() {
	const int baseline=2000;    // Maximum baseline in meters
	const int nSamples=10000;	// Number of data samples
	const int gSize=512;		// Size of output grid in pixels
	const Coord cellSize=50;	// Cellsize of output grid in wavelengths
	const int overSample=100;	// Oversampling factor of gridding function
	const Coord wCellSize=500;	// Cellsize in w in wavelengths
	const int wSize=64;		// Number of lookup planes in w projection
	const int nChan=16;		// Number of spectral channels

	// Initialize the data to be gridded
	Coord *u, *v, *w;
	Value *data;

	u = new Coord[nSamples];
	v = new Coord[nSamples];
	w = new Coord[nSamples];
	data = new Value[nSamples];
		
	for (int i=0;i<nSamples;i++) {
		u[i]=baseline*Coord(rand())/Coord(RAND_MAX)-baseline/2;
		v[i]=baseline*Coord(rand())/Coord(RAND_MAX)-baseline/2;
		w[i]=baseline*Coord(rand())/Coord(RAND_MAX)-baseline/2;
		data[i]=Coord(rand())/Coord(RAND_MAX);
	}

	// Measure frequency in inverse wavelengths
	Coord freq[nChan];
	for (int i=0;i<nChan;i++) {
		freq[i]=(1.4e9-2.0e5*Coord(i)/Coord(nChan))/2.998e8;
	}

	int support=3;		// Support for gridding function in pixels
	cout << "*** Standard gridding ***" << endl;
	cout << "Support = " << support << " pixels" << endl;
	standard(u, v, data, nSamples, freq, nChan, cellSize, gSize, 
		support, overSample);

	cout << "*** W projection gridding ***" << endl;
	int wsupport=3*sqrt(abs(baseline)*cellSize*freq[0])/cellSize; 
	int wOverSample=8;
		// Support including Fresnel term
	cout << "Support = " << wsupport << " pixels" << endl;
	wprojection(u, v, w, data, nSamples, freq, nChan, cellSize, 
		gSize, wsupport, wOverSample, wCellSize, wSize);
	
	cout << "Done" << endl;
	
	delete[] u,v,w,data;
	return 0;
}