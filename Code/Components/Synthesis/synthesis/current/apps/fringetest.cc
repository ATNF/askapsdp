/// @file
/// @brief utility to make an image demonstrating fringes for sw-correlation experiment
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
#include <fft/FFTWrapper.h>

// casa
#include <measures/Measures/MFrequency.h>
#include <tables/Tables/Table.h>
#include <casa/OS/Timer.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <casa/Arrays/Cube.h>




// std
#include <stdexcept>
#include <iostream>
#include <vector>

using std::cout;
using std::cerr;
using std::endl;

using namespace askap;
using namespace askap::synthesis;
using namespace askap::accessors;

casa::Matrix<casa::Complex> padSecond(const casa::Matrix<casa::Complex> &in, const casa::uInt factor) {
   if (factor == 1) {
       return in;
   }
   ASKAPDEBUGASSERT(factor>0);
   ASKAPDEBUGASSERT(in.ncolumn()>0);
   ASKAPDEBUGASSERT(in.nrow()>0);
   casa::Matrix<casa::Complex> result(in.nrow(), in.ncolumn()*factor,casa::Complex(0.,0.));
   const casa::uInt start = in.ncolumn()*(factor-1)/2;
   result(casa::IPosition(2,0,start), casa::IPosition(2, in.nrow() - 1, start + in.ncolumn() - 1)) = in;
   return result;
}

void process(const IConstDataSource &ds, size_t nAvg, size_t padding = 1) {
  IDataSelectorPtr sel=ds.createSelector();
  //sel->chooseBaseline(0,1);
  IDataConverterPtr conv=ds.createConverter();  
  conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),"MHz");
  conv->setEpochFrame(casa::MEpoch(casa::Quantity(55913.0,"d"),
                      casa::MEpoch::Ref(casa::MEpoch::UTC)),"s");
  conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));                    
  casa::Matrix<casa::Complex> buf;
  size_t counter = 0;
  casa::Cube<casa::Complex> imgBuf;
  const casa::uInt maxSteps = 1360;
  casa::uInt currentStep = 0;
  casa::Vector<casa::uInt> ant1IDs;
  casa::Vector<casa::uInt> ant2IDs;
    
  for (IConstDataSharedIter it=ds.createConstIterator(sel,conv);it!=it.end();++it) {  
       if (buf.nelements() == 0) {
           buf.resize(it->nRow(),it->frequency().nelements()*padding);
           buf.set(casa::Complex(0.,0.));
           ant1IDs = it->antenna1().copy();
           ant2IDs = it->antenna2().copy();
           for (casa::uInt row = 0; row<it->nRow();++row) {
                std::cout<<"plane "<<row<<" corresponds to "<<ant1IDs[row]<<" - "<<ant2IDs[row]<<" baseline"<<std::endl;
           }
           imgBuf.resize(buf.ncolumn(),maxSteps,it->nRow());
           imgBuf.set(casa::Complex(0.,0.));
       } else { 
           ASKAPCHECK(buf.ncolumn() == padding*it->frequency().nelements(), 
                  "Number of channels seem to have been changed, previously "<<buf.ncolumn()<<" now "<<it->frequency().nelements());
           ASKAPCHECK(imgBuf.nplane() == it->nRow(), "The number of rows in the accessor "<<it->nRow()<<
                      " is different to the maximum number of baselines");
           ASKAPDEBUGASSERT(ant1IDs.nelements() == it->nRow());
           ASKAPDEBUGASSERT(ant2IDs.nelements() == it->nRow());
           for (casa::uInt row = 0; row<it->nRow(); ++row) {
                ASKAPCHECK(ant1IDs[row] == it->antenna1()[row], "Mismatch of antenna 1 index for row "<<row<<
                           " - got "<<it->antenna1()[row]<<" expected "<<ant1IDs[row]);
                ASKAPCHECK(ant2IDs[row] == it->antenna2()[row], "Mismatch of antenna 2 index for row "<<row<<
                           " - got "<<it->antenna2()[row]<<" expected "<<ant2IDs[row]);
           }
       }
       ASKAPASSERT(it->nRow() == buf.nrow());
       ASKAPASSERT(it->nChannel()*padding == buf.ncolumn());
       ASKAPASSERT(it->nPol() >= 1);
       buf += padSecond(it->visibility().xyPlane(0),padding);
       if (++counter == nAvg) {
           buf /= float(nAvg);
           for (casa::uInt row = 0; row<buf.nrow(); ++row) {
                casa::Vector<casa::Complex> curRow = buf.row(row);
                scimath::fft(curRow, true);
           }
           ASKAPCHECK(currentStep < imgBuf.ncolumn(), "Image buffer is too small (in time axis)");
           imgBuf.xzPlane(currentStep++) = casa::transpose(buf);
           buf.set(casa::Complex(0.,0.));
           counter = 0;
       }
       //cout<<"time: "<<it->time()<<endl;
  }
  if (counter!=0) {
      buf /= float(counter);
      for (casa::uInt row = 0; row<buf.nrow(); ++row) {
           casa::Vector<casa::Complex> curRow = buf.row(row);
           scimath::fft(curRow, true);
      }
      ASKAPCHECK(currentStep < imgBuf.ncolumn(), "Image buffer is too small (in time axis)");
      imgBuf.xzPlane(currentStep) = casa::transpose(buf);
  } else if (currentStep > 0) {
      --currentStep;
  }
  std::cout<<imgBuf.shape()<<std::endl;
  SynthesisParamsHelper::saveAsCasaImage("fringe.img", casa::amplitude(imgBuf(casa::IPosition(3,0,0,0),
                 casa::IPosition(3,imgBuf.nrow()-1,currentStep,imgBuf.nplane()-1))));
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
     // padding factor
     const size_t padding = 1;
     process(ds, nAvg, padding);
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
