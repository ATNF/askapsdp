/// @file
/// @brief an utility to solve for antenna-based delays
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


#include <measures/Measures/MFrequency.h>
#include <casa/Arrays/Vector.h>
#include <casa/OS/Timer.h>

#include <delaysolver/DelaySolverImpl.h>

using namespace askap;
using namespace askap::accessors;

void process(const IConstDataSource &ds, const int ctrl = -1) {
  IDataSelectorPtr sel=ds.createSelector();
  sel->chooseFeed(0);
  sel->chooseCrossCorrelations();
  //sel->chooseAutoCorrelations();
  if (ctrl >=0 ) {
      sel->chooseUserDefinedIndex("CONTROL",casa::uInt(ctrl));
  }
  IDataConverterPtr conv=ds.createConverter();  
  conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),"MHz");
  conv->setEpochFrame(casa::MEpoch(casa::Quantity(55913.0,"d"),
                      casa::MEpoch::Ref(casa::MEpoch::UTC)),"s");
  conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));                    
 
  utils::DelaySolverImpl solver(1e6);
  
  for (IConstDataSharedIter it=ds.createConstIterator(sel,conv);it!=it.end();++it) {
       solver.process(*it);  
  }
  casa::Vector<double> delays = solver.solve();
  std::cout<<delays<<std::endl;
}

int main(int argc, char **argv) {
  try {
     if ((argc!=2) && (argc!=3)) {
         std::cerr<<"Usage: "<<argv[0]<<" [ctrl] measurement_set"<<std::endl;
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
