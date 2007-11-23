//   This C++ program has been written to demonstrate the convolutional resampling algorithm used in radio
// interferometry. It should compile with:
//      g++ -O2 -fstrict-aliasing tConvolveBLAS.cc -o tConvolveBLAS
// Enabling BLAS support on OS X:
//      g++ -DUSEBLAS -O2 -fstrict-aliasing -framework vecLib tConvolveBLAS.cc -o tConvolveBLAS
//
// Strict-aliasing tells the compiler that there are no memory locations accessed through aliases.
//
// The challenge is to minimize the run time - specifically the time per grid addition. On a MacBookPro 
// 2GHz Intel Core Duo this is about 5.9ns
//
// For further details contact Tim.Cornwell@csiro.au
// November 22, 2007
// - Rewritten from tConvolve to use BLAS, and to be much smarter about not using strides in C

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
typedef double Coord;
typedef float Real;
typedef std::complex<Real> Value;

// Perform gridding
//
// data - values to be gridded in a 1D vector
// support - Total width of convolution function=2*support+1
// overSample - Oversampling factor for the convolution function
// C - convolution function
// cOffset - offsets into convolution function per data point
// iu, iv - integer locations of grid points
// grid - Output grid
// gSize - size of one axis of grid

void gridData(const std::vector<Value>& data, const int support,
    const std::vector<Value>& C, const std::vector<unsigned int>& cOffset,
    const std::vector<unsigned int>& iu, const std::vector<unsigned int>& iv,
    std::vector<Value>& grid, const int gSize)
{

  int sSize=2*support+1;
  for (int find=0; find<data.size(); find++)
  {

    int coff=cOffset[find];

    // The actual grid point
    int gind=iu[find]+gSize*iv[find];
    for (int suppv=0; suppv<sSize; suppv++)
    {
#ifdef USEBLAS
      cblas_caxpy(sSize, &data[find], &C[coff-support], 1, &grid[gind-support], 1);
#else
      for (int suppu=0; suppu<sSize; suppu++)
      {
        grid[gind-support+suppu]+=data[find]*C[coff-support+suppu];
      }
#endif
      gind+=gSize;
      coff+=sSize;
    }
  }
}

// Perform degridding
//
// grid - Input grid
// gSize - size of one axis of grid
// support - Total width of convolution function=2*support+1
// overSample - Oversampling factor for the convolution function
// C - convolution function
// cOffset - offsets into convolution function per data point
// iu, iv - integer locations of grid points
// data - Output values in a 1D vector

void degridData(const std::vector<Value>& grid, const int gSize, const int support,
    const std::vector<Value>& C, const std::vector<unsigned int>& cOffset,
    const std::vector<unsigned int>& iu, const std::vector<unsigned int>& iv,
    std::vector<Value>& outdata)
{

  int sSize=2*support+1;

  for (int find=0; find<outdata.size(); find++)
  {

    outdata[find]=0.0;

    int coff=cOffset[find];

    // The actual grid point from which we offset
    int gind=iu[find]+gSize*iv[find];
    for (int suppv=0; suppv<sSize; suppv++)
    {
#ifdef USEBLAS
      Value dot;
      cblas_cdotu_sub(sSize, &grid[gind-support], 1, &C[coff-support], 1,
          &dot);
      outdata[find]+=dot;
#else
      for (int suppu=0; suppu<sSize; suppu++)
      {
        outdata[find]+=grid[gind-support+suppu]*C[coff-support+suppu];
      }
#endif
      gind+=gSize;
      coff+=sSize;
    }

  }
}

// Initialize W project convolution function
//
// nSamples - number of visibility samples
// freq - temporal frequency (inverse wavelengths)
// cellSize - size of one grid cell in wavelengths
// gSize - size of grid in pixels (per axis)
// support - Total width of convolution function=2*support+1
// wCellSize - size of one w grid cell in wavelengths
// wSize - Size of lookup table in w
void initC(const int nSamples, const std::vector<Coord>& w,
    const std::vector<Coord>& freq, const Coord cellSize, 
    const Coord baseline,
    const int wSize, const int gSize, int& support, int& overSample,
    Coord& wCellSize, std::vector<Value>& C)
{

  const int nChan = freq.size();

  cout << "Initializing W projection convolution function" << endl;
  support=static_cast<int>(1.5*sqrt(abs(baseline) *static_cast<Coord>(cellSize)
      *freq[0])/cellSize);
  overSample=8;
  cout << "Support = " << support << " pixels" << endl;
  wCellSize=2*baseline*freq[0]/wSize;
  cout << "W cellsize = " << wCellSize << " wavelengths" << endl;

  // Convolution function. This should be the convolution of the
  // w projection kernel (the Fresnel term) with the convolution
  // function used in the standard case. The latter is needed to
  // suppress aliasing. In practice, we calculate entire function
  // by Fourier transformation. Here we take an approximation that
  // is good enough.
  int sSize=2*support+1;

  int cCenter=(sSize-1)/2;

  C.resize(sSize*sSize*overSample*overSample*wSize);
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
}
// Initialize Lookup function
//
// nSamples - number of visibility samples
// freq - temporal frequency (inverse wavelengths)
// cellSize - size of one grid cell in wavelengths
// gSize - size of grid in pixels (per axis)
// support - Total width of convolution function=2*support+1
// wCellSize - size of one w grid cell in wavelengths
// wSize - Size of lookup table in w
void initCOffset(const std::vector<Coord>& u, const std::vector<Coord>& v,
    const std::vector<Coord>& w, const std::vector<Coord>& freq,
    const Coord cellSize, const Coord wCellSize, const Coord baseline,
    const int wSize, const int gSize, const int support, const int overSample,
    std::vector<unsigned int>& cOffset, std::vector<unsigned int>& iu,
    std::vector<unsigned int>& iv)
{

  const int nSamples = u.size();
  const int nChan = freq.size();

  int sSize=2*support+1;

  // Now calculate the offset for each visibility point
  cOffset.resize(nSamples*nChan);
  iu.resize(nSamples*nChan);
  iv.resize(nSamples*nChan);
  for (int i=0; i<nSamples; i++)
  {
    for (int chan=0; chan<nChan; chan++)
    {

      int find=i*nChan+chan;

      Coord uScaled=freq[chan]*u[i]/cellSize;
      iu[find]=int(uScaled);
      if (uScaled<Coord(iu[find]))
      {
        iu[find]-=1;
      }
      int fracu=int(overSample*(uScaled-Coord(iu[find])));
      iu[find]+=gSize/2;

      Coord vScaled=freq[chan]*v[i]/cellSize;
      iv[find]=int(vScaled);
      if (vScaled<Coord(iv[find]))
      {
        iv[find]-=1;
      }
      int fracv=int(overSample*(vScaled-Coord(iv[find])));
      iv[find]+=gSize/2;

      // The beginning of the convolution function for this point
      Coord wScaled=freq[chan]*w[i]/wCellSize;
      int woff=wSize/2+int(wScaled);
      cOffset[find]=sSize*sSize*(fracu+overSample*(fracv+overSample*woff));
    }
  }

}

int main()
{
  const int baseline=2000; // Maximum baseline in meters
  const int nSamples=10000; // Number of data samples
  const int gSize=512; // Size of output grid in pixels
  const Coord cellSize=40; // Cellsize of output grid in wavelengths
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

  // Initialize convolution function and offsets
  std::vector<std::complex<float> > C;
  int support, overSample;
  std::vector<unsigned int> cOffset;
  // Vectors of grid centers
  std::vector<unsigned int> iu;
  std::vector<unsigned int> iv;
  Coord wCellSize;
  
  initC(nSamples, w, freq, cellSize, baseline, wSize, gSize, support,
      overSample, wCellSize, C);
  initCOffset(u, v, w, freq, cellSize, wCellSize, baseline, wSize, gSize,
      support, overSample, cOffset, iu, iv);
  int sSize=2*support+1;
  
  std::vector<Value> grid(gSize*gSize);
  cout << "+++++ Forward processing +++++" << endl;

  clock_t start, finish;
  double time;

  start = clock();
  grid.assign(grid.size(), Value(0.0));
  gridData(data, support, C, cOffset, iu, iv, grid, gSize);
  finish = clock();
  // Report on timings
  // Report on timings
  time = (double(finish)-double(start))/CLOCKS_PER_SEC;
  cout << "    Time " << time << " (s) " << endl;
  cout << "    Time per visibility spectral sample " << 1e6*time/double(data.size()) << " (us) " << endl;
  cout << "    Time per degridding " << 1e9*time/(double(data.size())* double((sSize)*(sSize))) << " (ns) " << endl;

  cout << "+++++ Reverse processing +++++" << endl;

  grid.assign(grid.size(), Value(1.0));
  start = clock();
  degridData(grid, gSize, support, C, cOffset, iu, iv, outdata);
  finish = clock();
  // Report on timings
  time = (double(finish)-double(start))/CLOCKS_PER_SEC;
  cout << "    Time " << time << " (s) " << endl;
  cout << "    Time per visibility spectral sample " << 1e6*time/double(data.size()) << " (us) " << endl;
  cout << "    Time per degridding " << 1e9*time/(double(data.size())* double((sSize)*(sSize))) << " (ns) " << endl;

  cout << "Done" << endl;

  return 0;
}
