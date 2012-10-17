/// @file
/// @brief an utility to fit for antenna locations and associated tests
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


#include <dataaccess/TableDataSource.h>
#include <askap_accessors.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, "");

#include <askap/AskapError.h>
#include <dataaccess/SharedIter.h>

#include <dataaccess/TableManager.h>
#include <dataaccess/IDataConverterImpl.h>

// casa
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MCEpoch.h>
#include <measures/Measures/MeasConvert.h>
#include <tables/Tables/Table.h>
#include <casa/OS/Timer.h>
#include <casa/Arrays/ArrayMath.h>



// std
#include <stdexcept>
#include <iostream>
#include <vector>
#include <iomanip>

using std::cout;
using std::cerr;
using std::endl;

using namespace askap;
using namespace askap::accessors;

void publish(std::ostream &os, const casa::Vector<casa::Complex> &vis, double startTime, double avgTime, const casa::MVDirection &dir, const casa::Vector<double> &wBuf)
{
   ASKAPDEBUGASSERT(wBuf.nelements() == 3);
   const casa::MEpoch epoch(casa::Quantity(56100.0+avgTime/86400.,"d"), casa::MEpoch::Ref(casa::MEpoch::UTC));
   const double ra = dir.getValue()(0);
   const double dec = dir.getValue()(1);
   const double gmstInDays = casa::MEpoch::Convert(epoch,casa::MEpoch::Ref(casa::MEpoch::GMST1))().get("d").getValue("d");
   const double gmst = (gmstInDays - casa::Int(gmstInDays)) * casa::C::_2pi; // in radians
  
   const double H0 = gmst - ra, sH0 = sin(H0), cH0 = cos(H0), sd = sin(dec), cd = cos(dec);
  

   os<<std::scientific<<std::setprecision(15)<<startTime<<" "<<avgTime<<" "<<std::fixed<<std::setprecision(6);
   for (casa::uInt baseline = 0; baseline<vis.nelements(); ++baseline) {
        os<<" "<<std::fixed<<arg(vis[baseline])/casa::C::pi*180.<<" "<<std::setprecision(15)<<std::scientific<<wBuf[baseline];
   }
   os<<" "<<std::fixed<<-cH0*cd<<" "<<sH0*cd<<" "<<-sd<<" "<<H0/casa::C::pi*180.<<std::endl;
}

void process(const IConstDataSource &ds, size_t nAvg) {
  IDataSelectorPtr sel=ds.createSelector();
  //sel->chooseBaseline(0,1);
  sel->chooseFeed(0);
  IDataConverterPtr conv=ds.createConverter();  
  conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),"MHz");
  conv->setEpochFrame(casa::MEpoch(casa::Quantity(56100.0,"d"),
                      casa::MEpoch::Ref(casa::MEpoch::UTC)),"s");
  conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));                    
  size_t counter = 0;
  casa::Vector<casa::Complex> buf(3, casa::Complex(0.,0.));
  casa::uInt nChan = 0;
  double startTime = 0;
  double avgTime = 0;
  casa::Vector<double> wBuf(3,0.);
  casa::MVDirection dir;
    
  std::ofstream os("result.dat");
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
           dir = it->pointingDir1()[0];
           for (casa::uInt row = 0; row<it->nRow(); ++row) {
               wBuf[row] = it->uvw()[row](2) / casa::C::c * 2 * casa::C::pi;
           }
       }
       for (casa::uInt row=0; row<it->nRow(); ++row) {
            ASKAPCHECK(dir.separation(it->pointingDir1()[row])<1e-6, "Pointing/phase centre differs for row="<<row<<" time="<<it->time());
            ASKAPCHECK(dir.separation(it->pointingDir2()[row])<1e-6, "Pointing/phase centre differs for row="<<row<<" time="<<it->time());
       }
       avgTime += it->time();
       
       if (++counter == nAvg) {
           buf /= float(nAvg);
           avgTime /= double(nAvg);
           publish(os, buf, startTime, avgTime,dir,wBuf);
           buf.set(casa::Complex(0.,0.));
           counter = 0;
           avgTime = 0.;
       }
       //cout<<"time: "<<it->time()<<endl;
  }
  if (counter!=0) {
      buf /= float(counter);
      avgTime /= double(counter);
      publish(os, buf, startTime, avgTime,dir,wBuf);
  }
}


int main(int argc, char **argv) {
  try {
     if (argc!=2) {
         cerr<<"Usage "<<argv[0]<<" measurement_set"<<endl;
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
     cerr<<"AskapError has been caught. "<<ce.what()<<endl;
     return -1;
  }
  catch(const std::exception &ex) {
     cerr<<"std::exception has been caught. "<<ex.what()<<endl;
     return -1;
  }
  catch(...) {
     cerr<<"An unexpected exception has been caught"<<endl;
     return -1;
  }
  return 0;
}
