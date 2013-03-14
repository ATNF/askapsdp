#include <swcorrelator/CaptureWorker.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <complex>

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicSL.h>
#include <scimath/Mathematics/FFTServer.h>

using namespace askap;
using namespace askap::swcorrelator;

void fftExperiments(const std::vector<std::complex<float> > &data)
{
  const casa::uInt fftSize = 1024;
  casa::Vector<casa::Complex> input(fftSize);
  casa::Vector<casa::Complex> result(fftSize,casa::Complex(0.,0.));
  casa::FFTServer<casa::Float,casa::Complex> server;
  casa::Vector<casa::Complex> output(fftSize);
  for (casa::uInt start = 0; start+fftSize<data.size(); start+=fftSize) {
       for (casa::uInt i=0; i<fftSize; ++i) {
            input[i] = data[start + i];
       }
       server.fft(output,input);
       result += output;
  }
  std::ofstream os("samplefft.dat");
  for (casa::uInt i=0; i<result.nelements(); ++i) {
       os<<i<<" "<<casa::real(result[i])<<" "<<casa::imag(result[i])<<std::endl;
  }
}

int main(int argc, char **argv) {
   const std::string fname = argc >= 2 ? argv[1] : "apps/BB/ant0.beam0.chan0.bat4830973926000000.dat";
   size_t nbins = 100;
   std::vector<std::complex<float> > data = CaptureWorker::read(fname);
   float maxAmp = 0;
   for (size_t i = 0; i<data.size(); ++i) {
        const float amp = std::abs(data[i]);
        if (!i || (amp > maxAmp)) {
            maxAmp = amp;
        }
   }
   //
   fftExperiments(data);
   //
   const int intMaxAmp = int(maxAmp);
   std::vector<size_t> reCounts(nbins,0), imCounts(nbins,0);
   for (size_t i = 0; i<data.size(); ++i) {
        const int re = int(std::real(data[i]));
        const int im = int(std::imag(data[i]));
        const size_t reBin = ((re + intMaxAmp) * nbins) / intMaxAmp / 2;
        if ((reBin >= 0) && (reBin < nbins)) {
            ++reCounts[reBin];
        } 
        const size_t imBin = ((im + intMaxAmp) * nbins) / intMaxAmp / 2;
        if ((imBin >= 0) && (imBin < nbins)) {
            ++imCounts[imBin];
        }         
        //std::cout<<int(std::real(data[i]))<<" "<<int(std::imag(data[i]))<<std::endl;
   }
   std::ofstream os("hist.dat");
   for (size_t i = 0; i<nbins; ++i) {
       float x = -maxAmp + (float(i)/float(nbins))*maxAmp*2;
       os << i<<" "<< x << " " << reCounts[i]<< " "<< imCounts[i]<<std::endl;
   }
}
