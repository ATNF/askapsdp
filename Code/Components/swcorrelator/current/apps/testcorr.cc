/// @file 
/// @brief experiments with correlation to debug sw correlator
///
/// @copyright (c) 2007 CSIRO
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
///
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#include <askap/AskapError.h>
#include <fft/FFTWrapper.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Constants.h>
#include <casa/OS/Timer.h>


#include <swcorrelator/SimpleCorrelator.h>
#include <swcorrelator/CaptureWorker.h>

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;
using namespace askap;
using namespace askap::scimath;
using namespace askap::swcorrelator;

void storeArray(const std::string &name, const casa::Vector<casa::Complex> &buf) {
  ofstream os(name.c_str());
  for (int i=0; i<int(buf.nelements()); ++i) {
     os<<i<<" "<<real(buf[i])<<" "<<imag(buf[i])<<" "<<abs(buf[i])<<" "<<arg(buf[i])/casa::C::pi*180<<std::endl;
  }
}


// Main function
int main(int argc, const char** argv)
{
    try {
       casa::Timer timer;
       timer.mark();

       ASKAPCHECK(argc>=3, "Usage: "<<argv[0]<<" file1.dat file2.dat");

       std::vector<std::complex<float> > buf1 = CaptureWorker::read(argv[1]);
       std::vector<std::complex<float> > buf2 = CaptureWorker::read(argv[2]);
    
       const int nLags = buf1.size();//3000;

       std::cout<<"initialisation of data "<<"user:   " << timer.user() << " system: " << timer.system()
                                      << " real:   " << timer.real()<<std::endl;
       ASKAPCHECK(buf1.size() == buf2.size(), "Unequal number of samples in two buffers: "<<buf1.size()<<" != "<<buf2.size());
       std::cout<<"buffers have "<<buf1.size()<<" samples each"<<std::endl;
       timer.mark();

       //std::ofstream os("result.dat");
       casa::Vector<casa::Complex> outBuf(nLags,0.);
       int logStep = nLags / 100;
       if (logStep == 0) {   
           logStep = 1;
       }
       int nDone  = 0;
#pragma omp parallel for shared(nDone)
       for (int lag = 0; lag<nLags; ++lag) {
            #pragma omp critical
            { 
              ++nDone;
              if (nDone % logStep == 0) {
                std::cout<<"Done "<<nDone/logStep<<"%"<<std::endl;
              }
            }
            typedef std::complex<float> accType;
            Simple3BaselineCorrelator<accType> s3bc(0,-lag,0);
            s3bc.accumulate(buf1.begin(), buf2.begin(), buf1.begin(), int(buf1.size()));
            const accType vis12 = s3bc.getVis12() / float(s3bc.nSamples12()!=0 ? s3bc.nSamples12() : 1.);
            const accType vis23 = s3bc.getVis23() / float(s3bc.nSamples23()!=0 ? s3bc.nSamples23() : 1.);
            const accType vis13 = s3bc.getVis13() / float(s3bc.nSamples13()!=0 ? s3bc.nSamples13() : 1.);
            outBuf[lag] = vis12;
            //os<<lag<<" "<<abs(vis12)<<" "<<arg(vis12)<<" "<<abs(vis23)<<" "<<arg(vis23)<<" "<<abs(vis13)<<
            //    " "<<arg(vis13)<<std::endl;
            //os<<lag<<" "<<real(vis12)<<" "<<imag(vis12)<<" "<<real(vis23)<<" "<<imag(vis23)<<" "<<real(vis13)<<
            //    " "<<imag(vis13)<<std::endl;
       }

       std::cout<<"correlations: "<<"user:   " << timer.user() << " system: " << timer.system()
                                      << " real:   " << timer.real()<<std::endl;
       timer.mark();

       //fft(outBuf, false);
       storeArray("result.dat",outBuf);
       std::cout<<"fft/storing: "<<"user:   " << timer.user() << " system: " << timer.system()
                                      << " real:   " << timer.real()<<std::endl;
    }
    catch (const askap::AskapError& x) {
        std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    } catch (const std::exception& x) {
        std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    }

    return 0;
}
    
