/// @file
///
/// @brief Base class for parallel applications
/// @details
/// Supports algorithms by providing methods for initialization
/// of MPI connections, sending and models around.
/// There is assumed to be one master and many workers.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
/// 

#include <iostream>
#include <sstream>

#include <casa/OS/Timer.h>

#include <conrad/ConradError.h>

#include <parallel/SimParallel.h>

#include <measurementequation/MEParsetInterface.h>

#include <APS/ParameterSet.h>

using namespace std;

using namespace conrad;
using namespace conrad::cp;
using namespace LOFAR::ACC::APS;

namespace conrad
{
	namespace synthesis
	{

		SimParallel::SimParallel(int argc, const char** argv, const ParameterSet& parset) :
			SynParallel(argc, argv), itsParset(parset)
		{
			os() << "CONRAD simulation program"<< std::endl;

			string msname(parset.getString("DataSet", "test.ms"));
			itsSim=boost::shared_ptr<casa::NewMSSimulator> (new casa::NewMSSimulator(msname));

			// Make a MeasurementSet object for the disk-based MeasurementSet that we just
			// created. We need to explicitly flush it to disk - probably this should 
			/// be rectified in casa::NewMSSimulator eventually
			itsMs=boost::shared_ptr<casa::MeasurementSet> (new casa::MeasurementSet(msname, 
					casa::Table::Update));

			/// The antenna info is kept in a separate parset file
			readTelescope();

			/// Get the source definitions.
			readSources();

			/// Get the feed definitions
			readFeeds();

			/// Get the spectral window definitions. Not all of these need to be
			/// used.
			readSpw();

			/// Get miscellaneous information about the simulation
			readSimulation();
		}

		SimParallel::~SimParallel()
		{
			itsMs->flush();
		}

		void SimParallel::readTelescope()
		{
			ParameterSet parset(itsParset.getString("telescope", "ASKAP45.in"));

			/// Csimulator.name = ASKAP
			string telName=parset.getString("name");
			os() << "Simulating "<< telName << std::endl;
			ostringstream oos;
			oos << telName << ".";
			ParameterSet antParset(parset.makeSubset(oos.str()));

			/// Csimulator.ASKAP.number=45
			int nAnt=antParset.getInt32("number", 0);
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
			/// Csimulator.ASKAP.antenna0=[x,y,z]
			/// ...
			for (int iant=0; iant<nAnt; iant++)
			{
				ostringstream oos;
				oos << "antenna"<< iant;
				vector<float> xyz=antParset.getFloatVector(oos.str());
				x[iant]=xyz[0];
				y[iant]=xyz[1];
				z[iant]=xyz[2];
				mounts[iant]=mount;
				dishDiameter[iant]=diameter;
				oos.clear();
				oos << telName << iant;
				name[iant]=oos.str();
			}

			/// Csimulator.ASKAP.location=[+115deg, -26deg, 192km, WGS84]
			casa::MPosition
			    location=MEParsetInterface::asMPosition(antParset.getStringVector("location"));

			itsSim->initAnt(telName, x, y, z, dishDiameter, offset, mounts, name,
			    casa::String(coordinates), location);

			os() << "Successfully defined "<< nAnt << " antennas of "<< telName
			    << std::endl;
		}

		void SimParallel::readFeeds()
		{
			ParameterSet parset(itsParset.makeSubset("feeds."));

			casa::String mode=parset.getString("mode", "perfect X Y");
			int nFeeds=parset.getInt32("number", 0);
			casa::Vector<double> x(2*nFeeds);
			casa::Vector<double> y(2*nFeeds);
			casa::Vector<casa::String> pol(2*nFeeds);
			for (int feed=0; feed<nFeeds; feed++)
			{
				ostringstream os;
				os << "feed"<< feed;
				vector<double> xy(parset.getDoubleVector(os.str()));
				x[2*feed]=xy[0];
				y[2*feed]=xy[1];
				x[2*feed+1]=xy[0];
				y[2*feed+1]=xy[1];
				pol[2*feed]="X";
				pol[2*feed+1]="Y";
			}
			itsSim->initFeeds(mode, x, y, pol);
			os() << "Successfully defined "<< nFeeds << " feeds"<< std::endl;
		}

		/// Csimulator.sources.names = [3C273, 1934-638]
		/// Csimulator.sources.3C273.direction = 
		/// Csimulator.sources.1934-638.direction =
		void SimParallel::readSources()
		{
			ParameterSet parset(itsParset.makeSubset("sources."));

			const vector<string> sources=parset.getStringVector("names");
			for (int i=0; i<sources.size(); ++i)
			{
				ostringstream oos;
				oos << sources[i]<< ".direction";
				os() << "Simulating source "<< oos.str() << std::endl;
				casa::MDirection direction=MEParsetInterface::asMDirection(parset.getStringVector(oos.str()));
				itsSim->initFields(casa::String(sources[i]), direction, casa::String(""));
			}
			os() << "Successfully defined sources"<< std::endl;
		}

		/// Csimulator.spw.number=2
		/// Csimulator.spw.spw1=[LBand1, 128, 1.420GHz, -1MHz, "XX XY YX YY"]
		/// Csimulator.spw.spw2=[LBand2, 128, 1.000GHz, -1MHz, "XX XY YX YY"]
		void SimParallel::readSpw()
		{
			ParameterSet parset(itsParset.makeSubset("spw."));
			int nSpw=parset.getInt32("number");
			CONRADCHECK(nSpw>0, "No spectral windows defined");
			for (int spw=0; spw<nSpw; spw++)
			{
				ostringstream os;
				os << "spw"<< spw;
				vector<string> line=parset.getStringVector(os.str());
				itsSim->initSpWindows(line[0], MEParsetInterface::asInteger(line[1]),
				    MEParsetInterface::asQuantity(line[2]),
				    MEParsetInterface::asQuantity(line[3]),
				    MEParsetInterface::asQuantity(line[3]), line[4]);
			}
			os() << "Successfully defined "<< nSpw << " spectral windows"
			    << std::endl;
		}

		void SimParallel::readSimulation()
		{
			ParameterSet parset(itsParset.makeSubset("simulate."));

			/// Csimulator.simulate.blockage=0.1
			itsSim->setFractionBlockageLimit(parset.getDouble("blockage", 0.0));
			/// Csimulator.simulate.elevationlimit=8deg
			itsSim->setElevationLimit(MEParsetInterface::asQuantity(parset.getString(
			    "elevationlimit", "8deg")));
			/// Csimulator.simulate.autocorrwt=0.0
			itsSim->setAutoCorrelationWt(parset.getFloat("autocorrwt", 0.0));

			/// Csimulator.simulate.integrationtime=10s
			casa::Quantity
			    integrationTime(MEParsetInterface::asQuantity(parset.getString(
			        "integrationtime", "10s")));
			/// Csimulator.simulate.usehourangles=true
			bool useHourAngles(parset.getBool("usehourangles", true));
			/// Csimulator.simulate.referencetime=2007Mar07
			vector<string> refTimeString(parset.getStringVector("referencetime"));
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
			/// Now that the simulator is defined, we can observe each scan
			ParameterSet parset(itsParset.makeSubset("observe."));

			int nScans=parset.getInt32("number", 0);
			CONRADCHECK(nScans>0, "No scans defined");

			for (int scan=0; scan<nScans; scan++)
			{
				ostringstream oos;
				oos << "scan"<< scan;
				os() << "Observing "<< oos.str() << std::endl;
				vector<string> line=parset.getStringVector(oos.str());
				itsSim->observe(line[0], line[1],
				    MEParsetInterface::asQuantity(line[2]),
				    MEParsetInterface::asQuantity(line[3]));
			}
			os() << "Successfully simulated "<< nScans << " scans"<< std::endl;
			itsMs->flush();
		}

	}
}
