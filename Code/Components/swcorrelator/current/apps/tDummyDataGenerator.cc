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
#include <mwcommon/AskapParallel.h>
// we don't use the manager as such, but it is handy to use other stuff defined there
#include <swcorrelator/BufferManager.h>


#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string>

ASKAP_LOGGER(logger, ".tDummyDataGenerator");

#include <boost/thread/thread.hpp>
#include <boost/scoped_array.hpp>
#include <boost/asio.hpp>

using namespace std;
using namespace askap;
using namespace askap::scimath;
using namespace askap::swcorrelator;

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

struct Worker {
   Worker(const casa::Vector<casa::Complex> &data, const int antID, const int chanID, const int nbeams) : 
          itsData(data), itsAnt(antID), itsChan(chanID), itsNBeam(nbeams) 
   {
     ASKAPCHECK(itsNBeam > 0, "Number of beams is supposed to be positive");
     ASKAPCHECK(itsAnt >= 0, "Antenna ID should be non-negative");
     ASKAPCHECK(itsChan >= 0, "Spectral channel (card) ID should be non-negative");     
     ASKAPCHECK(sizeof(float) == sizeof(long), "float and long has different sizes, packing of the data stream is likely to fail!");
   }
   
   /// @brief wait for the next timestamp
   /// @details This method suspends the current data receiving thread until the next sampling trigger (with a different
   /// BAT from the one given as the parameter)
   /// @param[in] lastBAT BAT at the start of the last sampling trigger
   /// @return BAT time of the new sample or a negative number if the termination of the thread had been requested
   static long waitForSamplingTrigger(const long lastBAT) {
        try {
           // using shared_lock - multiple readers, single writer design
           boost::shared_lock<boost::shared_mutex> lock(theirSampleTriggerMutex);
           while ((theirSampleBAT == lastBAT) && !boost::this_thread::interruption_requested()) {  
                 theirSampleTrigger.wait(lock);
           }
           //boost::this_thread::sleep(boost::posix_time::seconds(1));
           return theirSampleBAT;      
        }
        catch (const boost::thread_interrupted&) {}
        return -1;
   }
   
   void operator()() const {      
      ASKAPLOG_INFO_STR(logger, "Data generator thread started, id="<<boost::this_thread::get_id());
      try {   
        const long msgSize = 2*itsData.nelements()+sizeof(BufferHeader)/sizeof(float); // in floats
        boost::scoped_array<float> buffer(new float[msgSize]); 
        // C-stype packing of metadata
        BufferHeader* hdr = (BufferHeader*)buffer.get();
        hdr->bat = 0; // initialise BAT with 0
        hdr->antenna = itsAnt; // antenna ID
        hdr->freqId = itsChan; // channel ID      
        //
        for (long sample = 0, counter = sizeof(BufferHeader)/sizeof(float); 
                  sample < long(itsData.nelements()); ++sample, counter+=2) {
             buffer[counter] = real(itsData[sample]);
             buffer[counter+1] = imag(itsData[sample]);                
        }
        // buffer is filled
        boost::asio::io_service ioService;
        boost::asio::ip::tcp::resolver resolver(ioService);
        //const std::string host = "localhost";
        const std::string host = "delphinus";
        boost::asio::ip::tcp::resolver::query query(host,"3000");
        boost::asio::ip::tcp::endpoint endpoint;
        // for now just used the last endpoint (at least it seems to work)
        // ideally, we want to try every endpoint and then use the first one which works
        // I am too bored to code accurate exception handling here
        for(boost::asio::ip::tcp::resolver::iterator destination = resolver.resolve(query); 
            destination != boost::asio::ip::tcp::resolver::iterator(); ++destination) {
            endpoint = *destination;
        }            
        ASKAPLOG_INFO_STR(logger, "Data generation thread (id="<<boost::this_thread::get_id()<<") is about to connect to endpoint="<<endpoint);
        boost::asio::ip::tcp::socket socket(ioService);
        socket.connect(endpoint);
      
        // starting the loop
        try {
           /*
           if (hdr->antenna == 1) {
               // introduce a pause to test not-keeping-up
               boost::this_thread::sleep(boost::posix_time::seconds(1));
           }
           */
           
           // buffer ready, wait for sampling trigger
           for (long newBAT = waitForSamplingTrigger(long(hdr->bat)); newBAT > 0; newBAT = waitForSamplingTrigger(long(hdr->bat))) {
                hdr->bat = uint64_t(newBAT);
                ASKAPLOG_INFO_STR(logger, "New sampling trigger, BAT="<<hdr->bat);
                // here we will send the buffer over the socket
                for (int beam=0; beam<itsNBeam; ++beam) {
                     boost::asio::write(socket, boost::asio::buffer((void*)buffer.get(),msgSize*sizeof(float)));    
                }
           }
        }
        catch (const boost::thread_interrupted&) {}
      } catch (const std::exception &ex) {
        ASKAPLOG_FATAL_STR(logger, "Data generation thread (id="<<boost::this_thread::get_id()<<") is about to die: "<<ex.what());
      }
      ASKAPLOG_INFO_STR(logger, "Thread is finishing");
   }
   
   /// @brief set BAT corresponding to the new sample, notify all threads
   /// @details This method is supposed to be called from the main thread when the
   /// sampling takes place (and therefore it is static and common to all threads)
   /// @param[in] newBAT BAT of the new sample
   static void triggerSample(const long newBAT) {
      {
        boost::lock_guard<boost::shared_mutex> lock(theirSampleTriggerMutex);
        theirSampleBAT = long(time(0));
      }
      theirSampleTrigger.notify_all();
   }
   
private:
   // static parameters to emulate simultaneous sampling on all antennas with potentially asynchronous arrival
   static boost::condition_variable_any theirSampleTrigger;
   static boost::shared_mutex theirSampleTriggerMutex;
   static long theirSampleBAT;
   // data
   casa::Vector<casa::Complex> itsData;   
   int itsAnt;
   int itsChan;
   int itsNBeam;
};

long Worker::theirSampleBAT = 0;
boost::condition_variable_any Worker::theirSampleTrigger;
boost::shared_mutex Worker::theirSampleTriggerMutex;

// Main function
int main(int argc, const char** argv)
{
    // This class must have scope outside the main try/catch block
    askap::mwcommon::AskapParallel comms(argc, argv);

    try {
       casa::Timer timer;
       timer.mark();
    
       const float samplingRate = 32./27.*1e6; // in samples per second
       casa::Vector<casa::Complex> buf1;
       casa::Vector<casa::Complex> buf2;
       acquire(buf1,buf2,5.2e-6,BufferManager::NumberOfSamples(),samplingRate);
       // assume that antenna1 = antenna3 for this simple test

       ASKAPLOG_INFO_STR(logger, "initialisation of dummy data "<<"user:   " << timer.user() << " system: " << timer.system()
                                      << " real:   " << timer.real());
       timer.mark();
       // connection to the correlator server comes here along with the threading stuff
       const int nBeam = 1;
       const int nChan = 1;
       
       boost::thread_group threads;
       for (int cnt = 0; cnt<nChan; ++cnt) {
            threads.create_thread(Worker(buf1, 0, cnt, nBeam));
            threads.create_thread(Worker(buf2, 1, cnt, nBeam));
            //threads.create_thread(Worker(buf1, 2, cnt, nBeam));
       }
       
       for (size_t cycle = 0; cycle < 10; ++cycle) {
            ASKAPLOG_INFO_STR(logger, "cycle "<<cycle);
            Worker::triggerSample(long(time(0)));
            sleep(1);
       }
       threads.interrupt_all();
       ASKAPLOG_INFO_STR(logger, "Waiting to finish");
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
    
