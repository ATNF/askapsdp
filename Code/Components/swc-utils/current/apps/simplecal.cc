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

std::string printComplex(const casa::Complex &val) {
  return std::string("[")+utility::toString<float>(casa::real(val))+" , "+utility::toString<float>(casa::imag(val))+"]";
}

void process(const IConstDataSource &ds, const float flux, const int ctrl = -1) {
  IDataSelectorPtr sel=ds.createSelector();
  sel->chooseCrossCorrelations();
  //sel->chooseFeed(7);
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

  casa::Vector<casa::uInt> ant1IDs;
  casa::Vector<casa::uInt> ant2IDs;
    
  // the assumed baseline order depends on this parameter
  const bool useSWCorrelator = false;

  casa::uInt cPol = 3;
  for (IConstDataSharedIter it=ds.createConstIterator(sel,conv);it!=it.end();++it) {  
       // for every iteration we first build an index into all unflagged rows
       std::vector<casa::uInt> rowIndex;
       rowIndex.reserve(it->nRow());
       ASKAPASSERT(cPol<it->nPol());
       for (casa::uInt row = 0; row<it->nRow(); ++row) {
            casa::Vector<casa::Bool> flags = it->flag().xyPlane(cPol).row(row);
      
            bool flagged = false;
            for (casa::uInt ch = 0; ch < flags.nelements(); ++ch) {
                 flagged |= flags[ch];
            }
            if (!flagged) {
                rowIndex.push_back(row);
            }
       }
       if (nChan == 0) {
           nChan = it->nChannel();
           nRow = rowIndex.size();//it->nRow();
           buf.resize(nRow,nChan);
           buf.set(casa::Complex(0.,0.));
           freq = it->frequency();
           ant1IDs = it->antenna1().copy();
           ant2IDs = it->antenna2().copy();
       } else { 
           ASKAPCHECK(nChan == it->nChannel(), 
                  "Number of channels seem to have been changed, previously "<<nChan<<" now "<<it->nChannel());
           //ASKAPCHECK(nRow == rowIndex.size(), 
           //       "Number of rows seem to have been changed, previously "<<nRow<<" now "<<it->nRow());
           if (nRow != rowIndex.size()) {
               std::cerr<<"Number of unflagged rows has been changed, initially "<<nRow<<" now "<<rowIndex.size()<<", integration cycle = "<<counter+1<<std::endl;
               continue;
           }
           //
           ASKAPDEBUGASSERT(ant1IDs.nelements() == it->nRow());
           ASKAPDEBUGASSERT(ant2IDs.nelements() == it->nRow());
           for (casa::uInt row = 0; row<it->nRow(); ++row) {
                ASKAPCHECK(ant1IDs[row] == it->antenna1()[row], "Mismatch of antenna 1 index for row "<<row<<
                           " - got "<<it->antenna1()[row]<<" expected "<<ant1IDs[row]);
                ASKAPCHECK(ant2IDs[row] == it->antenna2()[row], "Mismatch of antenna 2 index for row "<<row<<
                           " - got "<<it->antenna2()[row]<<" expected "<<ant2IDs[row]);
           }
       }
       
       ASKAPASSERT(it->nPol() >= 1);
       ASKAPASSERT(it->nChannel() > 1);
       // we require that 3 baselines come in certain order, so we can hard code conjugation for calculation
       // of the closure phase.
       // the order is different for software and hardware correlator. Just hard code the differences
       for (casa::uInt validRow = 0; validRow<nRow; validRow+=3) {
            const casa::uInt row = rowIndex[validRow];
            if (useSWCorrelator) {
                ASKAPCHECK(it->antenna2()[row] == it->antenna1()[row+1], "Expect baselines in the order 1-2,2-3 and 1-3");
                ASKAPCHECK(it->antenna1()[row] == it->antenna1()[row+2], "Expect baselines in the order 1-2,2-3 and 1-3");
                ASKAPCHECK(it->antenna2()[row+1] == it->antenna2()[row+2], "Expect baselines in the order 1-2,2-3 and 1-3");
             } else {
                ASKAPCHECK(it->antenna2()[row] == it->antenna1()[row+2], "Expect baselines in the order 1-2,1-3 and 2-3");
                ASKAPCHECK(it->antenna1()[row] == it->antenna1()[row+1], "Expect baselines in the order 1-2,1-3 and 2-3");
                ASKAPCHECK(it->antenna2()[row+1] == it->antenna2()[row+2], "Expect baselines in the order 1-2,1-3 and 2-3");
             }
       }
       //
       
       // add new spectrum to the buffer
       for (casa::uInt validRow=0; validRow<nRow; ++validRow) {
            const casa::uInt row = rowIndex[validRow];
            casa::Vector<casa::Bool> flags = it->flag().xyPlane(cPol).row(row);
      
            bool flagged = false;
            /*
            // to ensure nothing is flagged
            for (casa::uInt ch = 0; ch < flags.nelements(); ++ch) {
                 // to ensure nothing is flagged
                 flagged |= flags[ch];
            }
            */
            if (flagged) {
               ++nBadRows;
            } else {
                casa::Vector<casa::Complex> thisRow = buf.row(validRow);
                //thisRow += it->visibility().xyPlane(cPol).row(row);
                for (casa::uInt ch = 0; ch<thisRow.nelements(); ++ch) {
                     if (!flags[ch]) {
                         thisRow[ch] += it->visibility()(row,ch,cPol);
                     }
                }
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
       stopTime = it->time() + (useSWCorrelator ? 1 : 5); // 1s or 5s integration time is hardcoded
       /*
       if (counter == 3) {
           break;
       }
       */
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

      ASKAPCHECK(buf.ncolumn()>0, "Need at least 1 spectral channel!");
      std::ofstream os("roughcalib.in");
      if (flux>0) {
          os<<"# amplitudes adjusted to match flux = "<<flux<<" Jy of the 'calibrator'"<<std::endl;
      } else {
          os<<"# all gain amplitudes are 1."<<std::endl;
      }
      for (casa::uInt row = 0; row<buf.nrow(); row+=3) {
           ASKAPDEBUGASSERT(row+2<buf.nrow());
           casa::Vector<casa::Complex> spAvg(3,casa::Complex(0.,0.));
           for (casa::uInt baseline = 0; baseline<spAvg.nelements(); ++baseline) {
                casa::Vector<casa::Complex> thisRow = buf.row(row+baseline);
                //casa::Vector<casa::Complex> thisRow = buf.row(row+baseline)(casa::Slice(38,32));
                spAvg[baseline] = casa::sum(thisRow);
                spAvg[baseline] /= float(buf.ncolumn());
           }
           if (!useSWCorrelator) {
              // the hw-correlator has a different baseline order: 0-1, 0-2 and 1-2, we need to swap last two baselines to get 0-1,1-2,0-2 everywhere 
              const casa::Complex tempBuf = spAvg[2];
              spAvg[2] = spAvg[1];
              spAvg[1] = tempBuf;
           }
           const float ph1 = -arg(spAvg[0]);
           const float ph2 = -arg(spAvg[2]);
           const float closurePh = arg(spAvg[0]*spAvg[1]*conj(spAvg[2]));
           
           const casa::uInt beam = row/3;
           os<<"# Beam "<<beam<<" closure phase: "<<closurePh/casa::C::pi*180.<<" deg"<<std::endl;
           os<<"# measured phases              (0-1,1-2,0-2): "<<arg(spAvg[0])/casa::C::pi*180.<<" "<<arg(spAvg[1])/casa::C::pi*180.<<" "<<arg(spAvg[2])/casa::C::pi*180.<<std::endl;
           os<<"# measured amplitudes          (0-1,1-2,0-2): "<<casa::abs(spAvg[0])<<" "<<casa::abs(spAvg[1])<<" "<<casa::abs(spAvg[2])<<std::endl;
           float amp0 = 1.;
           float amp1 = 1.;
           float amp2 = 1.;
           if (flux > 0) {
               ASKAPCHECK((casa::abs(spAvg[0])> 1e-6) && (casa::abs(spAvg[1])> 1e-6) && (casa::abs(spAvg[2])> 1e-6), "One of the measured amplitudes is too close to 0.: "<<spAvg);
               amp0 = sqrt(casa::abs(spAvg[2]) * casa::abs(spAvg[0]) / casa::abs(spAvg[1]) / flux);
               amp1 = sqrt(casa::abs(spAvg[1]) * casa::abs(spAvg[0]) / casa::abs(spAvg[2]) / flux);
               amp2 = sqrt(casa::abs(spAvg[2]) * casa::abs(spAvg[1]) / casa::abs(spAvg[0]) / flux);
           }
           const casa::Complex g0(amp0,0.);
           const casa::Complex g1 = casa::Complex(cos(ph1),sin(ph1)) * amp1;
           const casa::Complex g2 = casa::Complex(cos(ph2),sin(ph2)) * amp2;

           os<<"# phases after calibration     (0-1,1-2,0-2): "<<arg(spAvg[0]/g0/conj(g1))/casa::C::pi*180.<<" "<<arg(spAvg[1]/g1/conj(g2))/casa::C::pi*180.<<" "<<arg(spAvg[2]/g0/conj(g2))/casa::C::pi*180.<<std::endl;
           os<<"# amplitudes after calibration (0-1,1-2,0-2): "<<casa::abs(spAvg[0]/g0/conj(g1))<<" "<<casa::abs(spAvg[1]/g1/conj(g2))<<" "<<casa::abs(spAvg[2]/g0/conj(g2))<<std::endl;

           os<<"gain.g11.0."<<beam<<" = "<<printComplex(g0)<<std::endl;
           os<<"gain.g22.0."<<beam<<" = "<<printComplex(g0)<<std::endl;
           os<<"gain.g11.1."<<beam<<" = "<<printComplex(g1)<<std::endl;
           os<<"gain.g22.1."<<beam<<" = "<<printComplex(g1)<<std::endl;
           os<<"gain.g11.2."<<beam<<" = "<<printComplex(g2)<<std::endl;
           os<<"gain.g22.2."<<beam<<" = "<<printComplex(g2)<<std::endl;
      }
  } else {
     std::cout<<"No data found!"<<std::endl;
  }
}


int main(int argc, char **argv) {
  try {
     if ((argc!=2) && (argc!=3)) {
         cerr<<"Usage: "<<argv[0]<<" [flux] measurement_set"<<endl;
	 return -2;
     }

     casa::Timer timer;
     const std::string msName = argv[argc - 1];
     const float flux = argc == 2 ? -1 : utility::fromString<float>(argv[1]);

     timer.mark();
     TableDataSource ds(msName,TableDataSource::MEMORY_BUFFERS);     
     std::cerr<<"Initialization: "<<timer.real()<<std::endl;
     timer.mark();
     process(ds,flux);
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
