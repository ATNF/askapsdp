///
/// @file 
///
/// Control parameters are passed in from a LOFAR ParameterSet file.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
#include <conrad/ConradError.h>

#include <ms/MeasurementSets/NewMSSimulator.h>

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <sstream>

#include <casa/aips.h>
#include <casa/Quanta.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MEpoch.h>

#include <APS/ParameterSet.h>

using std::cout;
using std::endl;

using namespace std;
using namespace casa;
using namespace conrad;
using namespace LOFAR::ACC::APS;

int asInteger(const string& s)
{
	int result;
	istringstream is(s);
	is>>result;
	return result;
}

casa::Quantity asQuantity(const string& s)
{
	casa::Quantity q;
	casa::Quantity::read(q, s);
	return q;
}

casa::MEpoch asMEpoch(const vector<string>& epoch)
{
	CONRADCHECK(epoch.size()==2, "Not a valid epoch");

	casa::Quantity datetime;
	casa::Quantity::read(datetime, epoch[0]);
	MEpoch::Types type;
	MEpoch::getType(type, epoch[1]);
	casa::MEpoch ep(datetime, type);
	return ep;
}

casa::MDirection asMDirection(const vector<string>& direction)
{
	CONRADCHECK(direction.size()==3, "Not a valid direction");

	casa::Quantity lng;
	casa::Quantity::read(lng, direction[0]);
	casa::Quantity lat;
	casa::Quantity::read(lat, direction[1]);
	MDirection::Types type;
	MDirection::getType(type, direction[2]);
	casa::MDirection dir(lng, lat, type);
	return dir;
}

casa::MPosition asMPosition(const vector<string>& position)
{
	CONRADCHECK(position.size()==4, "Not a valid position");

	casa::Quantity lng;
	casa::Quantity::read(lng, position[0]);
	casa::Quantity lat;
	casa::Quantity::read(lat, position[1]);
	casa::Quantity height;
	casa::Quantity::read(height, position[2]);
	MPosition::Types type;
	MPosition::getType(type, position[3]);
	casa::MVPosition mvPos(height, lng, lat);
	casa::MPosition pos(mvPos, type);
	return pos;
}

void readTelescope(NewMSSimulator& sim, const ParameterSet& parset)
{
	/// Csimulator.name = ASKAP
	string telName=parset.getString("name");
	std::cout << "Simulating "<< telName << std::endl;
	ostringstream os;
	os << telName << ".";
	ParameterSet antParset(parset.makeSubset(os.str()));

	/// Csimulator.ASKAP.number=45
	int nAnt=antParset.getInt32("number", 0);
	CONRADCHECK(nAnt>0, "No antennas defined in parset file");

	/// Csimulator.ASKAP.mount=equatorial
	string mount=antParset.getString("mount", "equatorial");
	CONRADCHECK((mount=="equatorial")||(mount=="alt-az"), "Antenna mount unknown");

	/// Csimulator.ASKAP.mount=equatorial
	double diameter=asQuantity(antParset.getString("diameter", "12m")).getValue("m");
	CONRADCHECK(diameter>0.0, "Antenna diameter not positive");

	casa::Vector<double> x(nAnt);
	casa::Vector<double> y(nAnt);
	casa::Vector<double> z(nAnt);

	casa::Vector<double> dishDiameter(nAnt);

	casa::Vector<double> offset(nAnt);
	offset.set(0.0);
	casa::Vector<casa::String> mounts(nAnt);
	casa::Vector<casa::String> name(nAnt);

	/// Antenna information in the form:
	/// Csimulator.ASKAP.antenna1=[x,y,z]
	/// ...
	for (int iant=0; iant<nAnt; iant++)
	{
		ostringstream os;
		os << "antenna"<< iant;
		vector<float> xyz=antParset.getFloatVector(os.str());
		x[iant]=xyz[0];
		y[iant]=xyz[1];
		z[iant]=xyz[2];
		mounts[iant]=mount;
		dishDiameter[iant]=diameter;
		os.clear();
		os << telName << iant;
		name[iant]=os.str();
	}
	/// Csimulator.ASKAP.location=[+115deg, -26deg, 192km, WGS84]
	casa::MPosition location=asMPosition(antParset.getStringVector("location"));
	
	sim.initAnt(telName, x, y, z, dishDiameter, offset, mounts, name,
	    casa::String("local"), location);
	std::cout << "Successfully defined "<< nAnt << " antennas of "<< telName << std::endl;
}

void readFeeds(NewMSSimulator& sim, const ParameterSet& parset)
{
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
	sim.initFeeds(mode, x, y, pol);
	std::cout << "Successfully defined " << nFeeds << " feeds"<< std::endl;
}

/// Csimulator.sources.names = [3C273, 1934-638]
/// Csimulator.sources.3C273.direction = 
/// Csimulator.sources.1934-638.direction =
void readSources(NewMSSimulator& sim, const ParameterSet& parset)
{
	const vector<string> sources=parset.getStringVector("names");
	for (int i=0; i<sources.size(); ++i)
	{
		ostringstream os;
		os << sources[i]<< ".direction";
		std::cout << "Simulating source "<< os.str() << std::endl;
		MDirection direction=asMDirection(parset.getStringVector(os.str()));
		sim.initFields(casa::String(sources[i]), direction, String(""));
	}
	std::cout << "Successfully defined sources" << std::endl;
}

/// Csimulator.spw.number=2
/// Csimulator.spw.spw1=[LBand1, 128, 1.420GHz, -1MHz, "XX XY YX YY"]
/// Csimulator.spw.spw2=[LBand2, 128, 1.000GHz, -1MHz, "XX XY YX YY"]
void readSpw(NewMSSimulator& sim, const ParameterSet& parset)
{
	int nSpw=parset.getInt32("number");
	CONRADCHECK(nSpw>0, "No spectral windows defined");
	for (int spw=0; spw<nSpw; spw++)
	{
		ostringstream os;
		os << "spw"<< spw;
		vector<string> line=parset.getStringVector(os.str());
		sim.initSpWindows(line[0], asInteger(line[1]), asQuantity(line[2]),
		    asQuantity(line[3]), asQuantity(line[3]), line[4]);
	}
	std::cout << "Successfully defined " << nSpw << " spectral windows"<< std::endl;
}

void readSimulation(NewMSSimulator& sim, const ParameterSet& parset)
{
	/// Csimulator.simulate.blockage=0.1
	sim.setFractionBlockageLimit(parset.getDouble("blockage", 0.0));
	/// Csimulator.simulate.elevationlimit=8deg
	sim.setElevationLimit(asQuantity(parset.getString("elevationlimit", "8deg")));
	/// Csimulator.simulate.autocorrwt=0.0
	sim.setAutoCorrelationWt(parset.getFloat("autocorrwt", 0.0));

	/// Csimulator.simulate.integrationtime=10s
	casa::Quantity integrationTime(asQuantity(parset.getString("integrationtime", "10s")));
	/// Csimulator.simulate.usehourangles=true
	bool useHourAngles(parset.getBool("usehourangles", true));
	/// Csimulator.simulate.referencetime=2007Mar07
	vector<string> refTimeString(parset.getStringVector("referencetime"));
	casa::MEpoch refTime(asMEpoch(refTimeString));
	sim.settimes(integrationTime, useHourAngles, refTime);
	std::cout << "Successfully set simulation parameters"<< std::endl;
}

/// Csimulator.observe.number=2
/// Csimulator.scan1=[1934-638, LBand1, 0s, 120s]
/// Csimulator.scan2=[3C273, LBand1, 120s, 1920s]
/// etc.
void readObserve(NewMSSimulator& sim, const ParameterSet& parset)
{
	int nScans=parset.getInt32("number", 0);
	CONRADCHECK(nScans>0, "No scans defined");

	for (int scan=0;scan<nScans;scan++)
	{
		ostringstream os;
		os << "scan" << scan;
		std::cout << "Observing " << os.str() << std::endl;
		vector<string> line=parset.getStringVector(os.str());
		sim.observe(line[0], line[1], asQuantity(line[2]), asQuantity(line[3]));
	}
	std::cout << "Successfully simulated " << nScans << " scans" << std::endl;	
}

// Main function

int main(int argc, const char** argv)
{
	try
	{

		std::cout << "CONRAD simulation program" << std::endl;

		ParameterSet parset("csimulator.in");

		NewMSSimulator sim(parset.getString("DataSet", "test.ms"));

		ParameterSet subset(parset.makeSubset("Csimulator."));

		ParameterSet telescopeParset(subset.getString("telescope", "ASKAP45.in"));
		readTelescope(sim, telescopeParset);

		ParameterSet sourceParset(subset.makeSubset("sources."));
		readSources(sim, sourceParset);

		ParameterSet feedParset(subset.makeSubset("feeds."));
		readFeeds(sim, feedParset);

		ParameterSet spwParset(subset.makeSubset("spw."));
		readSpw(sim, spwParset);
		
		ParameterSet simulateParset(subset.makeSubset("simulate."));
		readSimulation(sim, simulateParset);

		ParameterSet observeParset(subset.makeSubset("observe."));
		readObserve(sim, observeParset);


		///==============================================================================
	}
	catch (conrad::ConradError& x)
	{
		std::cerr << "Conrad error in " << argv[0] << ": " << x.what() << std::endl;
		exit(1);
	}
	catch (std::exception& x)
	{
		std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
		exit(1);
	}
	exit(0);
}

