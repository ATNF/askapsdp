/// @file
///
/// @brief Class for parallel simulation using CASA NewMSSimulator
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
/// 

#include <iostream>
#include <sstream>

#include <casa/OS/Timer.h>

#include <conrad_synthesis.h>

#include <conrad/ConradError.h>
#include <conrad_synthesis.h>
#include <conrad/ConradLogging.h>
CONRAD_LOGGER(logger, ".parallel");

#include <parallel/SimParallel.h>

#include <dataaccess/DataAccessError.h>
#include <dataaccess/TableDataSource.h>
#include <dataaccess/ParsetInterface.h>

#include <measurementequation/ImageFFTEquation.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <measurementequation/ImageRestoreSolver.h>
#include <measurementequation/MEParsetInterface.h>
#include <measurementequation/CalibrationME.h>
#include <measurementequation/NoXPolGain.h>
#include <measurementequation/ImagingEquationAdapter.h>


#include <measurementequation/ImageSolverFactory.h>
#include <gridding/VisGridderFactory.h>

#include <APS/ParameterSet.h>

using namespace std;

using namespace conrad;
using namespace conrad::cp;
using namespace LOFAR::ACC::APS;

namespace conrad
{
  namespace synthesis
  {

    SimParallel::SimParallel(int argc, const char** argv,
        const LOFAR::ACC::APS::ParameterSet& parset) :
      SynParallel(argc, argv), itsParset(parset)
    {
    }

    void SimParallel::init() {
      if(isMaster()) {
	readModels();
	broadcastModel();
      }

      if (isWorker())
      {
        string msname(substitute(itsParset.getString("dataset",
            "test%w.ms")));
        itsSim=boost::shared_ptr<Simulator> (new Simulator(msname));

        itsMs=boost::shared_ptr<casa::MeasurementSet> (new casa::MeasurementSet(msname, casa::Table::Update));

        // The antenna info is kept in a separate parset file
	readAntennas();
	
	/// Get the source definitions and get the model from the master
	readSources();
	receiveModel();

	/// Get the feed definitions
	readFeeds();
	
	/// Get the spectral window definitions. Not all of these need to be
	/// used.
	readSpws();
	
	/// Get miscellaneous information about the simulation
	readSimulation();
      }
    }
    
    SimParallel::~SimParallel()
    {
      if(isWorker())
	{
	  itsMs->flush();
	}
    }
    
    void SimParallel::readAntennas()
    {
      ParameterSet parset(itsParset);
      if(itsParset.isDefined("antennas.definition"))
	{
	  parset=ParameterSet(substitute(itsParset.getString("antennas.definition")));
	}
      
      /// Csimulator.name = ASKAP
      string telName=parset.getString("antennas.telescope");
      CONRADLOG_INFO_STR(logger, "Simulating "<< telName );
      ostringstream oos;
      oos << "antennas." << telName << ".";
      ParameterSet antParset(parset.makeSubset(oos.str()));
      
      /// Csimulator.ASKAP.number=45
      vector<string> antNames(antParset.getStringVector("names"));
      int nAnt=antNames.size();
      CONRADCHECK(nAnt>0, "No antennas defined in parset file");
      
      /// Csimulator.ASKAP.mount=equatorial
      string mount=antParset.getString("mount", "equatorial");
      CONRADCHECK((mount=="equatorial")||(mount=="alt-az"), "Antenna mount unknown");
      
      /// Csimulator.ASKAP.mount=equatorial
      double diameter=MEParsetInterface::asQuantity(antParset.getString("diameter", "12m")).getValue("m");
      CONRADCHECK(diameter>0.0, "Antenna diameter not positive");
      
      /// Csimulator.ASKAP.coordinates=local
      string coordinates=antParset.getString("coordinates", "local");
      CONRADCHECK((coordinates=="local")||(coordinates=="global"), "Coordinates type unknown");
      
      /// Csimulator.ASKAP.scale=0.333
      float scale=antParset.getFloat("scale", 1.0);
      
      /// Now we get the coordinates for each antenna in turn
      casa::Vector<double> x(nAnt);
      casa::Vector<double> y(nAnt);
      casa::Vector<double> z(nAnt);
      
      casa::Vector<double> dishDiameter(nAnt);
      
      casa::Vector<double> offset(nAnt);
      offset.set(0.0);
      casa::Vector<casa::String> mounts(nAnt);
      casa::Vector<casa::String> name(nAnt);
      
      /// Antenna information in the form:
      /// antennas.ASKAP.antenna0=[x,y,z]
      /// ...
      for (int iant=0; iant<nAnt; iant++)
	{
	  vector<float> xyz=antParset.getFloatVector(antNames[iant]);
	  x[iant]=xyz[0]*scale;
	  y[iant]=xyz[1]*scale;
	  z[iant]=xyz[2]*scale;
	  mounts[iant]=mount;
	  dishDiameter[iant]=diameter;
	  name[iant]=antNames[iant];
	}
      
      /// Csimulator.ASKAP.location=[+115deg, -26deg, 192km, WGS84]
      casa::MPosition location=
	MEParsetInterface::asMPosition(antParset.getStringVector("location"));
      
      itsSim->initAnt(telName, x, y, z, dishDiameter, offset, mounts, name,
		      casa::String(coordinates), location);
      CONRADLOG_INFO_STR(logger, "Successfully defined "<< nAnt << " antennas of "<< telName
			 );
    }
    
    void SimParallel::readFeeds()
    {
      ParameterSet parset(itsParset);
      if(itsParset.isDefined("feeds.definition"))
	{
	  parset=ParameterSet(substitute(itsParset.getString("feeds.definition")));
	}
      
      vector<string> feedNames(parset.getStringVector("feeds.names"));
      int nFeeds=feedNames.size();
      CONRADCHECK(nFeeds>0, "No feeds specified");
      
      casa::Vector<double> x(nFeeds);
      casa::Vector<double> y(nFeeds);
      casa::Vector<casa::String> pol(nFeeds);
      
      casa::String mode=parset.getString("feeds.mode", "perfect X Y");
      for (int feed=0; feed<nFeeds; feed++)
	{
	  ostringstream os;
	  os << "feeds."<< feedNames[feed];
	  vector<double> xy(parset.getDoubleVector(os.str()));
	  x[feed]=xy[0];
	  y[feed]=xy[1];
	  pol[feed]="X Y";
	}
      if(parset.isDefined("feeds.spacing"))
	{
	  casa::Quantity qspacing=MEParsetInterface::asQuantity(parset.getString("feeds.spacing"));
	  double spacing=qspacing.getValue("rad");
	  CONRADLOG_INFO_STR(logger, "Scaling feed specifications by " << qspacing );
	  x*=spacing;
	  y*=spacing;
	}
      itsSim->initFeeds(mode, x, y, pol);
      CONRADLOG_INFO_STR(logger, "Successfully defined "<< nFeeds << " feeds");
    }
    
    /// Csimulator.sources.names = [3C273, 1934-638]
    /// Csimulator.sources.3C273.direction = 
    /// Csimulator.sources.1934-638.direction =
    void SimParallel::readSources()
    {
      ParameterSet parset(itsParset);
      if(itsParset.isDefined("sources.definition"))
	{
	  parset=ParameterSet(substitute(itsParset.getString("sources.definition")));
	}
      
      const vector<string> sources=parset.getStringVector("sources.names");
      for (size_t i=0; i<sources.size(); ++i)
	{
	  {
	    ostringstream oos;
	    oos << "sources." << sources[i]<< ".direction";
	    CONRADLOG_INFO_STR(logger, "Simulating source "<< sources[i] );
	    casa::MDirection direction(MEParsetInterface::asMDirection(parset.getStringVector(oos.str())));
	    itsSim->initFields(casa::String(sources[i]), direction, casa::String(""));
	  }
	}
      CONRADLOG_INFO_STR(logger, "Successfully defined sources");
    }
    
    void SimParallel::readModels()
    {
      ParameterSet parset(itsParset);
      if(itsParset.isDefined("sources.definition"))
	{
	  parset=ParameterSet(substitute(itsParset.getString("sources.definition")));
	}
      
      const vector<string> sources=parset.getStringVector("sources.names");
      for (size_t i=0; i<sources.size(); ++i)
	{
	  {
	    ostringstream oos;
	    oos << "sources." << sources[i]<< ".model";
	    if(parset.isDefined(oos.str()))
              {
                string model=parset.getString(oos.str());
                CONRADLOG_INFO_STR(logger, "Adding image " << model << " as model for "<< sources[i] );
                ostringstream paramName;
                paramName << "image.i." << sources[i];
                SynthesisParamsHelper::getFromCasaImage(*itsModel, paramName.str(), model);
              }
	  }
	}
      CONRADLOG_INFO_STR(logger, "Successfully read models");
    }
    
    void SimParallel::readSpws()
    {
      ParameterSet parset(itsParset);
      if(itsParset.isDefined("spws.definition"))
	{
	  parset=ParameterSet(substitute(itsParset.getString("spws.definition")));
	}
      
      vector<string> names(parset.getStringVector("spws.names"));
      int nSpw=names.size();
      CONRADCHECK(nSpw>0, "No spectral windows defined");
      for (int spw=0; spw<nSpw; spw++)
	{
	  ostringstream os;
	  os << "spws."<< names[spw];
	  vector<string> line=parset.getStringVector(os.str());
	  itsSim->initSpWindows(names[spw], MEParsetInterface::asInteger(line[0]),
				MEParsetInterface::asQuantity(line[1]),
				MEParsetInterface::asQuantity(line[2]),
				MEParsetInterface::asQuantity(line[2]), line[3]);
	}
      CONRADLOG_INFO_STR(logger, "Successfully defined "<< nSpw << " spectral windows");
    }
    
    void SimParallel::readSimulation()
    {
      ParameterSet parset(itsParset);
      if(itsParset.isDefined("simulation.definition"))
	{
	  parset=ParameterSet(substitute(itsParset.getString("simulation.definition")));
	}
      
      /// Csimulator.simulate.blockage=0.1
      itsSim->setFractionBlockageLimit(parset.getDouble("simulation.blockage", 0.0));
      /// Csimulator.simulate.elevationlimit=8deg
      itsSim->setElevationLimit(MEParsetInterface::asQuantity(parset.getString(
									       "simulation.elevationlimit", "8deg")));
      /// Csimulator.simulate.autocorrwt=0.0
      itsSim->setAutoCorrelationWt(parset.getFloat("simulation.autocorrwt", 0.0));
      
      /// Csimulator.simulate.integrationtime=10s
      casa::Quantity
	integrationTime(MEParsetInterface::asQuantity(parset.getString(
								       "simulation.integrationtime", "10s")));
      /// Csimulator.simulate.usehourangles=true
      bool useHourAngles(parset.getBool("simulation.usehourangles", true));
      /// Csimulator.simulate.referencetime=2007Mar07
      vector<string> refTimeString(parset.getStringVector("simulation.referencetime"));
      casa::MEpoch refTime(MEParsetInterface::asMEpoch(refTimeString));
      itsSim->settimes(integrationTime, useHourAngles, refTime);
      CONRADLOG_INFO_STR(logger, "Successfully set simulation parameters");
    }
    
    /// Csimulator.observe.number=2
    /// Csimulator.scan1=[1934-638, LBand1, 0s, 120s]
    /// Csimulator.scan2=[3C273, LBand1, 120s, 1920s]
    /// etc.
    void SimParallel::simulate()
    {
      
      if(isWorker())
	{
	  /// Now that the simulator is defined, we can observe each scan
	  ParameterSet parset(itsParset);
	  if(itsParset.isDefined("observe.definition"))
            {
              parset=ParameterSet(substitute(itsParset.getString("observe.definition")));
            }
	  
	  int nScans=parset.getInt32("observe.number", 0);
	  CONRADCHECK(nScans>0, "No scans defined");
	  
	  for (int scan=0; scan<nScans; scan++)
            {
              ostringstream oos;
              oos << "observe.scan"<< scan;
              vector<string> line=parset.getStringVector(oos.str());
              string source=substitute(line[0]);
              string spw=substitute(line[1]);
              CONRADLOG_INFO_STR(logger, "Observing scan "<< scan << " on source " << source
				 << " at band " << spw << " from "
				 << MEParsetInterface::asQuantity(line[2]) << " to "
                                 << MEParsetInterface::asQuantity(line[3]) );
              itsSim->observe(source, spw,
			      MEParsetInterface::asQuantity(line[2]),
			      MEParsetInterface::asQuantity(line[3]));
            }
	  
	  CONRADLOG_INFO_STR(logger, "Successfully simulated "<< nScans << " scans");
	  itsMs->flush();
	  
	  predict(itsMs->tableName());
	  
	}
    }
    void SimParallel::predict(const string& ms)
    {
      if(isWorker()) {
	     casa::Timer timer;
	     timer.mark();
	     CONRADLOG_INFO_STR(logger, "Simulating data for " << ms );
	     CONRADLOG_INFO_STR(logger, "Model is " << *itsModel);
	     TableDataSource ds(ms, TableDataSource::WRITE_PERMITTED);
	     IDataSelectorPtr sel=ds.createSelector();
	     sel << itsParset;
	     IDataConverterPtr conv=ds.createConverter();
	     conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO), "Hz");
	     conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));
	     IDataSharedIter it=ds.createIterator(sel, conv);
	     /// Create the gridder using a factory acting on a
	     /// parameterset
	     IVisGridder::ShPtr gridder=VisGridderFactory::make(itsParset);
	     CONRADCHECK(gridder, "Gridder not defined correctly");
	     
	     // the measurement equation used for prediction 
	     // actual type depends on what we are simulating
	     // therefore it is uninitialized at the moment
	     conrad::scimath::Equation::ShPtr equation;
	     // an adapter to use imaging equation with the calibration framework
	     // we may not need it (if data corruption is off) at all, therefore
	     // it is uninitialized. We can't move it inside the if-block because
	     // the shared pointer must be valid at the time predict is called
	     // (a destructor is be called when it goes out of scope)
	     boost::shared_ptr<ImagingEquationAdapter> ieAdapter;
	     
	     if (itsParset.getBool("corrupt", false)) {
	        CONRADLOG_INFO_STR(logger, "Making equation to simulate calibration effects");
	        // initialize the adapter
	        ieAdapter.reset(new ImagingEquationAdapter);
	        ieAdapter->assign<ImageFFTEquation>(*itsModel, gridder);
	        scimath::Params gainModel; // to be filled later
	        // need to fill gainModel here after we fix ParsetInterface
            equation.reset(new CalibrationME<NoXPolGain>(gainModel,it,*ieAdapter));
	     } else {
	       CONRADLOG_INFO_STR(logger, "Calibration effects are not simulated");
	       equation.reset(new ImageFFTEquation (*itsModel, it, gridder));
	     }
	     CONRADCHECK(equation, "Equation is not defined correctly");
	     equation->predict();
	     CONRADLOG_INFO_STR(logger,  "Predicted data for "<< ms << " in "<< timer.real() << " seconds ");
	  }
    }
    
  }
}
