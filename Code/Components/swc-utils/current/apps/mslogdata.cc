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
#include <dataaccess/IConstDataAccessor.h>

#include <askap_accessors.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, "");

#include <askap/AskapError.h>
#include <dataaccess/SharedIter.h>

#include <dataaccess/TableManager.h>
#include <dataaccess/IDataConverterImpl.h>

#include <swcorrelator/DataMonitors.h>
#include <swcorrelator/CorrProducts.h>

#include <casa/OS/Timer.h>
#include <casa/Arrays/ArrayMath.h>

#include <Common/ParameterSet.h>

// std
#include <stdexcept>
#include <iostream>
#include <vector>
#include <iomanip>

// boost
#include <boost/shared_ptr.hpp>

using namespace askap;
using namespace askap::accessors;

struct DataLogger {
  
  explicit DataLogger(size_t nAvg) : itsNAvg(nAvg), itsStartTime(0.)  {}

  void setupMonitor(const IConstDataAccessor &acc);
  
  void reset();
  
  void process(const IConstDataSource &ds);
  
protected:
  void publish();
  
private:

  // data monitors
  boost::shared_ptr<swcorrelator::DataMonitors> itsMonitors;
  
  // buffers for data per beam
  std::vector<boost::shared_ptr<swcorrelator::CorrProducts> > itsCorrProducts;

  // number of cycles to average
  size_t itsNAvg;
  // time
  double itsStartTime;
  // buffer
  casa::Cube<casa::Complex> itsBuffer;
  // antenna 1 indices
  casa::Vector<casa::uInt> itsAnt1IDs;
  // antenna 2 indices
  casa::Vector<casa::uInt> itsAnt2IDs;
  // beam 1 indices
  casa::Vector<casa::uInt> itsBeam1IDs;
  // beam 2 indices
  casa::Vector<casa::uInt> itsBeam2IDs;
     
};

void DataLogger::setupMonitor(const IConstDataAccessor &acc)
{
  LOFAR::ParameterSet parset;
  parset.add("monitors","basic");
  itsMonitors.reset(new swcorrelator::DataMonitors(parset));
  
  const casa::uInt maxAntID = casa::max(casa::max(acc.antenna1()),casa::max(acc.antenna2()));
  const casa::uInt maxBeamID = casa::max(casa::max(acc.feed1()),casa::max(acc.feed2()));
  const int nBeam = int(maxBeamID)+1;
  
  itsMonitors->initialise(int(maxAntID)+1, nBeam, acc.nChannel());  
  
  itsCorrProducts.resize(nBeam);
  for (int beam=0;beam<nBeam; ++beam) {
       itsCorrProducts[beam].reset(new swcorrelator::CorrProducts(int(acc.nChannel()),beam));
  }
}


void DataLogger::reset()
{
  itsAnt1IDs.resize(0);
  itsAnt2IDs.resize(0);
  itsBeam1IDs.resize(0);
  itsBeam2IDs.resize(0);
  itsBuffer.resize(0,0,0);
  itsStartTime=0;
  itsMonitors.reset();
}

void DataLogger::publish() {
  ASKAPDEBUGASSERT(itsMonitors);
  for (int beam=0; beam<int(itsCorrProducts.size()); ++beam) {
       const boost::shared_ptr<swcorrelator::CorrProducts> &cp = itsCorrProducts[beam];
       ASKAPDEBUGASSERT(cp);
       
       cp->init(uint64_t(itsStartTime));
       
       for (casa::uInt row=0; row<itsBuffer.nrow(); ++row) {
            ASKAPCHECK(itsBeam1IDs[row] == itsBeam2IDs[row], "Cross-beam correlations are not supported");
            if (int(itsBeam1IDs[row]) != beam) {
                continue;
            }
            int baseline = -1;
            if ((itsAnt1IDs[row] == 0) && (itsAnt2IDs[row] == 1)) {
                 baseline = 0;
            } else if ((itsAnt1IDs[row] == 1) && (itsAnt2IDs[row] == 2)) {
                 baseline = 1;
            } else if ((itsAnt1IDs[row] == 0) && (itsAnt2IDs[row] == 2)) {
                 baseline = 2;
            } else {
               ASKAPTHROW(AskapError,"Unknown baseline "<<itsAnt1IDs[row]<<" - "<<itsAnt2IDs[row]<<" for row "<<row);
            }
            ASKAPDEBUGASSERT(baseline >= 0);
            cp->itsVisibility.row(baseline) = itsBuffer.xyPlane(0).row(row);
            cp->itsFlag.row(baseline).set(false);
       } 
       itsMonitors->publish(*cp);
  }
  itsMonitors->finalise();
}

void DataLogger::process(const IConstDataSource &ds) {
  IDataSelectorPtr sel=ds.createSelector();
  //sel->chooseFeed(1);
  IDataConverterPtr conv=ds.createConverter();  
  conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),"MHz");
  conv->setEpochFrame(casa::MEpoch(casa::Quantity(0.,"d"),
                      casa::MEpoch::Ref(casa::MEpoch::TAI)),"us");
  conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));                    
  size_t counter = 0;
      
  for (IConstDataSharedIter it=ds.createConstIterator(sel,conv);it!=it.end();++it) {  
       if (!itsMonitors) {
          setupMonitor(*it);
       }
       if (itsBuffer.nelements() == 0) {
           itsBuffer.resize(it->nRow(),it->nChannel(),it->nPol());
           itsBuffer.set(casa::Complex(0.,0.));
           itsStartTime = it->time();
           itsAnt1IDs = it->antenna1().copy();
           itsAnt2IDs = it->antenna2().copy();
           itsBeam1IDs = it->feed1().copy();
           itsBeam2IDs = it->feed2().copy();           
       } else { 
           ASKAPCHECK(itsBuffer.shape() == it->visibility().shape(), 
                  "The shape of the visibility cube seems to have been changed, previously "<<itsBuffer.shape()<<
                  " now "<<it->visibility().shape());
           ASKAPDEBUGASSERT(itsAnt1IDs.nelements() == it->nRow());
           ASKAPDEBUGASSERT(itsAnt2IDs.nelements() == it->nRow());
           ASKAPDEBUGASSERT(itsBeam1IDs.nelements() == it->nRow());
           ASKAPDEBUGASSERT(itsBeam2IDs.nelements() == it->nRow());
           for (casa::uInt row = 0; row<it->nRow(); ++row) {
                ASKAPCHECK(itsAnt1IDs[row] == it->antenna1()[row], "Mismatch of antenna 1 index for row "<<row<<
                           " - got "<<it->antenna1()[row]<<" expected "<<itsAnt1IDs[row]);
                ASKAPCHECK(itsAnt2IDs[row] == it->antenna2()[row], "Mismatch of antenna 2 index for row "<<row<<
                           " - got "<<it->antenna2()[row]<<" expected "<<itsAnt2IDs[row]);
                ASKAPCHECK(itsBeam1IDs[row] == it->feed1()[row], "Mismatch of beam index (for the 1st antenna) for row "<<row<<
                           " - got "<<it->feed1()[row]<<" expected "<<itsBeam1IDs[row]);
                ASKAPCHECK(itsBeam2IDs[row] == it->feed2()[row], "Mismatch of beam index (for the 2nd antenna) for row "<<row<<
                           " - got "<<it->feed2()[row]<<" expected "<<itsBeam2IDs[row]);
           }       
       }
       ASKAPASSERT(it->nPol() >= 1);
       ASKAPASSERT(it->nChannel() >= 1);
       
       itsBuffer += it->visibility();
       if (counter == 0) {
           itsStartTime = it->time();          
       }
       
       if (++counter == itsNAvg) {
           itsBuffer /= float(itsNAvg);
           // publish buffer
           publish();
           //
           itsBuffer.set(casa::Complex(0.,0.));
           counter = 0;
       }
       //cout<<"time: "<<it->time()<<endl;
  }
  if (counter!=0) {
      itsBuffer /= float(counter);
      // publish buffer
      publish();
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
     DataLogger dl(nAvg);
     dl.process(ds);
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