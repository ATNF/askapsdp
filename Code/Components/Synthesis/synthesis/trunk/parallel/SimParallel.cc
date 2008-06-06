/// @file
///
/// @brief Class for parallel simulation using CASA NewMSSimulator
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
/// @author Tim Cornwell <tim.cornwell@csiro.au>
/// 

#include <iostream>
#include <sstream>

#include <casa/OS/Timer.h>

#include <askap_synthesis.h>

#include <askap/AskapError.h>
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".parallel");

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
#include <measurementequation/SumOfTwoMEs.h>
#include <measurementequation/GaussianNoiseME.h>
#include <measurementequation/ComponentEquation.h>



#include <measurementequation/ImageSolverFactory.h>
#include <gridding/VisGridderFactory.h>

#include <APS/ParameterSet.h>

using namespace std;

using namespace askap;
using namespace askap::cp;
using namespace LOFAR::ACC::APS;

namespace askap
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
      ASKAPLOG_INFO_STR(logger, "Simulating "<< telName );
      ostringstream oos;
      oos << "antennas." << telName << ".";
      ParameterSet antParset(parset.makeSubset(oos.str()));
      
      /// Csimulator.ASKAP.number=45
      vector<string> antNames(antParset.getStringVector("names"));
      int nAnt=antNames.size();
      ASKAPCHECK(nAnt>0, "No antennas defined in parset file");
      
      /// Csimulator.ASKAP.mount=equatorial
      string mount=antParset.getString("mount", "equatorial");
      ASKAPCHECK((mount=="equatorial")||(mount=="alt-az"), "Antenna mount unknown");
      
      /// Csimulator.ASKAP.mount=equatorial
      double diameter=MEParsetInterface::asQuantity(antParset.getString("diameter", "12m")).getValue("m");
      ASKAPCHECK(diameter>0.0, "Antenna diameter not positive");
      
      /// Csimulator.ASKAP.coordinates=local
      string coordinates=antParset.getString("coordinates", "local");
      ASKAPCHECK((coordinates=="local")||(coordinates=="global"), "Coordinates type unknown");
      
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
      ASKAPLOG_INFO_STR(logger, "Successfully defined "<< nAnt << " antennas of "<< telName
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
      ASKAPCHECK(nFeeds>0, "No feeds specified");
      
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
	  ASKAPLOG_INFO_STR(logger, "Scaling feed specifications by " << qspacing );
	  x*=spacing;
	  y*=spacing;
	}
      itsSim->initFeeds(mode, x, y, pol);
      ASKAPLOG_INFO_STR(logger, "Successfully defined "<< nFeeds << " feeds");
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
	    ASKAPLOG_INFO_STR(logger, "Simulating source "<< sources[i] );
	    casa::MDirection direction(MEParsetInterface::asMDirection(parset.getStringVector(oos.str())));
	    itsSim->initFields(casa::String(sources[i]), direction, casa::String(""));
	  }
	}
      ASKAPLOG_INFO_STR(logger, "Successfully defined sources");
    }
    
    void SimParallel::readModels() {
      ParameterSet parset(itsParset);
      if(itsParset.isDefined("sources.definition")) {
	     parset=ParameterSet(substitute(itsParset.getString("sources.definition")));
	  }
      
      const vector<string> sources=parset.getStringVector("sources.names");
      for (vector<string>::const_iterator it = sources.begin(); it != sources.end(); ++it) {
	       const std::string modelPar = "sources."+*it+".model";
	       if (parset.isDefined(modelPar)) {
               string model=parset.getString(modelPar);
               ASKAPLOG_INFO_STR(logger, "Adding image " << model << " as model for "<< *it );
               const string paramName = "image.i." + *it;
               SynthesisParamsHelper::getFromCasaImage(*itsModel, paramName, model);
           }
           const std::string compListPar = "sources."+*it+".components";
           if (parset.isDefined(compListPar)) {
               const vector<string> compList = parset.getStringVector(compListPar);
               for (vector<string>::const_iterator cmp = compList.begin(); 
                    cmp != compList.end(); ++cmp) {
                    ASKAPLOG_INFO_STR(logger, "Loading component "<<*cmp<<" as part of the model for "<<*it);
                    SynthesisParamsHelper::copyComponent(itsModel, parset, *cmp);
               }
           }
	  }
      ASKAPLOG_INFO_STR(logger, "Successfully read models");
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
      ASKAPCHECK(nSpw>0, "No spectral windows defined");
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
      ASKAPLOG_INFO_STR(logger, "Successfully defined "<< nSpw << " spectral windows");
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
      ASKAPLOG_INFO_STR(logger, "Successfully set simulation parameters");
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
	  ASKAPCHECK(nScans>0, "No scans defined");
	  
	  for (int scan=0; scan<nScans; scan++)
            {
              ostringstream oos;
              oos << "observe.scan"<< scan;
              vector<string> line=parset.getStringVector(oos.str());
              string source=substitute(line[0]);
              string spw=substitute(line[1]);
              ASKAPLOG_INFO_STR(logger, "Observing scan "<< scan << " on source " << source
				 << " at band " << spw << " from "
				 << MEParsetInterface::asQuantity(line[2]) << " to "
                                 << MEParsetInterface::asQuantity(line[3]) );
              itsSim->observe(source, spw,
			      MEParsetInterface::asQuantity(line[2]),
			      MEParsetInterface::asQuantity(line[3]));
            }
	  
	  ASKAPLOG_INFO_STR(logger, "Successfully simulated "<< nScans << " scans");
	  itsMs->flush();
	  
	  predict(itsMs->tableName());
	  
	}
    }
    void SimParallel::predict(const string& ms)
    {
      if(isWorker()) {
	     casa::Timer timer;
	     timer.mark();
	     ASKAPLOG_INFO_STR(logger, "Simulating data for " << ms );
	     ASKAPDEBUGASSERT(itsModel);
	     ASKAPLOG_INFO_STR(logger, "Model is " << *itsModel);
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
	     ASKAPCHECK(gridder, "Gridder not defined correctly");
	     	     
	     // a part of the equation defined via image
	     askap::scimath::Equation::ShPtr imgEquation;
	     if (SynthesisParamsHelper::hasImage(itsModel)) {
	         ASKAPLOG_INFO_STR(logger, "Sky model contains at least one image, building an image-specific equation");
	         // it should ignore parameters which are not applicable (e.g. components)
	         imgEquation.reset(new ImageFFTEquation (*itsModel, it, gridder));
         } 
         // a part of the equation defined via components
         boost::shared_ptr<ComponentEquation> compEquation;
         if (SynthesisParamsHelper::hasComponent(itsModel)) {
            // model is a number of components
            ASKAPLOG_INFO_STR(logger, "Sky model contains at least one component, building a component-specific equation");
	        // it doesn't matter which iterator is passed below. It is not used
	       // it should ignore parameters which are not applicable (e.g. images)
            compEquation.reset(new ComponentEquation(*itsModel,it));
         }
         // the measurement equation used for prediction 
	     // actual type depends on what we are simulating
	     // therefore it is uninitialized at the moment
	     askap::scimath::Equation::ShPtr equation;
         
         if (imgEquation && !compEquation) {
             ASKAPLOG_INFO_STR(logger, "Pure image-based model (no components defined)");
             equation = imgEquation;
         } else if (compEquation && !imgEquation) {
             ASKAPLOG_INFO_STR(logger, "Pure component-based model (no images defined)");
             equation = compEquation;
         } else if (imgEquation && compEquation) {
             ASKAPLOG_INFO_STR(logger, "Making a sum of image-based and component-based equations");
             equation = imgEquation;
             addEquation(equation,compEquation,it);
         } else {
            ASKAPTHROW(AskapError, "No sky models are defined");
         }
	     
	     if (itsParset.getBool("corrupt", false)) {
 	         corruptEquation(equation, it);
	     } else {
	         ASKAPLOG_INFO_STR(logger, "Calibration effects are not simulated");
	     }
	     ASKAPCHECK(equation, "Equation is not defined correctly");
	     
	     if (itsParset.getBool("noise", false)) {
	         ASKAPCHECK(itsParset.isDefined("noise.variance"), "noise.variance  is missing in the input parset. It should contain a variance of the noise to be simulated.");   
	         const double variance = itsParset.getDouble("noise.variance");
	         ASKAPLOG_INFO_STR(logger, "Gaussian noise (variance="<<variance<<
	                                   ") will be added to visibilities");
	         casa::Int seed1 = itsParset.getInt32("noise.seed1",0);
	         casa::Int seed2 = itsParset.getInt32("noise.seed2",10);
	         if (itsParset.isDefined("noise.seed1")) {
	             ASKAPLOG_INFO_STR(logger, "Set seed1 to "<<seed1);
	         }
	         if (itsParset.isDefined("noise.seed2")) {
	             ASKAPLOG_INFO_STR(logger, "Set seed2 to "<<seed2);
	         }
	         boost::shared_ptr<GaussianNoiseME const> noiseME(new
	                        GaussianNoiseME(variance,seed1,seed2));
	         addEquation(equation, noiseME,it);               
	     }
	     equation->predict();
	     ASKAPLOG_INFO_STR(logger,  "Predicted data for "<< ms << " in "<< timer.real() << " seconds ");
	  }
    }
    
    /// @brief a helper method to corrupt the data (opposite to calibration)
    /// @details Applying gains require different operations depending on
    /// the type of the measurement equation (accessor-based or iterator-based).
    /// It is encapsulated in this method. The method accesses itsParset to
    /// extract the information about calibration model.
    /// @param[in] equation a non-const reference to the shared pointer holding
    /// an equation to update
    /// @param[in] it iterator over the dataset (this is a legacy of the current
    /// design of the imaging code, when equation requires an iterator. It should 
    /// get away at some stage)
    void SimParallel::corruptEquation(boost::shared_ptr<scimath::Equation> &equation,
                                      const IDataSharedIter &it)
    {
	   ASKAPLOG_INFO_STR(logger, "Making equation to simulate calibration effects");
	   boost::shared_ptr<IMeasurementEquation> accessorBasedEquation = 
	             boost::dynamic_pointer_cast<IMeasurementEquation>(equation);
       if (!accessorBasedEquation) {
	       // initialize an adapter
	       // to use imaging equation with the calibration framework
           // form a replacement equation first
	       const boost::shared_ptr<ImagingEquationAdapter> 
	                      new_equation(new ImagingEquationAdapter);

	       // the actual equation is locked inside ImagingEquationAdapter
	       // in a shared pointer. We can change equation variable
	       // after the next line              
	       new_equation->assign(equation);       	            
           accessorBasedEquation = new_equation;	   
       }     
	   scimath::Params gainModel; 
	   ASKAPCHECK(itsParset.isDefined("corrupt.gainsfile"), "corrupt.gainsfile is missing in the input parset. It should point to the parset file with gains");   
	   const std::string gainsfile = itsParset.getString("corrupt.gainsfile");
	   ASKAPLOG_INFO_STR(logger, "Loading gains from file '"<<gainsfile<<"'");
	   gainModel << ParameterSet(gainsfile);
	   ASKAPDEBUGASSERT(accessorBasedEquation);
	   
	   equation.reset(new CalibrationME<NoXPolGain>(gainModel,it,accessorBasedEquation));      
    }
    
    /// @brief a helper method to add up an equation
    /// @details Some times it is necessary to replace a measurement equation
    /// with a sum of two equations. Typical use cases are adding noise to
    /// the visibility data and simulating using a composite model containing
    /// both components and images. This method replaces the input equation
    /// with the sum of the input equation and some other equation also passed
    /// as a parameter. It takes care of equation types and instantiates 
    /// adapters if necessary.
    /// @param[in] equation a non-const reference to the shared pointer holding
    /// an equation to update
    /// @param[in] other a const reference to the shared pointer holding
    /// an equation to be added
    /// @param[in] it iterator over the dataset (this is a legacy of the current
    /// design of the imaging code, when equation requires an iterator. It should 
    /// get away at some stage)            
    /// @note This method can be moved somewhere else, as it may be needed in
    /// some other places as well
    void SimParallel::addEquation(boost::shared_ptr<scimath::Equation> &equation,
                       const boost::shared_ptr<IMeasurementEquation const> &other,
                       const IDataSharedIter &it)
    {
	   boost::shared_ptr<IMeasurementEquation> accessorBasedEquation = 
	             boost::dynamic_pointer_cast<IMeasurementEquation>(equation);
       if (!accessorBasedEquation) {
           // form a replacement equation first
	       const boost::shared_ptr<ImagingEquationAdapter> 
	                      new_equation(new ImagingEquationAdapter);
	       // the actual equation is locked inside ImagingEquationAdapter
	       // in a shared pointer. We can change equation variable
	       // after the next line              
	       new_equation->assign(equation);       	            
           accessorBasedEquation = new_equation;
       }

       // we need to instantiate a new variable and then assign it to avoid 
       // assigning a pointer to itself behind the scene, which would happen if 
       /// everything is accessor-based up front (although 
       // such situations are probably dealt with correctly inside the shared pointer)
       const boost::shared_ptr<scimath::Equation> result(new 
                SumOfTwoMEs(accessorBasedEquation,other,it));
       equation = result;         
    }
  }
}
