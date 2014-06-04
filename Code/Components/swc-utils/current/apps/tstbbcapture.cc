/// @file tstbbcapture.cc
///
/// @copyright (c) 2014 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

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
#include <scimath/Mathematics/StatAcc.h>
#include <scimath/Mathematics/HistAcc.h>
#include <askap/AskapError.h>


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
try {
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
   // use casa methods to get the histogram because the rounding may be done differently 
   const float binWidth = 2*maxAmp / nbins;
   casa::HistAcc<float> histRe(-maxAmp, maxAmp, binWidth);
   casa::HistAcc<float> histIm(-maxAmp, maxAmp, binWidth);
   std::vector<size_t> reCounts(nbins,0), imCounts(nbins,0);
   for (size_t i = 0; i<data.size(); ++i) {
        histRe.put(std::real(data[i]));
        histIm.put(std::imag(data[i]));
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

   casa::Block<casa::uInt> binsRe, binsIm;
   casa::Block<float> valsRe, valsIm;
   const casa::uInt nbinsRe = histRe.getHistogram(binsRe, valsRe);
   const casa::uInt nbinsIm = histIm.getHistogram(binsIm, valsIm);
   ASKAPASSERT(nbinsRe == nbinsIm);

   std::ofstream os("hist.dat");
   /*
   for (size_t i = 0; i<nbins; ++i) {
       float x = -maxAmp + (float(i)/float(nbins))*maxAmp*2;
       os << i<<" "<< x << " " << reCounts[i]<< " "<< imCounts[i]<<std::endl;
   } */
   for (size_t i = 0; i<binsRe.nelements(); ++i) {
       os <<i<<" "<<valsRe[i]<<" "<<valsIm[i]<<" "<<binsRe[i]<<" "<<binsIm[i]<<std::endl;
   }
   std::cout<<"Mean(Re)="<<histRe.getStatistics().getMean()<<" Mean(Im)="<<histIm.getStatistics().getMean()<<std::endl;
} catch (const AskapError &ae) {
   std::cerr<<ae.what()<<std::endl;
}
}
