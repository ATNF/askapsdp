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
#include <casa/OS/Directory.h>

#include <delaysolver/DelaySolverImpl.h>

#include <Common/ParameterSet.h>

#include <iomanip>

using namespace askap;
using namespace askap::accessors;

class DelaySolverApp : public askap::Application {
public:
   /// @brief process a single file
   /// @param[in] ds data source
   /// @param[in] currentDelays a vector with fixed delays (per antenna) used for observations
   void process(const IConstDataSource &ds, const std::vector<double>& currentDelays);
   
   /// @brief run application
   /// @param[in] argc number of parameters
   /// @param[in] argv parameter vector
   /// @return exit code
   virtual int run(int argc, char *argv[]);
};

void DelaySolverApp::process(const IConstDataSource &ds, const std::vector<double>& currentDelays) {
  IDataSelectorPtr sel=ds.createSelector();
  casa::uInt beam = config().getUint("beam",0);
  sel->chooseFeed(beam);
  sel->chooseCrossCorrelations();
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
  if (currentDelays.size() > 0) {
      ASKAPLOG_INFO_STR(logger, "Old delays (ns): "<< std::setprecision(9) << currentDelays);
      ASKAPCHECK(currentDelays.size() == delays.nelements(), "Number of antennas differ in fixeddelays parameter and in the dataset");
      for (casa::uInt ant = 0; ant < delays.nelements(); ++ant) {
           delays[ant] += currentDelays[ant];
      }
      ASKAPLOG_INFO_STR(logger, "New delays (ns): "<< std::setprecision(9)<<delays);
      const std::string outParset = "corrected_fixeddelay.parset"; 
      {
          std::ofstream os(outParset.c_str());
          os << "cp.ingest.tasks.FringeRotationTask.params.fixeddelays = " << std::setprecision(9)<<delays << std::endl;
      }
      ASKAPLOG_INFO_STR(logger, "The new delays are now stored in "<<outParset);       
  } else {
      ASKAPLOG_WARN_STR(logger, "No fixed delays specified in the parset -> no update");
  }
}

int DelaySolverApp::run(int, char **) {
  try {

     casa::Timer timer;
     std::string msName = parameter("ms");
     const std::string sbID = parameter("sb");

     // get current delays from the application's parset, this is only intended to be used if no scheduling block ID is given
     std::vector<double> currentDelays = config().getDoubleVector("cp.ingest.tasks.FringeRotationTask.params.fixeddelays", 
                                               std::vector<double>());
     if (config().isDefined("ms")) {
         ASKAPCHECK(msName == "", "Use either ms parset parameter or the command line argument, not both");
         msName = config().getString("ms");
     }
     
     if (sbID != "") {
         // scheduling block ID is specified, the file name will be taken from SB
         ASKAPCHECK(msName == "", "When the scheduling block ID is specified, the file name is taken from that SB. "
                    "Remove the -f command line parameter or ms keyword in the parset to continue.");
         casa::Path path2sb(config().getString("sbpath","./"));
         path2sb.append(sbID);
         const casa::Directory sbDir(path2sb);
         // do not follow symlinks, non-recursive
         const casa::Vector<casa::String> dirContent = sbDir.find(casa::Regex::fromPattern("*.ms"),casa::False, casa::False);
         ASKAPCHECK(dirContent.nelements() > 0, "Unable to find a measurement set file in "<<sbDir.path().absoluteName());
         ASKAPCHECK(dirContent.nelements() == 1, "Multiple measurement sets are present in "<<sbDir.path().absoluteName());
         msName = dirContent[0];
         // fixed delays will be taken from cpingest.in in the SB directory
         casa::Path path2cpingest(path2sb);
         path2cpingest.append("cpingest.in");
         const LOFAR::ParameterSet ingestParset(path2cpingest.absoluteName());
         ASKAPCHECK(currentDelays.size() == 0, "When the scheduling block ID is specified, the current fixed delays are taken "
                    "from the ingest pipeline parset stored with that SB. Remove it from the application's parset to continue.");
         
         currentDelays = ingestParset.getDoubleVector("cp.ingest.tasks.FringeRotationTask.params.fixeddelays");                                
     }
     timer.mark();
     ASKAPLOG_INFO_STR(logger, "Processing measurement set "<<msName);
     TableDataSource ds(msName,TableDataSource::MEMORY_BUFFERS);     
     std::cerr<<"Initialization: "<<timer.real()<<std::endl;
     timer.mark();
     process(ds, currentDelays);
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
  app.addParameter("ms","f", "Measurement set name (optional)","");
  app.addParameter("sb","s", "Scheduling block number (optional)","");  
  return app.main(argc,argv);
}

