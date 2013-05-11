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
#include <casa/Arrays/ArrayMath.h>

#include <utils/ComplexGaussianNoise.h>

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;
using namespace askap;
using namespace askap::scimath;

struct Worker {
 Worker() : itsGen(1.) {}
 void run();

protected:
 casa::Complex oneSample() const;
 void acquire(casa::Vector<casa::Complex> &buf1, casa::Vector<casa::Complex> &buf2, const int nSamples) const;
 void storeArray(const std::string &name, const casa::Vector<casa::Complex> &buf) const; 
 static float clip(float in);
private:
 scimath::ComplexGaussianNoise itsGen;
};

float Worker::clip(float in) {
   const float scale = 1e2;
   int intIn = int(in * scale);
   intIn &= ~0x7;
   return float(intIn) / scale;
}


casa::Complex Worker::oneSample() const
{
  const casa::Complex value = itsGen();
  
  //return value;
  return casa::Complex(clip(real(value)), clip(imag(value)));
}

void Worker::acquire(casa::Vector<casa::Complex> &buf1, casa::Vector<casa::Complex> &buf2, const int nSamples) const
{
  buf1.resize(nSamples);
  buf2.resize(nSamples);
  for (int i=0; i<nSamples; ++i) {
       buf1[i] = oneSample();
       buf2[i] = oneSample();
  }
}

void Worker::storeArray(const std::string &name, const casa::Vector<casa::Complex> &buf) const {
  ofstream os(name.c_str());
  for (int i=0; i<int(buf.nelements()); ++i) {
     os<<i<<" "<<real(buf[i])<<" "<<imag(buf[i])<<" "<<abs(buf[i])<<" "<<arg(buf[i])/casa::C::pi*180<<std::endl;
  }
}

void Worker::run() {
  casa::Vector<casa::Complex> buf1;
  casa::Vector<casa::Complex> buf2;

  const size_t nChan = 1024;
  const size_t nBlocks = 200000; 

  casa::Vector<casa::Complex> resBuffer(nChan,casa::Complex(0.,0.));
  for (size_t block = 0; block < nBlocks; ++block) {
       acquire(buf1, buf2, int(nChan));

       fft(buf1, true);       
       fft(buf2, true); 
       for (int i=0; i<int(buf1.nelements()); ++i) {
            buf1[i] *= conj(buf2[i]);            
       }
       //fft(buf1, false);
       resBuffer += buf1;
  }
  ASKAPDEBUGASSERT(nBlocks > 0);
  resBuffer /= float(nBlocks);
  storeArray("a.dat",resBuffer);
}


// Main function
int main(int, const char** argv)
{
    try {
       casa::Timer timer;
       timer.mark();

       Worker wrk;
       wrk.run();
    
       std::cout<<"run time: "<<"user:   " << timer.user() << " system: " << timer.system()
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
    
