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
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasFrame.h>
#include <tables/Tables/Table.h>
#include <casa/OS/Timer.h>
#include <casa/Arrays/ArrayMath.h>

#include <utils/DelayEstimator.h>
#include <fitting/GenericNormalEquations.h>
#include <fitting/LinearSolver.h>
#include <fitting/DesignMatrix.h>

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

// 15, 8, 9
const double antITRFpos[3][3]={{-2555397.93943903,5097670.48452923,-2848570.361727}, /*initial location {-2555394.936910,5097674.796317,-2848567.461727},*/
                               {-2556005.813742, 5097327.008027, -2848641.257970},
                               {-2555892.578900, 5097559.600315, -2848328.739449}};

void heightCorrected(double h, casa::uInt ant) 
{
   ASKAPDEBUGASSERT(ant<3);
   const double length=sqrt(antITRFpos[ant][0]*antITRFpos[ant][0]+antITRFpos[ant][1]*antITRFpos[ant][1]+antITRFpos[ant][2]*antITRFpos[ant][2]);
   std::cout<<"[";
   for (size_t i=0;i<3;++i) {
        if (i!=0) {
            std::cout<<",";
        }
        std::cout<<std::setprecision(15)<<antITRFpos[ant][i]/length*h+antITRFpos[ant][i];
   }
   std::cout<<"]";
}

struct RateEstimator {
   // note with 1/2pi resolution delay estimator just gives the slope coefficient
   RateEstimator() : itsCounter(0), itsTime(0.), itsStartTime(0.), itsEndTime(0.), itsEstimator(1./2./casa::C::pi) { itsPhases.reserve(260); }

   void init() { itsPhases.erase(itsPhases.begin(), itsPhases.end()); itsTime = 0.; itsStartTime = 0.; ASKAPDEBUGASSERT(itsPhases.size()==0);}

   // phase in radians, time in seconds
   void add(const double phase, const double time) { 
        if (itsPhases.size() == 0) {
            itsStartTime = time;
        }
        itsEndTime = time;
        itsPhases.push_back(phase);
   }

   size_t size() const { return itsPhases.size();}

   double getMidPoint() const {
      const double midPoint = (itsEndTime+itsStartTime) / 2.;
      return midPoint;
   }

   double getDuration() const {
      return itsEndTime - itsStartTime;
   }

   bool isEmpty() const { return itsPhases.size() == 0;}
   
   mutable size_t itsCounter; 

   /// @return time rate in rad per second
   double getRate() const {
      ASKAPDEBUGASSERT(itsPhases.size() > 1);

      const double interval = (itsEndTime - itsStartTime) / double(itsPhases.size() - 1);

      ASKAPDEBUGASSERT(interval > 0.);

      // just for simplicity - reuse the existing code
      casa::Vector<casa::Complex> buf(itsPhases.size(),0.);
      //std::ofstream os("dbg.dat");
      for (casa::uInt i = 0; i<buf.nelements(); ++i) {
           buf[i] = casa::polar(1.,itsPhases[i]);
           //os<<itsPhases[i]*180./casa::C::pi<<std::endl;
      }
      //std::cout<<itsEstimator.getDelay(buf) / interval<<" interval="<<interval<<std::endl;
      //if (++itsCounter >=3) { throw 1;}
      // use delay estimator to get us time rate because the math is the same
      return itsEstimator.getDelay(buf) / interval;
   }
private:
   std::vector<double> itsPhases;

   double itsTime;
 
   double itsStartTime;
   double itsEndTime;

   scimath::DelayEstimator itsEstimator;
};

void finaliseRate(RateEstimator &re, scimath::GenericNormalEquations &gne, casa::uInt index1, casa::uInt index2, double cH0, double sH0, double cd, double effLO)
{
   if (re.size() < 2) {
       return;
   }
   if ((index1 != 0) || (index2 != 1)) return;
   scimath::DesignMatrix dm;
   const std::string ant1str = utility::toString(index1);
   const std::string ant2str = utility::toString(index2);
   const double rate = re.getRate();

   if ((rate*180./casa::C::pi > 0.5) || (rate*180./casa::C::pi <-0.1)) { return; }

   const double siderealRate = casa::C::_2pi / 86400. / (1. - 1./365.25);

   
   dm.addDerivative("x"+ant1str, casa::Vector<casa::Double>(1,sH0*cd*siderealRate));
   dm.addDerivative("x"+ant2str, casa::Vector<casa::Double>(1,-sH0*cd*siderealRate));
   dm.addDerivative("y"+ant1str, casa::Vector<casa::Double>(1,cH0*cd*siderealRate));
   dm.addDerivative("y"+ant2str, casa::Vector<casa::Double>(1,-cH0*cd*siderealRate));
   

   /*
   ASKAPDEBUGASSERT(index1<3);
   ASKAPDEBUGASSERT(index2<3);
   const double length1=sqrt(antITRFpos[index1][0]*antITRFpos[index1][0]+antITRFpos[index1][1]*antITRFpos[index1][1]+antITRFpos[index1][2]*antITRFpos[index1][2]);
   const double length2=sqrt(antITRFpos[index2][0]*antITRFpos[index2][0]+antITRFpos[index2][1]*antITRFpos[index2][1]+antITRFpos[index2][2]*antITRFpos[index2][2]);

   const double coeffX1 = antITRFpos[index1][0]/length1;
   const double coeffY1 = antITRFpos[index1][1]/length1;
   const double coeffX2 = antITRFpos[index2][0]/length2;
   const double coeffY2 = antITRFpos[index2][1]/length2;
   dm.addDerivative("h"+ant1str, casa::Vector<casa::Double>(1,-sH0*cd*siderealRate*coeffX1-cH0*cd*siderealRate*coeffY1));
   dm.addDerivative("h"+ant2str, casa::Vector<casa::Double>(1,sH0*cd*siderealRate*coeffX2+cH0*cd*siderealRate*coeffY2));
   */
       
   dm.addResidual(casa::Vector<casa::Double>(1,rate*casa::C::c/effLO/2./casa::C::pi), casa::Vector<double>(1, 1.));

   gne.add(dm);
   
   
   const double factor = 360.*effLO/casa::C::c*cd*siderealRate;
   if ((index1 == 0) && (index2 == 1)) 
       //std::cout<<rate*180/casa::C::pi<<" "<<sH0*factor<<" "<<cH0*factor<<" "<<(sH0*siderealRate*coeffX1+cH0*siderealRate*coeffY1)*360.*effLO/casa::C::c<<std::endl;
       std::cout<<rate*180/casa::C::pi<<" "<<sH0*factor<<" "<<cH0*factor<<std::endl;
   
   re.init();
}

void processDelays(scimath::GenericNormalEquations &gne, const accessors::IConstDataAccessor &acc, std::vector<RateEstimator> &re)
{
   ASKAPDEBUGASSERT(acc.nRow()>0);
   ASKAPDEBUGASSERT(acc.nChannel()>1);
   ASKAPDEBUGASSERT(acc.nPol()>0);
   ASKAPDEBUGASSERT(acc.nRow() == re.size());
   // the following assumes averaging down to 1 MHz
   //const double effectiveLO = 1e6*(acc.frequency()[0] - 343 - 0.018518518/2.);
   scimath::DelayEstimator de(1e6*(acc.frequency()[1]-acc.frequency()[0]));
   casa::Matrix<casa::Complex> vis = acc.visibility().xyPlane(3);
   casa::Matrix<casa::Bool> flags = acc.flag().xyPlane(3);
  
   //const double accTime = re[0].getMidPoint();

   const double accTime = acc.time();
   const casa::MEpoch epoch(casa::Quantity(56100.0+accTime/86400.,"d"), casa::MEpoch::Ref(casa::MEpoch::UTC));
   casa::MeasFrame frame(epoch);
   const casa::MVDirection dir = casa::MDirection::Convert(casa::MDirection(acc.pointingDir1()[0]), 
                                 casa::MDirection::Ref(casa::MDirection::JTRUE, frame))().getAngle();
   //std::cout<<epoch<<std::endl;
   //std::cout<<printDirection(dir)<<std::endl;
   const double ra = dir.getValue()(0);
   const double dec = dir.getValue()(1);
   const double gmstInDays = casa::MEpoch::Convert(epoch,casa::MEpoch::Ref(casa::MEpoch::GMST1))().get("d").getValue("d");
   const double gmst = (gmstInDays - casa::Int(gmstInDays)) * casa::C::_2pi; // in radians
  
   const double H0 = gmst - ra, sH0 = sin(H0), cH0 = cos(H0), sd = sin(dec), cd = cos(dec);
  
   for (casa::uInt baseline = 0; baseline < vis.nrow(); ++baseline) {
        const casa::uInt index1 = acc.antenna1()[baseline];
        const casa::uInt index2 = acc.antenna2()[baseline];

        bool flagged = false;
        for (casa::uInt ch = 0; ch < flags.ncolumn(); ++ch) {
             flagged |= flags(baseline,ch);
        }
        if (flagged) {
            if (!re[baseline].isEmpty()) {
                //finaliseRate(re[baseline], gne, index1, index2, cH0, sH0, cd, effectiveLO);
            }
            continue;
        }

        ASKAPDEBUGASSERT(vis.ncolumn()>0);
        // direct measurement of the delay
        const double delay = de.getDelay(vis.row(baseline));

        
        // phase as a proxy
        //casa::Complex avgVis = casa::sum(vis.row(baseline)) / casa::Float(vis.ncolumn());
        //const double phase = arg(avgVis);

        if (!re[baseline].isEmpty() && (re[baseline].getDuration() > 300.)) {
            //finaliseRate(re[baseline], gne, index1, index2, cH0, sH0, cd, effectiveLO);
        }
        //re[baseline].add(phase, acc.time());

        //const double delay = phase / 2. / casa::C::pi / effectiveLO;
        

        const std::string ant1str = utility::toString(index1);
        const std::string ant2str = utility::toString(index2);
        ASKAPASSERT(index1<3);
        ASKAPASSERT(index2<3);

        

        scimath::DesignMatrix dm;
/*
        const double length1=sqrt(antITRFpos[index1][0]*antITRFpos[index1][0]+antITRFpos[index1][1]*antITRFpos[index1][1]+antITRFpos[index1][2]*antITRFpos[index1][2]);
        const double length2=sqrt(antITRFpos[index2][0]*antITRFpos[index2][0]+antITRFpos[index2][1]*antITRFpos[index2][1]+antITRFpos[index2][2]*antITRFpos[index2][2]);

        const double coeff1 = -cH0*cd*antITRFpos[index1][0]/length1+sH0*cd*antITRFpos[index1][1]/length1-sd*antITRFpos[index1][2]/length1;
        dm.addDerivative("h"+ant1str, casa::Vector<casa::Double>(1,coeff1));
        dm.addDerivative("h"+ant2str, casa::Vector<casa::Double>(1,cH0*cd*antITRFpos[index2][0]/length2-sH0*cd*antITRFpos[index2][1]/length2+sd*antITRFpos[index2][2]/length2));

        if (baseline == 0)
        std::cout<<baseline<<" "<<acc.antenna1()[baseline]<<" "<<acc.antenna2()[baseline]<<" "<<-cH0*cd<<" "<<sH0*cd<<" "<<-sd<<" "<<H0/casa::C::pi*180.<<" "<<delay*casa::C::c<<" "<<coeff1<<std::endl;

        */
        /*
        dm.addDerivative("d"+ant1str, casa::Vector<casa::Double>(1,casa::C::c*1e-9));
        dm.addDerivative("d"+ant2str, casa::Vector<casa::Double>(1,-casa::C::c*1e-9));
        */
        /*
        dm.addDerivative("p"+ant1str, casa::Vector<casa::Double>(1,360.*effectiveLO*casa::C::c));
        dm.addDerivative("p"+ant2str, casa::Vector<casa::Double>(1,-360.*effectiveLO*casa::C::c));
        */
 
        
        dm.addDerivative("x"+ant1str, casa::Vector<casa::Double>(1,cH0*cd));
        dm.addDerivative("x"+ant2str, casa::Vector<casa::Double>(1,-cH0*cd));
        dm.addDerivative("y"+ant1str, casa::Vector<casa::Double>(1,-sH0*cd));
        dm.addDerivative("y"+ant2str, casa::Vector<casa::Double>(1,sH0*cd));
        dm.addDerivative("z"+ant1str, casa::Vector<casa::Double>(1,sd));
        dm.addDerivative("z"+ant2str, casa::Vector<casa::Double>(1,-sd));
        
        dm.addResidual(casa::Vector<casa::Double>(1,-delay*casa::C::c), casa::Vector<double>(1, 1.));
        gne.add(dm);
        
   }
}

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

void process(scimath::GenericNormalEquations &gne, const IConstDataSource &ds, size_t nAvg) {
  std::vector<RateEstimator> re(3);

  IDataSelectorPtr sel=ds.createSelector();
  //sel->chooseBaseline(0,1);
  sel->chooseFeed(0);
  sel->chooseCrossCorrelations();
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

  casa::Vector<casa::uInt> ant1IDs;
  casa::Vector<casa::uInt> ant2IDs;

  std::ofstream os("result.dat");
  for (IConstDataSharedIter it=ds.createConstIterator(sel,conv);it!=it.end();++it) {  
       if (nChan == 0) {
           nChan = it->nChannel();
           startTime = it->time();
           ant1IDs = it->antenna1().copy();
           ant2IDs = it->antenna2().copy();
           for (casa::uInt row = 0; row<it->nRow();++row) {
                std::cout<<"plane "<<row<<" corresponds to "<<ant1IDs[row]<<" - "<<ant2IDs[row]<<" baseline"<<std::endl;
           }
       } else { 
           ASKAPCHECK(nChan == it->nChannel(), 
                  "Number of channels seem to have been changed, previously "<<nChan<<" now "<<it->nChannel());
           if ((ant1IDs.nelements() != it->nRow()) || (ant2IDs.nelements() != it->nRow())) {
               std::cout<<"Ingoring "<<it->nRow()<<" rows"<<std::endl;
               continue;
           } 
           //ASKAPDEBUGASSERT(ant1IDs.nelements() == it->nRow());
           //ASKAPDEBUGASSERT(ant2IDs.nelements() == it->nRow());
           for (casa::uInt row = 0; row<it->nRow(); ++row) {
                ASKAPCHECK(ant1IDs[row] == it->antenna1()[row], "Mismatch of antenna 1 index for row "<<row<<
                           " - got "<<it->antenna1()[row]<<" expected "<<ant1IDs[row]);
                ASKAPCHECK(ant2IDs[row] == it->antenna2()[row], "Mismatch of antenna 2 index for row "<<row<<
                           " - got "<<it->antenna2()[row]<<" expected "<<ant2IDs[row]);
           }
       }
       ASKAPCHECK(it->nRow() == 3, "Expect 3 baselines, the accessor has "<<it->nRow()<<" rows");
       ASKAPASSERT(it->nPol() >= 1);
       ASKAPASSERT(it->nChannel() >= 1);
       casa::Vector<casa::Complex> freqAvBuf(it->nRow(), casa::Complex(0.,0.));
       for (casa::uInt ch=0; ch<it->nChannel(); ++ch) {
            freqAvBuf += it->visibility().xyPlane(3).column(ch);
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
       processDelays(gne,*it,re);

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
     if (argc<2) {
         cerr<<"Usage "<<argv[0]<<" measurement_set1 [measurement_set2 ...]"<<endl;
	 return -2;
     }

     casa::Timer timer;

     timer.mark();

     std::vector<std::string> datasets(argc - 1);
     for (int i=1; i<argc; ++i) {
          datasets[i-1] = argv[i];
     }
     scimath::GenericNormalEquations gne;

     for (std::vector<std::string>::const_iterator ci = datasets.begin(); ci != datasets.end(); ++ci) {
          
          TableDataSource ds(*ci,TableDataSource::MEMORY_BUFFERS);     
          // number of cycles to average
          const size_t nAvg = 1;
          process(gne, ds, nAvg);
     }
    
     // now solve
     scimath::LinearSolver solver;
     solver.setAlgorithm("SVD");
     solver.addNormalEquations(gne);
     scimath::Quality q;
     std::vector<std::string> unknowns = gne.unknowns();
     scimath::Params params;
     for (std::vector<std::string>::const_iterator ci = unknowns.begin(); ci!=unknowns.end(); ++ci) {
          params.add(*ci, 0.);
          
          /*
          // fix the vertical component (no appropriate data taken yet)
          if (ci->find("z") == 0) {
              params.fix(*ci);
          }
          */
          
     }
     //params.fix("h1");
     //params.fix("h2");
     //params.fix("p1");
     
     // antenna 8 is the reference for now
     params.fix("x1");
     params.fix("y1");

     params.fix("z1");

     params.fix("x2");
     params.fix("y2");
     params.fix("z2");
     //params.fix("y0");
     

     solver.addNormalEquations(gne);
     solver.solveNormalEquations(params,q);
     std::cout<<q<<std::endl;
       std::cout<<params<<std::endl;
     const size_t parPerAnt = 3; // 3;
     if (unknowns.size() % parPerAnt == 0) {
        for (size_t ant=0; ant < unknowns.size() / parPerAnt; ++ant) {
             const std::string antStr = utility::toString(ant);
             //std::cout<<"ant: "<<ant<<" dH: "<<params.scalarValue("h"+antStr)<<" (metres) "<<std::endl;
             //std::cout<<"height corrected: "; heightCorrected(params.scalarValue("h"+antStr), ant); std::cout<<std::endl;

             std::cout<<"ant: "<<ant<<" dX: "<<params.scalarValue("x"+antStr)<<" dY: "<<
                        params.scalarValue("y"+antStr) << " dZ: "<<params.scalarValue("z"+antStr)<<" (metres)"<<std::endl;

 /*            
             std::cout<<"ant: "<<ant<<" dX: "<<params.scalarValue("x"+antStr)<<" dY: "<<
                        params.scalarValue("y"+antStr) <<" (metres)"<<std::endl;
 */ 
             std::cout<<"Full: ["<<std::setprecision(15)<<antITRFpos[ant][0]+params.scalarValue("x"+antStr)<<","<<
                                   std::setprecision(15)<<antITRFpos[ant][1]+params.scalarValue("y"+antStr)<<","<<
                                   std::setprecision(15)<<antITRFpos[ant][2]+params.scalarValue("z"+antStr)<<"]"<<std::endl;
 
        }
     } else {
       std::cout<<params<<std::endl;
     }

             std::cout<<"Test: ["<<std::setprecision(15)<<antITRFpos[2][0]+0.5<<","<<
                                   std::setprecision(15)<<antITRFpos[2][1]-0.7<<","<<
                                   std::setprecision(15)<<antITRFpos[2][2]+0.<<"]"<<std::endl;
     
     //
     std::cout<<"custom offset: "; heightCorrected(15., 0); std::cout<<std::endl;
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
