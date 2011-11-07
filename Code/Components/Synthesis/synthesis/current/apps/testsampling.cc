/// @file 
///
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


// for debugging
#include <apps/SimpleCorrelator.h>

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;
using namespace askap;
using namespace askap::scimath;

casa::Complex sampledFunc(const float time, const float delay)
{
  /*
  const float freq = 1/3.33*1e6;//0.5e6; // 1 MHz
  const float phase = -2.*casa::C::pi*freq*(time-delay);
  return casa::Complex(cos(phase),sin(phase));
  */
  casa::Complex res = 0.;
  const int spPt = 200;
  for (int i=0; i<spPt; ++i) {
       const float freq = 1e6/sqrt(2.)*float(i-spPt/2)/float(spPt);
       const float phase = -2.*casa::C::pi*freq*(time-delay);
       res += casa::Complex(cos(phase),sin(phase))/float(spPt);
  }
  return res;
}

void acquire(casa::Vector<casa::Complex> &buf1, casa::Vector<casa::Complex> &buf2, const float delay, 
                 const int nSamples, const float rate)
{
  buf1.resize(nSamples);
  buf2.resize(nSamples);
  for (int i=0; i<nSamples; ++i) {
       const float time = float(i) / rate;
       buf1[i] = sampledFunc(time,0.);
       buf2[i] = sampledFunc(time, delay);       
  }
}

void storeArray(const std::string &name, const casa::Vector<casa::Complex> &buf) {
  ofstream os(name.c_str());
  for (int i=0; i<int(buf.nelements()); ++i) {
     os<<i<<" "<<real(buf[i])<<" "<<imag(buf[i])<<" "<<abs(buf[i])<<" "<<arg(buf[i])/casa::C::pi*180<<std::endl;
  }
}


// Main function
int main(int, const char** argv)
{
    try {
       casa::Timer timer;
       timer.mark();
    
       const float samplingRate = 32./27.*1e6; // in samples per second
       casa::Vector<casa::Complex> buf1;
       casa::Vector<casa::Complex> buf2;
       /*
       acquire(buf1,buf2,5.2e-6,32,samplingRate);
       fft(buf1, true);       
       fft(buf2, true); 
       for (int i=0; i<int(buf1.nelements()); ++i) {
            buf1[i] *= conj(buf2[i]);            
       }
       fft(buf1, false);
       
       storeArray("a.dat",buf1);
       */
       acquire(buf1,buf2,5.2e-6,32*31250,samplingRate);
       // assume that antenna1 = antenna3 for this simple test
       casa::Vector<casa::Complex> buf3(buf1);
       /*
       std::vector<std::complex<int> > ant1(buf1.nelements());
       std::vector<std::complex<int> > ant2(buf2.nelements());
       std::vector<std::complex<int> > ant3(buf3.nelements());
       const float factor = 1e4;
       for (size_t i=0; i<buf1.nelements(); ++i) {
            ant1[i] = std::complex<int>(int(real(buf1[i])*factor),int(imag(buf1[i])*factor));
            ant2[i] = std::complex<int>(int(real(buf2[i])*factor),int(imag(buf2[i])*factor));
            ant3[i] = std::complex<int>(int(real(buf3[i])*factor),int(imag(buf3[i])*factor));            
       }
       */
       std::cout<<"initialisation of dummy data "<<"user:   " << timer.user() << " system: " << timer.system()
                                      << " real:   " << timer.real()<<std::endl;
       timer.mark();
       int nDelays = 1;
       //typedef std::complex<int> accType;
       typedef casa::Complex accType;
       SimpleCorrelator<accType> sc12(nDelays);
       SimpleCorrelator<accType> sc13(nDelays);
       SimpleCorrelator<accType> sc23(nDelays);

       std::cout<<"initialisation of correlators "<<"user:   " << timer.user() << " system: " << timer.system()
                                      << " real:   " << timer.real()<<std::endl;
       timer.mark();
       for (size_t i=0; i<64*18; ++i) {
           
           sc12.accumulate(buf1.data(), buf2.data(), int(buf1.nelements()));
           sc13.accumulate(buf1.data(), buf3.data(), int(buf1.nelements()));
           sc23.accumulate(buf2.data(), buf3.data(), int(buf2.nelements()));
           /*
           sc12.accumulate(ant1.begin(), ant2.begin(), int(ant1.size()));
           sc13.accumulate(ant1.begin(), ant3.begin(), int(ant3.size()));
           sc23.accumulate(ant2.begin(), ant3.begin(), int(ant2.size()));
           */
        }

       std::cout<<"accumulation "<<"user:   " << timer.user() << " system: " << timer.system()
                                      << " real:   " << timer.real()<<std::endl;
       std::cout<<"throughput for 64 MHz, 9 dual pol beams, 3 baselines and "<<nDelays<<" delay steps is "<<1e3 / timer.real() <<
                  " kSamples/sec"<<std::endl;                               
       
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
    