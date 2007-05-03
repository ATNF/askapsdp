//   This C++ program has been written to demonstrate the 
// convolutional resampling algorithm used in radio
// interferometry. It should compile with:
// 	g++ -O2 tConvolve.cc -o tConvolve
// Compiler option -O3 will optimize away the gridding!
//
// The challenge is to minimize the run time - specifically
// the time per grid addition. On a MacBookPro and an Opteron
// this is about 12ns.
//
//   For further details contact Tim.Cornwell@csiro.au
// May 3, 2007

#include <iostream>
#include <cmath>
#include <ctime>
#include <complex>
#include <vector>
#include <algorithm>

using std::cout;
using std::endl;
using std::complex;

// Typedefs for easy testing
// Cost of using double for Coord is low, cost for 
// double for Real is also low
typedef double Coord;
typedef double Real;
typedef std::complex<Real> Value;

// Perform standard data independent gridding
//
// u,v,w - components of spatial frequency
// data - values to be gridded
// freq - temporal frequency (inverse wavelengths)
// cellSize - size of one grid cell in wavelengths
// C - convolution function
// support - Total width of convolution function=2*support+1
// overSample - Oversampling factor for the convolution function
// cOffset - offsets into convolution function per data point
// grid - Output grid
// 

int generic(const std::vector<Coord>& u, 
					const std::vector<Coord>& v, 
					const std::vector<Coord>& w, 
					std::vector<Value>& data,
					const std::vector<Coord>& freq,
					const Coord cellSize,
					const std::vector<Real>& C,
					const int support,
					const int overSample,
					const std::vector<uint>& cOffset,
					std::vector<Value>& grid)
{

	const int gSize = static_cast<int>(std::sqrt(static_cast<float>(grid.size())));
	const int nSamples = u.size();
	const int nChan = freq.size();
	 

	int cSize=2*(support+1)*overSample+1;

	int cCenter=(cSize-1)/2;
	
	// Grid
	grid.assign(grid.size(), Value(0.0));


	cout << "+++++ Forward processing +++++" << endl;
	
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

			int find=i*nChan+chan;

			int coff=cOffset[find];
			
			Coord uScaled=freq[chan]*u[i]/cellSize;
			int iu=int(uScaled);
			int fracu=int(overSample*(uScaled-Coord(iu)));
			iu+=gSize/2;

			Coord vScaled=freq[chan]*v[i]/cellSize;
			int iv=int(vScaled);
			int fracv=int(overSample*(vScaled-Coord(iv)));
			iv+=gSize/2;

			for (int suppv=-support;suppv<+support;suppv++) {
				int vind=cSize*(fracv+suppv+cCenter)+fracu+cCenter+coff;
				int gind=iu+gSize*(iv+suppv);
				for (int suppu=-support;suppu<+support;suppu++) {
					Real wt=C[vind+suppu];
					grid[gind+suppu]+=wt*data[find];
					sumwt+=wt;
				}					
			}
		}
	}
	finish = clock();
	// Report on timings
	cout << "    Total weight = " << sumwt << endl;
	time = (double(finish)-double(start))/CLOCKS_PER_SEC;
	cout << "    Time " << time << " (s) " << endl;
	cout << "    Time per visibility sample " << 1e6*time/double(nSamples) 
		<< " (us) " << endl;
	cout << "    Time per visibility spectral sample " 
		<< 1e6*time/double(nSamples*nChan) << " (us) " << endl;
	cout << "    Time per grid-addition " 
		<< 1e9*time/(double(nSamples)*double(nChan)*double((2*support)*(2*support+1)))
		<< " (ns) " << endl;
		
	cout << "+++++ Reverse processing +++++" << endl;
	
	// Just run the gridding in reverse
	start = clock();
	for (int i=0;i<nSamples;i++) {
		for (int chan=0;chan<nChan;chan++) {
			
			double sumviswt=0.0;

			int find=i*nChan+chan;

			int coff=cOffset[find];

			Coord uScaled=freq[chan]*u[i]/cellSize;
			int iu=int(uScaled);
			int fracu=int(overSample*(uScaled-Coord(iu)));
			iu+=gSize/2;

			Coord vScaled=freq[chan]*v[i]/cellSize;
			int iv=int(vScaled);
			int fracv=int(overSample*(vScaled-Coord(iv)));
			iv+=gSize/2;

			for (int suppv=-support;suppv<+support;suppv++) {
				int vind=cSize*(fracv+suppv+cCenter)+fracu+cCenter+coff;
				int gind=iu+gSize*(iv+suppv);
				for (int suppu=-support;suppu<+support;suppu++) {
					Real wt=C[vind+suppu];
					data[find]=data[find]+wt*grid[gind+suppu];
					sumviswt+=wt;
				}
			}
			data[find]=data[find]/sumviswt;
		}
	}
	finish = clock();
	// Report on timings
	time = (double(finish)-double(start))/CLOCKS_PER_SEC;
	cout << "    Time " << time << " (s) " << endl;
	cout << "    Time per visibility sample " << 1e6*time/double(nSamples) 
		<< " (us) " << endl;
	cout << "    Time per visibility spectral sample " 
		<< 1e6*time/double(nSamples*nChan) << " (us) " << endl;
	cout << "    Time per grid-addition " 
		<< 1e9*time/(double(nSamples)*double(nChan)*double((2*support)*(2*support+1)))
		<< " (ns) " << endl;

	return 0;
}

int standard(const std::vector<Coord>& u, const std::vector<Coord>& v, const std::vector<Coord>& w, 
					std::vector<Value>& data,
					const std::vector<Coord>& freq,
					const Coord cellSize,
					std::vector<Value>& grid) {

		
	cout << "*************************** Standard gridding ***********************" << endl;
	int support=3;		// Support for gridding function in pixels
	const int overSample=100;
	cout << "Support = " << support << " pixels" << endl;

	// Convolution function
	// We take this to be the product of two Gaussian. More often it 
	// is the product of two prolate spheroidal wave functions
	int cSize=2*(support+1)*overSample+1;

	std::vector<Real> C(cSize*cSize);
	
	int cCenter=(cSize-1)/2;
	
	// Keep this symmetrically to streamline index handling later....
	for (int i=0;i<cSize;i++) {
		double i2=std::pow(double(i-cCenter)/double(overSample), 2);
		for (int j=0;j<cSize;j++) {
			double r2=i2+std::pow(double(j-cCenter)/double(overSample), 2);
			C[i+cSize*j]=std::exp(-r2);
		}
	}

	std::vector<uint> cOffset;
	cOffset.assign(data.size(),0);
	
	return generic(u, v, w, data, freq, cellSize, C, support, overSample, cOffset, grid); 
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
// wCellSize - size of one w grid cell in wavelengths
// wSize - Size of lookup table in w 
int wprojection(const std::vector<Coord>& u, 
					const std::vector<Coord>& v, 
					const std::vector<Coord>& w, 
					std::vector<Value>& data,
					const std::vector<Coord>& freq,
					const Coord cellSize,
					const int wSize,
					std::vector<Value>& grid) {

	const int nSamples = u.size();
	const int nChan = freq.size();

	cout << "************************* W projection gridding *********************" << endl;
	const Coord baseline=*std::max_element(w.begin(), w.end());
	int support=static_cast<int>(3.0*sqrt(abs(baseline)*static_cast<Coord>(cellSize)*freq[0])/cellSize); 
	int overSample=8;
	cout << "Support = " << support << " pixels" << endl;
	const Coord wCellSize=2*baseline*freq[0]/wSize;
	cout << "W cellsize = " << wCellSize << " wavelengths" << endl;

	// Convolution function. This should be the convolution of the
	// w projection kernel (the Fresnel term) with the convolution
	// function used in the standard case. The latter is needed to
	// suppress aliasing. In practice, we calculate entire function
	// by Fourier transformation. Here we take an approximation that
	// is good enough.
	int cSize=2*(support+1)*overSample+1;

	int cCenter=(cSize-1)/2;
	
	std::vector<Real> C(cSize*cSize*wSize);
	
	for (int k=0;k<wSize;k++) {
		if(k!=wSize/2) {
			double w=double(k-wSize/2);
			double fScale=sqrt(abs(w)*wCellSize*freq[0])/cellSize;
			for (int j=0;j<cSize;j++) {
				double j2=std::pow(double(j-cCenter)/double(overSample), 2);
				for (int i=0;i<cSize;i++) {
					double r2=j2+std::pow(double(i-cCenter)/double(overSample), 2);
					long int cind=i+cSize*(j+cSize*k);
					C[cind]=static_cast<Real>(std::cos(r2/(w*fScale)));
				}
			}
		}
		else {
			for (int j=0;j<cSize;j++) {
				double j2=std::pow(double(j-cCenter)/double(overSample), 2);
				for (int i=0;i<cSize;i++) {
					double r2=j2+std::pow(double(i-cCenter)/double(overSample), 2);
					long int cind=i+cCenter+cSize*(j+cCenter+cSize*k);
					C[cind]=static_cast<Real>(std::exp(-r2));
				}
			}
		}
	}

	std::vector<uint> cOffset(data.size());
	for (int i=0;i<nSamples;i++) {
		for (int chan=0;chan<nChan;chan++) {

			int find=i*nChan+chan;

			Coord wScaled=freq[chan]*w[i]/wCellSize;		
			cOffset[find]=wSize/2+int(wScaled);
		}
	}
		
	return generic(u, v, w, data, freq, cellSize, C, support, overSample, cOffset, grid);
}

int main() {
	const int baseline=2000;    // Maximum baseline in meters
	const int nSamples=100000;	// Number of data samples
	const int gSize=512;		// Size of output grid in pixels
	const Coord cellSize=50;	// Cellsize of output grid in wavelengths
	const int wSize=64;		// Number of lookup planes in w projection
	const int nChan=16;		// Number of spectral channels
	
	// Initialize the data to be gridded
	std::vector<Coord> u(nSamples);
	std::vector<Coord> v(nSamples);
	std::vector<Coord> w(nSamples);
	std::vector<Value> data(nSamples*nChan);
		
	for (int i=0;i<nSamples;i++) {
		u[i]=baseline*Coord(rand())/Coord(RAND_MAX)-baseline/2;
		v[i]=baseline*Coord(rand())/Coord(RAND_MAX)-baseline/2;
		w[i]=baseline*Coord(rand())/Coord(RAND_MAX)-baseline/2;
		for (int chan=0;chan<nChan;chan++) {
			data[i*nChan+chan]=Coord(rand())/Coord(RAND_MAX);
		}
	}

	// Measure frequency in inverse wavelengths
	std::vector<Coord> freq(nChan);
	for (int i=0;i<nChan;i++) {
		freq[i]=(1.4e9-2.0e5*Coord(i)/Coord(nChan))/2.998e8;
	}

	std::vector<Value> grid(gSize*gSize);
	
	standard(u, v, w, data, freq, cellSize, grid);

	wprojection(u, v, w, data, freq, cellSize, wSize, grid);
	
	cout << "Done" << endl;
	
	return 0;
}
