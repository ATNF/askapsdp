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
       const float samplingRate = 32./27.*1e6; // in samples per second
       casa::Vector<casa::Complex> buf1;
       casa::Vector<casa::Complex> buf2;
       acquire(buf1,buf2,5.2e-6,32,samplingRate);
       fft(buf1, true);       
       fft(buf2, true); 
       for (int i=0; i<int(buf1.nelements()); ++i) {
            buf1[i] *= conj(buf2[i]);            
       }
       fft(buf1, false);
       
       storeArray("a.dat",buf1);
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
    