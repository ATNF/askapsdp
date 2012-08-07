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
#include <measurementequation/SynthesisParamsHelper.h>




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

void process(const IConstDataSource &ds, size_t nAvg) {
  IDataSelectorPtr sel=ds.createSelector();
  sel->chooseBaseline(0,2);
  IDataConverterPtr conv=ds.createConverter();  
  conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),"MHz");
  conv->setEpochFrame(casa::MEpoch(casa::Quantity(55913.0,"d"),
                      casa::MEpoch::Ref(casa::MEpoch::UTC)),"s");
  conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));                    
  casa::Vector<casa::Complex> buf;
  size_t counter = 0;
  casa::Matrix<casa::Complex> imgBuf;
  const casa::uInt maxSteps = 360;
  casa::uInt currentStep = 0;
    
  for (IConstDataSharedIter it=ds.createConstIterator(sel,conv);it!=it.end();++it) {  
       if (buf.nelements() == 0) {
           buf.resize(it->frequency().nelements());
           buf.set(casa::Complex(0.,0.));
           imgBuf.resize(buf.nelements(),maxSteps);
           imgBuf.set(casa::Complex(0.,0.));
       } else { 
           ASKAPCHECK(buf.nelements() == it->frequency().nelements(), 
                  "Number of channels seem to have been changed, previously "<<buf.nelements()<<" now "<<it->frequency().nelements());
       }
       ASKAPASSERT(it->nRow() == 1);
       ASKAPASSERT(it->nChannel() == buf.nelements());
       ASKAPASSERT(it->nPol() >= 1);
       buf += it->visibility().xyPlane(0).row(0);
       if (++counter == nAvg) {
           buf /= float(nAvg);
           //buf[13]=0.;
           scimath::fft(buf, true);
           imgBuf.column(currentStep++) = buf;
           buf.set(casa::Complex(0.,0.));
           counter = 0;
           ASKAPCHECK(currentStep < imgBuf.ncolumn(), "Image buffer is too small, need more than "<<imgBuf.nrow());
       }
       //cout<<"time: "<<it->time()<<endl;
  }
  if (counter!=0) {
      buf /= float(counter);
      //buf[13]=0.;
      scimath::fft(buf, true);
      imgBuf.column(currentStep) = buf;
  } else if (currentStep > 0) {
      --currentStep;
  }
  std::cout<<imgBuf.shape()<<std::endl;
  SynthesisParamsHelper::saveAsCasaImage("fringe.img", casa::amplitude(imgBuf(casa::IPosition(2,0,0),
                 casa::IPosition(2,imgBuf.nrow()-1,currentStep))));
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
