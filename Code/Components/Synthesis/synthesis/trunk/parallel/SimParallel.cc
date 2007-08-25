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

#include <conrad/ConradError.h>

#include <parallel/SimParallel.h>

#include <dataaccess/DataAccessError.h>
#include <dataaccess/TableDataSource.h>
#include <dataaccess/ParsetInterface.h>

#include <measurementequation/ImageFFTEquation.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <measurementequation/ImageRestoreSolver.h>
#include <measurementequation/MEParsetInterface.h>

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
			if (isWorker())
			{
				string msname(substituteWorkerNumber(parset.getString("dataset",
				    "test%w.ms")));
				itsSim=boost::shared_ptr<casa::NewMSSimulator> (new casa::NewMSSimulator(msname));

				itsMs=boost::shared_ptr<casa::MeasurementSet> (new casa::MeasurementSet(msname, casa::Table::Update));

				/// The antenna info is kept in a separate parset file
				    readAntennas();

				    /// Get the source definitions.
				    readSources();

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
				    parset=ParameterSet(substituteWorkerNumber(itsParset.getString("antennas.definition")));
			    }

			    /// Csimulator.name = ASKAP
			    string telName=parset.getString("antennas.telescope");
			    os() << "Simulating "<< telName << std::endl;
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
				    x[iant]=xyz[0];
				    y[iant]=xyz[1];
				    z[iant]=xyz[2];
				    mounts[iant]=mount;
				    dishDiameter[iant]=diameter;
				    name[iant]=antNames[iant];
			    }

			    /// Csimulator.ASKAP.location=[+115deg, -26deg, 192km, WGS84]
			    casa::MPosition location=
			    MEParsetInterface::asMPosition(antParset.getStringVector("location"));

			    itsSim->initAnt(telName, x, y, z, dishDiameter, offset, mounts, name,
					    casa::String(coordinates), location);
			    os() << "Successfully defined "<< nAnt << " antennas of "<< telName
			    << std::endl;
		    }

		    void SimParallel::readFeeds()
		    {
			    ParameterSet parset(itsParset);
			    if(itsParset.isDefined("feeds.definition"))
			    {
				    parset=ParameterSet(substituteWorkerNumber(itsParset.getString("feeds.definition")));
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
				    os() << "Scaling feed specifications by " << qspacing << std::endl;
				    x*=spacing;
				    y*=spacing;
			    }
			    itsSim->initFeeds(mode, x, y, pol);
			    os() << "Successfully defined "<< nFeeds << " feeds"<< std::endl;
		    }

		    /// Csimulator.sources.names = [3C273, 1934-638]
		    /// Csimulator.sources.3C273.direction = 
		    /// Csimulator.sources.1934-638.direction =
		    void SimParallel::readSources()
		    {
			    ParameterSet parset(itsParset);
			    if(itsParset.isDefined("sources.definition"))
			    {
				    parset=ParameterSet(substituteWorkerNumber(itsParset.getString("sources.definition")));
			    }

			    const vector<string> sources=parset.getStringVector("sources.names");
			    for (int i=0; i<sources.size(); ++i)
			    {
				    {
					    ostringstream oos;
					    oos << "sources." << sources[i]<< ".direction";
					    os() << "Simulating source "<< sources[i] << std::endl;
					    casa::MDirection direction(MEParsetInterface::asMDirection(parset.getStringVector(oos.str())));
					    itsSim->initFields(casa::String(sources[i]), direction, casa::String(""));
				    }
				    {
					    ostringstream oos;
					    oos << "sources." << sources[i]<< ".model";
					    if(parset.isDefined(oos.str()))
					    {
						    string model=parset.getString(oos.str());
						    os() << "Adding image " << model << " as model for "<< sources[i] << std::endl;
						    ostringstream paramName;
						    paramName << "image.i." << sources[i];
						    SynthesisParamsHelper::getFromCasaImage(*itsModel, paramName.str(), model);
					    }
				    }
			    }
			    os() << "Successfully defined sources"<< std::endl;
		    }

		    void SimParallel::readSpws()
		    {
			    ParameterSet parset(itsParset);
			    if(itsParset.isDefined("spws.definition"))
			    {
				    parset=ParameterSet(substituteWorkerNumber(itsParset.getString("spws.definition")));
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
			    os() << "Successfully defined "<< nSpw << " spectral windows"
			    << std::endl;
		    }

		    void SimParallel::readSimulation()
		    {
			    ParameterSet parset(itsParset);
			    if(itsParset.isDefined("simulation.definition"))
			    {
				    parset=ParameterSet(substituteWorkerNumber(itsParset.getString("simulation.definition")));
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
			    os() << "Successfully set simulation parameters"<< std::endl;
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
					    parset=ParameterSet(substituteWorkerNumber(itsParset.getString("observe.definition")));
				    }

				    int nScans=parset.getInt32("observe.number", 0);
				    CONRADCHECK(nScans>0, "No scans defined");

				    for (int scan=0; scan<nScans; scan++)
				    {
					    ostringstream oos;
					    oos << "observe.scan"<< scan;
					    vector<string> line=parset.getStringVector(oos.str());
					    string source=substituteWorkerNumber(line[0]);
					    string spw=substituteWorkerNumber(line[1]);
					    os() << "Observing scan "<< scan << " on source " << source
					    << " at band " << spw << " from "
					    << MEParsetInterface::asQuantity(line[2]) << " to "
					    << MEParsetInterface::asQuantity(line[3]) << std::endl;
					    itsSim->observe(source, spw,
							    MEParsetInterface::asQuantity(line[2]),
							    MEParsetInterface::asQuantity(line[3]));
				    }

				    os() << "Successfully simulated "<< nScans << " scans"<< std::endl;
				    itsMs->flush();

				    predict(itsMs->tableName());

			    }
		    }
		    void SimParallel::predict(const string& ms)
		    {
			    if(isWorker())
			    {
				    casa::Timer timer;
				    timer.mark();
				    os() << "Simulating data for " << ms << std::endl;
				    os() << "Model is " << *itsModel;
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
				    conrad::scimath::Equation::ShPtr equation(new ImageFFTEquation (*itsModel, it, gridder));
				    equation->predict();
				    os() << "Predicted data for "<< ms << " in "<< timer.real() << " seconds "<< std::endl;
			    }
		    }

	    }
    }
