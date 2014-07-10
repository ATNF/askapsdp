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
#include <askap/Application.h>
#include <dataaccess/SharedIter.h>
#include <dataaccess/TableManager.h>
#include <dataaccess/IDataConverterImpl.h>
#include <utils/PolConverter.h>


#include <measures/Measures/MFrequency.h>
#include <casa/Arrays/Vector.h>
#include <casa/OS/Timer.h>

#include <delaysolver/DelaySolverImpl.h>

#include <iomanip>

using namespace askap;
using namespace askap::accessors;

class DelaySolverApp : public askap::Application {
public:
   /// @brief process a single file
   /// @param[in] ds data source
   void process(const IConstDataSource &ds);
   
   /// @brief run application
   /// @param[in] argc number of parameters
   /// @param[in] argv parameter vector
   /// @return exit code
   virtual int run(int argc, char *argv[]);
};

void DelaySolverApp::process(const IConstDataSource &ds) {
  IDataSelectorPtr sel=ds.createSelector();
  sel->chooseFeed(0);
  sel->chooseCrossCorrelations();
  //sel->chooseAutoCorrelations();
  IDataConverterPtr conv=ds.createConverter();  
  conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),"Hz");
  conv->setEpochFrame(casa::MEpoch(casa::Quantity(55913.0,"d"),
                      casa::MEpoch::Ref(casa::MEpoch::UTC)),"s");
  conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));                    
 
  const double targetRes = config().getDouble("resolution",1e6);
  const std::string stokesStr = config().getString("stokes","XX");
  const casa::Vector<casa::Stokes::StokesTypes> stokesVector = scimath::PolConverter::fromString(stokesStr);
  ASKAPCHECK(stokesVector.nelements() == 1, "Exactly one stokes parameter should be defined, you have "<<stokesStr);
  const double ampCutoff = config().getDouble("cutoff",-1.);
  const casa::uInt refAnt = config().getUint("refant",1);
  const bool exclude13 = config().getBool("exclude13", false);
  utils::DelaySolverImpl solver(targetRes, stokesVector[0], ampCutoff, refAnt);
  if (exclude13) {
      solver.excludeBaselines(casa::Vector<std::pair<casa::uInt,casa::uInt> >(1,std::pair<casa::uInt,
             casa::uInt>(1,2)));
  }
  
  for (IConstDataSharedIter it=ds.createConstIterator(sel,conv);it!=it.end();++it) {
       solver.process(*it);  
  }
  // corrections have the opposite sign from determined delays, hence the minus
  // the units in the fcm are in ns
  casa::Vector<double> delays = -solver.solve() * 1e9;
  ASKAPLOG_INFO_STR(logger, "Corrections (ns): "<<std::setprecision(9)<<delays);
  const std::vector<double> currentDelays = config().getDoubleVector("cp.ingest.tasks.FringeRotationTask.params.fixeddelays", std::vector<double>());
  if (currentDelays.size() > 0) {
      ASKAPCHECK(currentDelays.size() == delays.nelements(), "Number of antennas differ in fixeddelays parameter and in the dataset");
      for (casa::uInt ant = 0; ant < delays.nelements(); ++ant) {
           delays[ant] += currentDelays[ant];
      }
      ASKAPLOG_INFO_STR(logger, "New delays (ns): "<< std::setprecision(9)<<delays);
  }
}

int DelaySolverApp::run(int, char **) {
  try {

     casa::Timer timer;
     const std::string msName = parameter("ms");

     timer.mark();
     TableDataSource ds(msName,TableDataSource::MEMORY_BUFFERS);     
     std::cerr<<"Initialization: "<<timer.real()<<std::endl;
     timer.mark();
     process(ds);
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

int main(int argc, char *argv[]) {
  DelaySolverApp app;
  app.addParameter("ms","f", "Measurement set name (no default)");
  return app.main(argc,argv);
}

