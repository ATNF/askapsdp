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

#include <boost/thread/thread.hpp>
#include <boost/scoped_array.hpp>

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

// global parameters to emulate simultaneous sampling on all antennas with potentially asynchronous arrival
boost::condition_variable sampleTrigger;
boost::mutex sampleTriggerMutex;
long sampleBAT = 0;

struct Worker {
   Worker(const casa::Vector<casa::Complex> &data, const int antID, const int chanID, const int nbeams) : 
          itsData(data), itsAnt(antID), itsChan(chanID), itsNBeam(nbeams) 
   {
     ASKAPCHECK(itsNBeam > 0, "Number of beams is supposed to be positive");
     ASKAPCHECK(itsAnt >= 0, "Antenna ID should be non-negative");
     ASKAPCHECK(itsChan >= 0, "Spectral channel (card) ID should be non-negative");     
     ASKAPCHECK(sizeof(float) == sizeof(long), "float and long has different sizes, packing of the data stream is likely to fail!");
   }
   
   void operator()() const {      
      boost::this_thread::disable_interruption di;
      boost::scoped_array<float> buffer(new float[2*itsData.nelements()*itsNBeam+3]); // BAT, antenna and channel indices are in the stream
      // C-stype packing of the 3 compulsory IDs
      long *idPtr = (long*)buffer.get();
      idPtr[0] = 0; // initialise BAT with 0
      idPtr[1] = itsAnt; // antenna ID
      idPtr[2] = itsChan; // channel ID
      //
      for (long beam = 0, counter = 3; beam < itsNBeam; ++beam) {
           for (long sample = 0; sample < long(itsData.nelements()); ++sample, counter+=2) {
                buffer[counter] = real(itsData[sample]);
                buffer[counter+1] = imag(itsData[sample]);                
           }
      }
      // buffer ready, wait for sampling trigger
      while (!boost::this_thread::interruption_requested()) {
             boost::unique_lock<boost::mutex> lock(sampleTriggerMutex);
             while ((sampleBAT == idPtr[0]) && !boost::this_thread::interruption_requested()) {  
                 sampleTrigger.wait(lock);
             }
             //boost::this_thread::sleep(boost::posix_time::seconds(1));
             if (!boost::this_thread::interruption_requested()) {
                 idPtr[0] = sampleBAT;
                 //
                 // here we will send the buffer over the socket
                 std::cout<<"test, BAT="<<idPtr[0]<<std::endl;
             }
      }
      std::cout<<"finishing"<<std::endl;
   }
   
private:
   casa::Vector<casa::Complex> itsData;   
   int itsAnt;
   int itsChan;
   int itsNBeam;
};

// Main function
int main(int, const char** argv)
{
    try {
       casa::Timer timer;
       timer.mark();
       casa::Time time;
    
       const float samplingRate = 32./27.*1e6; // in samples per second
       casa::Vector<casa::Complex> buf1;
       casa::Vector<casa::Complex> buf2;
       acquire(buf1,buf2,5.2e-6,32*3125,samplingRate);
       // assume that antenna1 = antenna3 for this simple test

       std::cout<<"initialisation of dummy data "<<"user:   " << timer.user() << " system: " << timer.system()
                                      << " real:   " << timer.real()<<std::endl;
       timer.mark();
       // connection to the correlator server comes here along with the threading stuff
       const int nBeam = 1;
       const int nChan = 1;
       
       boost::thread_group threads;
       for (int cnt = 0; cnt<nChan; ++cnt) {
            threads.create_thread(Worker(buf1, 0, cnt, nBeam));
       }
       
       for (size_t cycle = 0; cycle < 10; ++cycle) {
            std::cout<<"cycle "<<cycle<<std::endl;
            {
              boost::lock_guard<boost::mutex> lock(sampleTriggerMutex);
              sampleBAT = long(time.age());
            }
            sampleTrigger.notify_all();
            sleep(1);
       }
       threads.interrupt_all();
       sampleTrigger.notify_all();
       std::cout<<"waiting to finish"<<std::endl;       
       threads.join_all();
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
    
