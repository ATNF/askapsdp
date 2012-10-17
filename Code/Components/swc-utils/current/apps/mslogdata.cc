/// @file 
///
/// @brief utility to log the content of the measurement set via basic monitor
/// @details Although we could've done the same using casa, it is handy to have a
/// specialised routine using our own classes (and it can later be used as a template
/// for the code of other similar utilities requiring iteration over the data.
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

#include <dataaccess/TableDataSource.h>
#include <askap_accessors.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, "");

#include <askap/AskapError.h>
#include <dataaccess/SharedIter.h>

#include <dataaccess/TableManager.h>
#include <dataaccess/IDataConverterImpl.h>

#include <casa/OS/Timer.h>
#include <casa/Arrays/ArrayMath.h>

// std
#include <stdexcept>
#include <iostream>
#include <vector>
#include <iomanip>

using namespace askap;
using namespace askap::accessors;

void process(const IConstDataSource &ds, size_t nAvg) {
  IDataSelectorPtr sel=ds.createSelector();
  sel->chooseFeed(1);
  IDataConverterPtr conv=ds.createConverter();  
  conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),"MHz");
  conv->setEpochFrame(casa::MEpoch(casa::Quantity(55913.0,"d"),
                      casa::MEpoch::Ref(casa::MEpoch::UTC)),"s");
  conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));                    
  size_t counter = 0;
  casa::Vector<casa::Complex> buf(3, casa::Complex(0.,0.));
  casa::uInt nChan = 0;
  double startTime = 0;
    
  for (IConstDataSharedIter it=ds.createConstIterator(sel,conv);it!=it.end();++it) {  
       if (nChan == 0) {
           nChan = it->nChannel();
           startTime = it->time();
       } else { 
           ASKAPCHECK(nChan == it->nChannel(), 
                  "Number of channels seem to have been changed, previously "<<nChan<<" now "<<it->nChannel());
       }
       ASKAPCHECK(it->nRow() == 3, "Expect 3 baselines, the accessor has "<<it->nRow()<<" rows");
       ASKAPASSERT(it->nPol() >= 1);
       ASKAPASSERT(it->nChannel() >= 1);
       // we require that 3 baselines come in certain order, so we can hard code conjugation for calculation
       // of the closure phase
       ASKAPCHECK(it->antenna2()[0] == it->antenna1()[1], "Expect baselines in the order 1-2,2-3 and 1-3");
       ASKAPCHECK(it->antenna1()[0] == it->antenna1()[2], "Expect baselines in the order 1-2,2-3 and 1-3");
       ASKAPCHECK(it->antenna2()[1] == it->antenna2()[2], "Expect baselines in the order 1-2,2-3 and 1-3");
       //
       casa::Vector<casa::Complex> freqAvBuf(3, casa::Complex(0.,0.));
       for (casa::uInt ch=0; ch<it->nChannel(); ++ch) {
            freqAvBuf += it->visibility().xyPlane(0).column(ch);
       }
       freqAvBuf /= float(it->nChannel());
       buf += freqAvBuf;
       if (counter == 0) {
           startTime = it->time();
       }
       
       if (++counter == nAvg) {
           buf /= float(nAvg);
           // publish buf
           buf.set(casa::Complex(0.,0.));
           counter = 0;
       }
       //cout<<"time: "<<it->time()<<endl;
  }
  if (counter!=0) {
      buf /= float(counter);
      // publish buf
  }
}


int main(int argc, char **argv) {
  try {
     if (argc!=2) {
         std::cerr<<"Usage "<<argv[0]<<" measurement_set"<<std::endl;
	 return -2;
     }

     casa::Timer timer;

     timer.mark();
     TableDataSource ds(argv[1],TableDataSource::MEMORY_BUFFERS);     
     std::cerr<<"Initialization: "<<timer.real()<<std::endl;
     timer.mark();
     // number of cycles to average
     const size_t nAvg = 1;
     process(ds, nAvg);
     std::cerr<<"Job: "<<timer.real()<<std::endl;
     
  }
  catch(const AskapError &ce) {
     std::cerr<<"AskapError has been caught. "<<ce.what()<<std::endl;
     return -1;
  }
  catch(const std::exception &ex) {
     std::cerr<<"std::exception has been caught. "<<ex.what()<<std::endl;
     return -1;
  }
  catch(...) {
     std::cerr<<"An unexpected exception has been caught"<<std::endl;
     return -1;
  }
  return 0;
}