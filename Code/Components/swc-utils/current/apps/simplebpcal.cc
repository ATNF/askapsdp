/// @file
/// @brief an utility to "calibrate" 3-antenna experiment
/// @details The number of measurements is not enough to do a proper calibration.
/// This is why the ccalibrator cannot be used. However, we can align the data to
/// get a basic effect of the calibration and also optionally adjust amplitudes assuming
/// a strong source has been observed. This tool is similar to simplecal, but does
/// frequency-dependent calibration. It also takes into account flagging specific to
/// the current MRO system.
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

/// @brief flux model for 1934-638
/// @details This method estimates the flux density of 1934-638 for a given frequency using the cm-wavelength 
/// model of Reynolds et al. (see miriad or query 1934-638 in the ATCA calibrator database for a
/// reference).   
/// @param[in] freqInMHz frequency of interest (in MHz)
/// @return estimated flux density in Jy
double get1934fluxDensity(const double freqInMHz)
{
  ASKAPCHECK( (freqInMHz > 500) && (freqInMHz < 10000), 
      "The flux model of 1934-638 is only valid from 500 MHz to 10 GHz, you have freq = "<<freqInMHz<<" MHz");
  const double lgF = log(freqInMHz) / log(10.);
  // polynomial fit
  const double lgS = -30.7667 + (26.4908 - (7.0977 - 0.6053334 * lgF) * lgF) * lgF;
  return exp(lgS * log(10.));
}  


std::string printComplex(const casa::Complex &val) {
  return std::string("[")+utility::toString<float>(casa::real(val))+" , "+utility::toString<float>(casa::imag(val))+"]";
}

// cubes -> baseline x channel x pol (are resized inside)
// baseline order is 1-2,1-3 and 2-3, two polarisations, number of channels are set when the first data point is sighted
void makeAvgSpectra(const IConstDataSource &ds, const casa::uInt beam, casa::Cube<casa::Complex> &spc, casa::Cube<casa::Bool> &flags, casa::Vector<double> &freq) {
  double skipAtStart = 135; // number of seconds to skip at start of the file (FR settling)

  IDataSelectorPtr sel=ds.createSelector();
  sel->chooseCrossCorrelations();
  sel->chooseFeed(beam);
  IDataConverterPtr conv=ds.createConverter();  
  conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),"MHz");
  conv->setEpochFrame(casa::MEpoch(casa::Quantity(0.,"d"),
                      casa::MEpoch::Ref(casa::MEpoch::UTC)),"s");
  conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));                    
  size_t counter = 0;
  double startTime = 0;
  double stopTime = 0;
  // same shape as flags and spc
  casa::Cube<casa::uInt> counters;

  for (IConstDataSharedIter it=ds.createConstIterator(sel,conv);it!=it.end();++it) {  
       if (++counter == 1) {
           startTime = it->time();
       }
       if (it->time() - startTime < skipAtStart) {
           continue;
       }
       if (counters.nrow() == 0) {
           // 3 baselines, two polarisations
           counters.resize(3, it->nChannel(), 2);
           counters.set(0u);
           spc.resize(3, it->nChannel(), 2);
           spc.set(casa::Complex(0.,0.));
           flags.resize(3, it->nChannel(), 2);
           flags.set(casa::True);
           freq.resize(it->nChannel());
           freq = it->frequency();
       } else { 
           ASKAPCHECK(counters.ncolumn() == it->nChannel(), 
                  "Number of channels seem to have been changed, previously "<<counters.ncolumn()<<" now "<<it->nChannel());
       }
       
       ASKAPASSERT(it->nPol() == 4);
       ASKAPASSERT(it->nChannel() > 1);

       for (casa::uInt row = 0; row<it->nRow(); ++row) {
            casa::uInt baselineIndex = 4;
            if ((it->antenna1()[row] == 0) && (it->antenna2()[row] == 1)) {
                baselineIndex = 0;
            } else if ((it->antenna1()[row] == 0) && (it->antenna2()[row] == 2)) {
                baselineIndex = 1;
            } else if ((it->antenna1()[row] == 1) && (it->antenna2()[row] == 2)) {
                baselineIndex = 2;
            } else {
                ASKAPLOG_WARN_STR(logger, "Unexpected baseline "<<it->antenna1()[row]<<" - "<<it->antenna2()[row]<<", ignoring");
                continue;
            }
            ASKAPDEBUGASSERT(baselineIndex < counters.nrow());
            for (casa::uInt pol = 0; pol < 2; ++pol) {
                 casa::uInt polIndex = (pol == 0 ? 0 : 3);
                 ASKAPDEBUGASSERT(polIndex < it->nPol());
                 casa::Vector<casa::Bool> visFlags = it->flag().xyPlane(polIndex).row(row);
                 casa::Vector<casa::Complex> vis = it->visibility().xyPlane(polIndex).row(row);
                 ASKAPDEBUGASSERT(vis.nelements() == it->nChannel());
                 for (casa::uInt ch = 0; ch < vis.nelements(); ++ch) {
                      if (!visFlags[ch]) {
                          spc(baselineIndex, ch, pol) += vis[ch];
                          flags(baselineIndex, ch, pol) = false;
                          counters(baselineIndex, ch, pol) += 1;
                      }
                 }
            }
       }

       ++counter;
       stopTime = it->time() + 5; // 5s integration time is hardcoded
  }
  if (counter!=0) {
      std::cout<<"Averaged "<<counter<<" integration cycles, time span "<<(stopTime-startTime)/60.<<" minutues"<<std::endl;
      // normalisation
      for (casa::uInt baseline = 0; baseline < counters.nrow(); ++baseline) {
           for (casa::uInt ch = 0; ch < counters.ncolumn(); ++ch) {
                for (casa::uInt pol = 0; pol < counters.nplane(); ++pol) {
                     if (!flags(baseline, ch, pol)) {
                         ASKAPDEBUGASSERT(counters(baseline,ch,pol) > 0);
                         spc(baseline,ch,pol) /= casa::Float(counters(baseline,ch,pol));
                     }
                }
           }
      }
  } else {
     std::cout<<"No data found!"<<std::endl;
  }
} 

// input - vector of visibilities (order: 1-2, 1-3, 2-3), freq - frequency in MHz, return: 4-vector with complex gains (antennas 0,1,2) +  closure phase in deg (real). 
casa::Vector<casa::Complex> processOne(const casa::Vector<casa::Complex> &vis, double freq)
{
   ASKAPASSERT(vis.nelements() == 3);
   casa::Vector<casa::Complex> result(4, casa::Complex(0.,0.));
   const double flux = get1934fluxDensity(freq);

   casa::Vector<casa::Complex> spAvg(3,casa::Complex(0.,0.));
   spAvg[0] = vis[0];
   spAvg[1] = vis[2];
   spAvg[2] = vis[1];

   const float ph1 = -arg(spAvg[0]);
   const float ph2 = -arg(spAvg[2]);
   const float closurePh = arg(spAvg[0]*spAvg[1]*conj(spAvg[2]));
           
   result[3] = casa::Complex(closurePh/casa::C::pi*180., 0.);
   float amp0 = 1.;
   float amp1 = 1.;
   float amp2 = 1.;
   if (flux > 0) {
       ASKAPCHECK((casa::abs(spAvg[0])> 1e-6) && (casa::abs(spAvg[1])> 1e-6) && (casa::abs(spAvg[2])> 1e-6), "One of the measured amplitudes is too close to 0.: "<<spAvg);
       amp0 = sqrt(casa::abs(spAvg[2]) * casa::abs(spAvg[0]) / casa::abs(spAvg[1]) / flux);
       amp1 = sqrt(casa::abs(spAvg[1]) * casa::abs(spAvg[0]) / casa::abs(spAvg[2]) / flux);
       amp2 = sqrt(casa::abs(spAvg[2]) * casa::abs(spAvg[1]) / casa::abs(spAvg[0]) / flux);
   }
   result[0] = casa::Complex(amp0,0.);
   result[1] = casa::Complex(cos(ph1),sin(ph1)) * amp1;
   result[2] = casa::Complex(cos(ph2),sin(ph2)) * amp2;
   return result;
}

void process(const std::vector<std::string> &fnames)
{
    for (size_t beam = 0; beam < fnames.size(); ++beam) {

         TableDataSource ds(fnames[beam],TableDataSource::MEMORY_BUFFERS);     
        
         casa::Cube<casa::Complex> spc;
         casa::Cube<casa::Bool> flags;
         casa::Vector<double> freq;
         makeAvgSpectra(ds, beam, spc, flags, freq);
         ASKAPDEBUGASSERT(spc.shape() == flags.shape());
         ASKAPDEBUGASSERT(freq.nelements() == flags.ncolumn());
         ASKAPDEBUGASSERT(spc.nrow() == 3);
         ASKAPDEBUGASSERT(spc.nplane() == 2);

         const std::string asciiFname = "result_beam"+utility::toString<size_t>(beam)+".dat";
         std::ofstream os(asciiFname.c_str());
         for (casa::uInt ch=0; ch < flags.ncolumn(); ++ch) {
              
              casa::Vector<casa::Complex> xxRes(4, casa::Complex(0.,0.));
              if (!flags(0,ch,0) && !flags(1,ch,0) && !flags(2,ch,0)) {
                   xxRes = processOne(spc.xyPlane(0).column(ch), freq[ch]);
              }      
              casa::Vector<casa::Complex> yyRes(4, casa::Complex(0.,0.));
              if (!flags(0,ch,1) && !flags(1,ch,1) && !flags(2,ch,1)) {
                   yyRes = processOne(spc.xyPlane(1).column(ch), freq[ch]);
              }      
              os<<ch;
              for (size_t i=0; i<3; ++i) {
                   os<<" "<<casa::abs(xxRes[i])<<" "<<casa::arg(xxRes[i]) / casa::C::pi * 180.<<" "<<
                            casa::abs(yyRes[i])<<" "<<casa::arg(yyRes[i]) / casa::C::pi * 180.;
              }
              os<<" "<<casa::real(xxRes[3])<<" "<<casa::real(yyRes[3])<<endl;
         }
     }
}


int main(int argc, char **argv) {
  try {
     if (argc<2) {
         cerr<<"Usage: "<<argv[0]<<" measurement_set1 ... measurement_setN"<<endl;
	 return -2;
     }

     casa::Timer timer;
    
     timer.mark();

     casa::Matrix<std::pair<casa::Complex, casa::Float> > bandpasses;
     std::vector<std::string> msNames(argc - 1);

     for (int beam = 0; beam < argc - 1; ++beam) {
          msNames[beam] = argv[beam + 1];
          std::cerr<<"Beam "<<beam + 1<<" data will be taken from "<<msNames[beam]<<std::endl;
     }

     process(msNames);

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
