//   This C++ program has been written to demonstrate the
// convolutional resampling algorithm used in radio
// interferometry. It should compile with:
//      g++ -O2 tConvolveBLAS.cc -o tConvolveBLAS
// Enabling BLAS support on OS X:
//      g++ -DUSEBLAS -O2 -framework vecLib tConvolveBLAS.cc -o tConvolveBLAS
//
// The challenge is to minimize the run time - specifically
// the time per grid addition. On a MacBookPro 2GHz Intel Core Duo
// this is about 6.6ns
//
//   For further details contact Tim.Cornwell@csiro.au
// November 22, 2007

#include <iostream>
#include <cmath>
#include <ctime>
#include <complex>
#include <vector>
#include <algorithm>

#ifdef USEBLAS

#ifdef __APPLE_CC__
#include <vecLib/cblas.h>
#endif

#endif

using std::cout;
using std::endl;
using std::complex;
using std::abs;

// Typedefs for easy testing
// Cost of using double for Coord is low, cost for
// double for Real is also low
typedef float Coord;
typedef float Real;
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

int generic(const std::vector<Coord>& u, const std::vector<Coord>& v,
    const std::vector<Coord>& w, const std::vector<Value>& data,
    std::vector<Value>& outdata, const std::vector<Coord>& freq,
    const Coord cellSize, const std::vector<Value>& C, const int support,
    const int overSample, const std::vector<unsigned int>& cOffset,
    std::vector<Value>& grid, const int gSize)
{

  const int nSamples = u.size();
  const int nChan = freq.size();

  int sSize=2*(support+1);

  int cCenter=sSize/2;

  cout << "+++++ Forward processing +++++" << endl;

  // Grid
  grid.assign(grid.size(), Value(0.0));

  clock_t start, finish;
  double time;

  start = clock();
  // Loop over all samples adding them to the grid
  // First scale to the correct pixel location
  // Then find the fraction of a pixel to the nearest pixel
  // Loop over the entire support, calculating weights from
  // the convolution function and adding the scaled
  // visibility to the grid.
  for (int i=0; i<nSamples; i++)
  {
    for (int chan=0; chan<nChan; chan++)
    {

      int find=i*nChan+chan;

      int coff=cOffset[find];

      Coord uScaled=freq[chan]*u[i]/cellSize;
      int iu=int(uScaled);
      if (uScaled<Coord(iu))
      {
        iu-=1;
      }
      int fracu=int(overSample*(uScaled-Coord(iu)));
      iu+=gSize/2;

      Coord vScaled=freq[chan]*v[i]/cellSize;
      int iv=int(vScaled);
      if (vScaled<Coord(iv))
      {
        iv-=1;
      }
      int fracv=int(overSample*(vScaled-Coord(iv)));
      iv+=gSize/2;

      // The beginning of the convolution function for this point
      int cind=sSize*sSize*(fracu+overSample*(fracv+overSample*coff));
      // The actual grid point
      int gind=iu+gSize*iv; 
      for (int suppv=0; suppv<sSize; suppv++)
      {
#ifdef USEBLAS
        //        void cblas_zaxpy(const int N, const void *alpha, const void *X,
        //                         const int incX, void *Y, const int incY);
        cblas_caxpy(sSize, &data[find], &C[cind], 1, &grid[gind-support], 1);
#else
        for (int suppu=0; suppu<sSize; suppu++)
        {
          grid[gind+suppu-support]+=data[find]*C[cind+suppu];
        }
#endif
        gind+=gSize;
        cind+=sSize;
      }
    }
  }
  finish = clock();
  // Report on timings
  time = (double(finish)-double(start))/CLOCKS_PER_SEC;
  cout << "    Time " << time << " (s) " << endl;
  cout << "    Time per visibility sample " << 1e6*time/double(nSamples) << " (us) " << endl;
  cout << "    Time per visibility spectral sample " << 1e6*time/double(nSamples*nChan) << " (us) " << endl;
  cout << "    Time per grid-addition " << 1e9*time/(double(nSamples)*double(nChan)* double((2*support)*(2*support+1))) << " (ns) " << endl;

  cout << "+++++ Reverse processing +++++" << endl;

  grid.assign(grid.size(), Value(1.0));
  
  // Just run the gridding in reverse
  start = clock();
  for (int i=0; i<nSamples; i++)
  {
    for (int chan=0; chan<nChan; chan++)
    {

      int find=i*nChan+chan;
      outdata[find]=0.0;

      int coff=cOffset[find];

      Coord uScaled=freq[chan]*u[i]/cellSize;
      int iu=int(uScaled);
      if (uScaled<Coord(iu))
      {
        iu-=1;
      }
      int fracu=int(overSample*(uScaled-Coord(iu)));
      iu+=gSize/2;

      Coord vScaled=freq[chan]*v[i]/cellSize;
      int iv=int(vScaled);
      if (vScaled<Coord(iv))
      {
        iv-=1;
      }
      int fracv=int(overSample*(vScaled-Coord(iv)));
      iv+=gSize/2;

      // The beginning of the convolution function for this point
      int cind=sSize*sSize*(fracu+overSample*(fracv+overSample*coff));
      // The actual grid point
      int gind=iu+gSize*iv; 
      for (int suppv=0; suppv<sSize; suppv++)
      {
#ifdef USEBLAS
        Value dot;
        cblas_cdotu_sub(2*support+1, &grid[gind-support], 1, &C[cind-support+cCenter], 1,
            &dot);
        outdata[find]+=dot;
#else
        for (int suppu=0; suppu<sSize; suppu++)
        {
          outdata[find]+=grid[gind+suppu-support]*C[cind+suppu-support+cCenter];
        }
#endif
        gind+=gSize;
        cind+=sSize;
      }
    }
  }
  finish = clock();
  // Report on timings
  time = (double(finish)-double(start))/CLOCKS_PER_SEC;
  cout << "    Time " << time << " (s) " << endl;
  cout << "    Time per visibility sample " << 1e6*time/double(nSamples) << " (us) " << endl;
  cout << "    Time per visibility spectral sample " << 1e6*time/double(nSamples*nChan) << " (us) " << endl;
  cout << "    Time per grid-addition " << 1e9*time/(double(nSamples)*double(nChan)* double((2*support)*(2*support+1))) << " (ns) " << endl;

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
// wCellSize - size of one w grid cell in wavelengths
// wSize - Size of lookup table in w
int wprojection(const std::vector<Coord>& u, const std::vector<Coord>& v,
    const std::vector<Coord>& w, const std::vector<Value>& data,
    std::vector<Value>& outdata, const std::vector<Coord>& freq,
    const Coord cellSize, const Coord baseline, const int wSize,
    std::vector<Value>& grid, const int gSize)
{

  const int nSamples = u.size();
  const int nChan = freq.size();

  cout
      << "************************* W projection gridding *********************"
      << endl;
  int support=static_cast<int>(1.5*sqrt(abs(baseline)
      *static_cast<Coord>(cellSize)*freq[0])/cellSize);
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
  int sSize=2*(support+1);

  int cCenter=sSize/2;

  std::vector<Value> C(sSize*sSize*overSample*overSample*wSize);
  cout << "Size of convolution function = " << sSize*sSize*overSample
      *overSample*wSize*8/(1024*1024) << " MB" << std::endl;
  cout << "Shape of convolution function = [" << sSize << ", " << sSize << ", "
      << overSample << ", " << overSample << ", " << wSize << "]" << std::endl;

  for (int k=0; k<wSize; k++)
  {
    double w=double(k-wSize/2);
    double fScale=sqrt(abs(w)*wCellSize*freq[0])/cellSize;
    for (int osj=0; osj<overSample; osj++)
    {
      for (int osi=0; osi<overSample; osi++)
      {
        for (int j=0; j<sSize; j++)
        {
          double j2=std::pow((double(j-cCenter)+double(osj)/double(overSample)), 2);
          for (int i=0; i<sSize; i++)
          {
            double r2=j2+std::pow((double(i-cCenter)+double(osi)/double(overSample)), 2);
            long int cind=i+sSize*(j+sSize*(osi+overSample*(osj+overSample*k)));
            if (w!=0.0)
            {
              C[cind]=static_cast<Value>(std::cos(r2/(w*fScale)));
            }
            else
            {
              C[cind]=static_cast<Value>(std::exp(-r2));
            }
          }
        }
      }
    }
  }

  // Now normalise the convolution function
  Real sumC=0.0;
  for (int i=0; i<sSize*sSize*overSample*overSample*wSize; i++)
  {
    sumC+=abs(C[i]);
  }

  for (int i=0; i<sSize*sSize*overSample*overSample*wSize; i++)
  {
    C[i]*=Value(wSize*overSample*overSample/sumC);
  }
  sumC=0.0;
  for (int i=0; i<sSize*sSize*overSample*overSample*wSize; i++)
  {
    sumC+=abs(C[i]);
  }

  std::vector<unsigned int> cOffset(data.size());
  for (int i=0; i<nSamples; i++)
  {
    for (int chan=0; chan<nChan; chan++)
    {

      int find=i*nChan+chan;

      Coord wScaled=freq[chan]*w[i]/wCellSize;
      cOffset[find]=wSize/2+int(wScaled);
    }
  }

  return generic(u, v, w, data, outdata, freq, cellSize, C, support,
      overSample, cOffset, grid, gSize);
}

int main()
{
  const int baseline=2000; // Maximum baseline in meters
  const int nSamples=10000; // Number of data samples
  const int gSize=512; // Size of output grid in pixels
  const Coord cellSize=50; // Cellsize of output grid in wavelengths
  const int wSize=64; // Number of lookup planes in w projection
  const int nChan=16; // Number of spectral channels

  // Initialize the data to be gridded
  std::vector<Coord> u(nSamples);
  std::vector<Coord> v(nSamples);
  std::vector<Coord> w(nSamples);
  std::vector<Value> data(nSamples*nChan);
  std::vector<Value> outdata(nSamples*nChan);

  for (int i=0; i<nSamples; i++)
  {
    u[i]=baseline*Coord(rand())/Coord(RAND_MAX)-baseline/2;
    v[i]=baseline*Coord(rand())/Coord(RAND_MAX)-baseline/2;
    w[i]=baseline*Coord(rand())/Coord(RAND_MAX)-baseline/2;
    for (int chan=0; chan<nChan; chan++)
    {
      data[i*nChan+chan]=1.0;
      outdata[i*nChan+chan]=0.0;
    }
  }

  // Measure frequency in inverse wavelengths
  std::vector<Coord> freq(nChan);
  for (int i=0; i<nChan; i++)
  {
    freq[i]=(1.4e9-2.0e5*Coord(i)/Coord(nChan))/2.998e8;
  }

  std::vector<Value> grid(gSize*gSize);

  wprojection(u, v, w, data, outdata, freq, cellSize, baseline, wSize, grid,
      gSize);

  cout << "Done" << endl;

  return 0;
}
