/// @file
/// @brief an utility to "calibrate" 3-antenna experiment with the sw-correlation
/// @details The number of measurements is not enough to do a proper calibration.
/// This is why the ccalibrator cannot be used. However, we can align the data to
/// get a basic effect of the calibration and also optionally adjust amplitudes assuming
/// a strong source has been observed
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


#include <dataaccess/TableDataSource.h>
#include <askap_accessors.h>
#include <askap/AskapLogging.h>
#include <askap/AskapUtil.h>
ASKAP_LOGGER(logger, "");

#include <askap/AskapError.h>
#include <dataaccess/SharedIter.h>

#include <dataaccess/TableManager.h>
#include <dataaccess/IDataConverterImpl.h>
#include <swcorrelator/BasicMonitor.h>

// casa
#include <measures/Measures/MFrequency.h>
#include <tables/Tables/Table.h>
#include <casa/OS/Timer.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>



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

void process(const IConstDataSource &ds, const int ctrl = -1) {
  IDataSelectorPtr sel=ds.createSelector();
  //sel->chooseFeed(1);
  if (ctrl >=0 ) {
      sel->chooseUserDefinedIndex("CONTROL",casa::uInt(ctrl));
  }
  IDataConverterPtr conv=ds.createConverter();  
  conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),"MHz");
  conv->setEpochFrame(casa::MEpoch(casa::Quantity(55913.0,"d"),
                      casa::MEpoch::Ref(casa::MEpoch::UTC)),"s");
  conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));                    
  casa::Matrix<casa::Complex> buf;
  casa::Vector<double> freq;
  size_t counter = 0;
  size_t nGoodRows = 0;
  size_t nBadRows = 0;
  casa::uInt nChan = 0;
  casa::uInt nRow = 0;
  double startTime = 0;
  double stopTime = 0;
    
  for (IConstDataSharedIter it=ds.createConstIterator(sel,conv);it!=it.end();++it) {  
       if (nChan == 0) {
           nChan = it->nChannel();
           nRow = it->nRow();
           buf.resize(nRow,nChan);
           buf.set(casa::Complex(0.,0.));
           freq = it->frequency();
       } else { 
           ASKAPCHECK(nChan == it->nChannel(), 
                  "Number of channels seem to have been changed, previously "<<nChan<<" now "<<it->nChannel());
           ASKAPCHECK(nRow == it->nRow(), 
                  "Number of rows seem to have been changed, previously "<<nRow<<" now "<<it->nRow());
       }
       
       ASKAPASSERT(it->nPol() >= 1);
       ASKAPASSERT(it->nChannel() > 1);
       // we require that 3 baselines come in certain order, just to be sure
       for (casa::uInt row = 0; row<nRow; row+=3) {
            ASKAPCHECK(it->antenna2()[row] == it->antenna1()[row+1], "Expect baselines in the order 1-2,2-3 and 1-3");
            ASKAPCHECK(it->antenna1()[row] == it->antenna1()[row+2], "Expect baselines in the order 1-2,2-3 and 1-3");
            ASKAPCHECK(it->antenna2()[row+1] == it->antenna2()[row+2], "Expect baselines in the order 1-2,2-3 and 1-3");
       }
       //
       
       // add new spectrum to the buffer
       for (casa::uInt row=0; row<nRow; ++row) {
            casa::Vector<casa::Bool> flags = it->flag().xyPlane(0).row(row);
            bool flagged = false;
            for (casa::uInt ch = 0; ch < flags.nelements(); ++ch) {
                 flagged |= flags[ch];
            }
            if (flagged) {
               ++nBadRows;
            } else {
                casa::Vector<casa::Complex> thisRow = buf.row(row);
                thisRow += it->visibility().xyPlane(0).row(row);
                ++nGoodRows;
            }
       }
       if ((counter == 0) && (nGoodRows == 0)) {
           // all data are flagged, completely ignoring this iteration and consider the next one to be first
           nChan = 0;
           continue;
       }
      
       if (++counter == 1) {
           startTime = it->time();
       }
       stopTime = it->time() + 1; // 1s integration time is hardcoded
  }
  if (counter!=0) {
      buf /= float(counter);
      std::cout<<"Averaged "<<counter<<" integration cycles, "<<nGoodRows<<" good and "<<nBadRows<<" bad rows, time span "<<(stopTime-startTime)/60.<<" minutues"<<std::endl;
      { // export averaged spectrum
        ASKAPDEBUGASSERT(freq.nelements() == nChan);
        std::ofstream os("avgspectrum.dat");
        for (casa::uInt chan=0; chan<nChan; ++chan) {
             os<<chan<<" "<<freq[chan];
             for (casa::uInt row=0; row<nRow; ++row) {
                  os<<" "<<casa::abs(buf(row,chan))<<" "<<casa::arg(buf(row,chan))/casa::C::pi*180.;
             }
             os<<std::endl;
        }
      }
      // if flux is positive, adjust amplitudes to get the desired flux
      //const float target_flux = -1;

      ASKAPCHECK(buf.ncolumn()>0, "Need at least 1 spectral channel!");
      std::ofstream os("roughcalib.in");
      for (casa::uInt row = 0; row<buf.nrow(); row+=3) {
           ASKAPDEBUGASSERT(row+2<buf.nrow());
           casa::Vector<casa::Complex> spAvg(3,casa::Complex(0.,0.));
           for (casa::uInt baseline = 0; baseline<spAvg.nelements(); ++baseline) {
                casa::Vector<casa::Complex> thisRow = buf.row(row+baseline);
                spAvg[baseline] = casa::sum(thisRow);
                spAvg[baseline] /= float(buf.ncolumn());
           }
           const float ph1 = -arg(spAvg[0]);
           const float ph2 = -arg(spAvg[2]);
           const float closurePh = arg(spAvg[0]*spAvg[1]*conj(spAvg[2]));
           
           const casa::uInt beam = row/3;
           os<<"# Beam "<<beam<<" closure phase: "<<closurePh/casa::C::pi*180.<<" deg"<<std::endl;
           os<<"gain.g11.0."<<beam<<" [1.0,0.0]"<<std::endl;
           os<<"gain.g22.0."<<beam<<" [1.0,0.0]"<<std::endl;
           os<<"gain.g11.1."<<beam<<" ["<<cos(ph1)<<","<<sin(ph1)<<"]"<<std::endl;
           os<<"gain.g22.1."<<beam<<" ["<<cos(ph1)<<","<<sin(ph1)<<"]"<<std::endl;
           os<<"gain.g11.2."<<beam<<" ["<<cos(ph2)<<","<<sin(ph2)<<"]"<<std::endl;
           os<<"gain.g22.2."<<beam<<" ["<<cos(ph2)<<","<<sin(ph2)<<"]"<<std::endl;
      }
  } else {
     std::cout<<"No data found!"<<std::endl;
  }
}


int main(int argc, char **argv) {
  try {
     if ((argc!=2) && (argc!=3)) {
         cerr<<"Usage: "<<argv[0]<<" [ctrl] measurement_set"<<endl;
	 return -2;
     }

     casa::Timer timer;
     const std::string msName = argv[argc - 1];
     const int ctrl = argc == 2 ? -1 : utility::fromString<int>(argv[1]);

     timer.mark();
     TableDataSource ds(msName,TableDataSource::MEMORY_BUFFERS);     
     std::cerr<<"Initialization: "<<timer.real()<<std::endl;
     timer.mark();
     process(ds,ctrl);
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
