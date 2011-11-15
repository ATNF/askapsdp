/// @file 
///
/// @brief dummy data generator
/// @details This application is intended to test software correlator. It just
/// pumps some dummy data through TCP sockets to the correlator.
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
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>


#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string>

ASKAP_LOGGER(logger, ".tDummyDataGenerator");


using namespace std;
using namespace askap;
using namespace askap::scimath;

casa::Complex sampledFunc(const float time, const float delay)
{
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


// Main function
int main(int, const char** argv)
{
    try {
       casa::Timer timer;
       timer.mark();
    
       const float samplingRate = 32./27.*1e6; // in samples per second
       casa::Vector<casa::Complex> buf1;
       casa::Vector<casa::Complex> buf2;
       acquire(buf1,buf2,5.2e-6,32*31250,samplingRate);
       // assume that antenna1 = antenna3 for this simple test
       casa::Vector<casa::Complex> buf3(buf1);

       std::cout<<"initialisation of dummy data "<<"user:   " << timer.user() << " system: " << timer.system()
                                      << " real:   " << timer.real()<<std::endl;
       timer.mark();
       // connection to the correlator server comes here along with the threading stuff
       //              
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
    
