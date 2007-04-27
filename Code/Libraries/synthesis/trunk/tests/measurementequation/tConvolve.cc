#include <iostream>
#include <cmath>
#include <ctime>
#include <complex>

using std::cerr;
using std::endl;
using std::complex;

typedef double Coord;
typedef double Real;
typedef std::complex<Real> Value;

// Perform standard data independent gridding
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
	Real C[cSize][cSize];
	for (int i=0;i<cSize;i++) {
		double i2=std::pow(double(i)/double(overSample), 2);
		for (int j=0;j<cSize;j++) {
			double r2=i2+std::pow(double(j)/double(overSample), 2);
			C[i][j]=std::exp(-r2);
		}
	}

	// Grid
	Value grid[gSize][gSize];
	for (int j=0;j<gSize;j++) {
		for (int i=0;i<gSize;i++) {
			grid[j][i]=0.0;
		}
	}

	cerr << "Filling grid" << endl;
	
	clock_t start,finish;
	double time;

	Real sumwt=0.0;
	
	start = clock();
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

			for (int suppu=-support;suppu<+support;suppu++) {
				for (int suppv=-support;suppv<+support;suppv++) {
					Real wt=C[abs(fracu+suppu)][abs(fracv+suppv)];
					grid[iu+suppu][iv+suppv]+=wt*data[i];
					sumwt+=wt;
				}
			}
		}
	}
	finish = clock();
	cerr << "Total weight = " << sumwt << endl;
	time = (double(finish)-double(start))/CLOCKS_PER_SEC;
	cerr << "Time " << time << " (s) " << endl;
	cerr << "Time per visibility sample " << 1e6*time/double(nSamples) 
		<< " (us) " << endl;
	cerr << "Time per visibility spectral sample " 
		<< 1e6*time/double(nSamples*nChan) << " (us) " << endl;
	cerr << "Time per grid-addition " 
		<< 1e9*time/double(nSamples*nChan*(2*support)*(2*support+1)) 
		<< " (ns) " << endl;
	
	return 0;
}

// Perform w projection (data dependent) gridding
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

	// Convolution function
	// In practice, we calculate this by Fourier transformation. Here we
	// take an approximation
	int cSize=(support+1)*overSample;
	
	Real* C;
	C=(Real*)malloc(cSize*cSize*wSize*sizeof(Real));
	
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
	Value grid[gSize][gSize];
	for (int j=0;j<gSize;j++) {
		for (int i=0;i<gSize;i++) {
			grid[j][i]=0.0;
		}
	}

	cerr << "Filling grid" << endl;
	
	clock_t start,finish;
	double time;

	Real sumwt=0.0;
	
	start = clock();
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

			Coord wScaled=freq[chan]*w[i]/wCellSize;		
			int k=wSize/2+int(wScaled);

			for (int suppu=-support;suppu<+support;suppu++) {
				for (int suppv=-support;suppv<+support;suppv++) {
					long int cind=abs(fracu+suppu) + 
						cSize*(abs(fracv+suppv)+cSize*k);
					Real wt=C[cind];
					grid[iu+suppu][iv+suppv]+=wt*data[i];
					sumwt+=wt;
				}
			}
		}
	}
	finish = clock();
	cerr << "Total weight = " << sumwt << endl;
	time = (double(finish)-double(start))/CLOCKS_PER_SEC;
	cerr << "Time " << time << " (s) " << endl;
	cerr << "Time per visibility sample " << 1e6*time/double(nSamples) 
		<< " (us) " << endl;
	cerr << "Time per visibility spectral sample " 
		<< 1e6*time/double(nSamples*nChan) << " (us) " << endl;
	cerr << "Time per grid-addition " 
		<< 1e9*time/double(nSamples*nChan*(2*support)*(2*support+1)) 
		<< " (ns) " << endl;

	return 0;
}
int main() {
	const int baseline=2000;    // Maximum baseline in meters
	const int nSamples=100000;	// Number of data samples
	const int gSize=512;		// Size of output grid in pixels
	const Coord cellSize=50;	// Cellsize of output grid in wavelengths
	const int overSample=100;	// Oversampling factor of gridding function
	const Coord wCellSize=500;	// Cellsize in w in wavelengths
	const int wSize=64;			// Number of lookup planes in w projection
	const int nChan=1;		// Number of spectral channels

	// Initialize the data to be gridded
	Coord *u, *v, *w;
	Value *data;

	u = (Coord*) malloc(nSamples*sizeof(Coord));
	v = (Coord*) malloc(nSamples*sizeof(Coord));
	w = (Coord*) malloc(nSamples*sizeof(Coord));
	data = (Value*) malloc(nSamples*sizeof(Value));
		
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
	cerr << "*** Standard gridding ***" << endl;
	cerr << "Support = " << support << " pixels" << endl;
	standard(u, v, data, nSamples, freq, nChan, cellSize, gSize, 
		support, overSample);

	cerr << "*** W projection gridding ***" << endl;
	int wsupport=3*sqrt(abs(baseline)*cellSize*freq[0])/cellSize; 
		// Support including Fresnel term
	cerr << "Support = " << wsupport << " pixels" << endl;
	cerr << "Predict " << (wsupport/support)*(wsupport/support) 
		<< " times slower than standard" << endl;
	wprojection(u, v, w, data, nSamples, freq, nChan, cellSize, 
		gSize, wsupport, overSample, wCellSize, wSize);
	
	cerr << "Done" << endl;
	return 0;
}