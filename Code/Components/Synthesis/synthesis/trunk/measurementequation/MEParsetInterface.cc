/// @file
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///

#include <casa/aips.h>
#include <casa/Quanta.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/Measure.h>

#include <conrad/ConradError.h>

#include <measurementequation/MEParsetInterface.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <measurementequation/ImageSolver.h>
#include <measurementequation/ImageMultiScaleSolver.h>

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <sstream>

using namespace std;
using namespace conrad;

namespace conrad
{
	namespace synthesis
	{

		void operator<<(conrad::scimath::Params::ShPtr& params,
				const LOFAR::ACC::APS::ParameterSet &parset)
		{
			vector<string> images=parset.getStringVector("Names");
			for (vector<string>::iterator it=images.begin();it!=images.end();it++)
			{
				std::vector<int> shape=parset.getInt32Vector(*it+".shape");
				int nchan=parset.getInt32(*it+".nchan");
				std::vector<double> freq=parset.getDoubleVector(*it+".frequency");
				std::vector<std::string> direction=parset.getStringVector(*it+".direction");
				std::vector<std::string> cellsize=parset.getStringVector(*it+".cellsize");

				SynthesisParamsHelper::add(*params, *it, direction, cellsize, shape,
						freq[0], freq[1], nchan);
			}
		}

		void operator<<(conrad::scimath::Solver::ShPtr& solver, 
				const LOFAR::ACC::APS::ParameterSet &parset)
		{
			conrad::scimath::Params params;
			if(parset.getString("solver")=="Clean")
			{
				std::vector<float> defaultScales(3);
				defaultScales[0]=0.0;
				defaultScales[1]=10.0;
				defaultScales[2]=30.0;
				std::vector<float> scales=parset.getFloatVector("solver.Clean.scales", defaultScales);
				solver = conrad::scimath::Solver::ShPtr(new ImageMultiScaleSolver(params, casa::Vector<float>(scales)));
				std::cout << "Constructed image multiscale solver" << std::endl;
				solver->setGain(parset.getFloat("solver.Clean.gain", 0.7));
				solver->setAlgorithm(parset.getString("solver.Clean.algorithm", "MultiScale"));
				solver->setVerbose(parset.getBool("solver.Clean.verbose", true));
				solver->setNiter(parset.getInt32("solver.Clean.niter", 100));
				casa::Quantity threshold;
				casa::Quantity::read(threshold, parset.getString("solver.Clean.threshold", "0Jy"));
				solver->setThreshold(threshold);
			}
			else
			{
				solver = conrad::scimath::Solver::ShPtr(new ImageSolver(params));
				casa::Quantity threshold;
				casa::Quantity::read(threshold, parset.getString("solver.Dirty.threshold", "0Jy"));
				solver->setThreshold(threshold);
				std::cout << "Constructed dirty image solver" << std::endl;
			}
		}

		int MEParsetInterface::asInteger(const string& s)
		{
			int result;
			istringstream is(s);
			is>>result;
			return result;
		}

		casa::Quantity MEParsetInterface::asQuantity(const string& s)
		{
			casa::Quantity q;
			casa::Quantity::read(q, s);
			return q;
		}

		casa::MEpoch MEParsetInterface::asMEpoch(const vector<string>& epoch)
		{
			CONRADCHECK(epoch.size()==2, "Not a valid epoch");

			casa::Quantity datetime;
			casa::Quantity::read(datetime, epoch[0]);
			casa::MEpoch::Types type;
			casa::MEpoch::getType(type, epoch[1]);
			casa::MEpoch ep(datetime, type);
			return ep;
		}

		casa::MDirection MEParsetInterface::asMDirection(const vector<string>& direction)
		{
			CONRADCHECK(direction.size()==3, "Not a valid direction");

			casa::Quantity lng;
			casa::Quantity::read(lng, direction[0]);
			casa::Quantity lat;
			casa::Quantity::read(lat, direction[1]);
			casa::MDirection::Types type;
			casa::MDirection::getType(type, direction[2]);
			casa::MDirection dir(lng, lat, type);
			return dir;
		}

		casa::MPosition MEParsetInterface::asMPosition(const vector<string>& position)
		{
			CONRADCHECK(position.size()==4, "Not a valid position");

			casa::Quantity lng;
			casa::Quantity::read(lng, position[0]);
			casa::Quantity lat;
			casa::Quantity::read(lat, position[1]);
			casa::Quantity height;
			casa::Quantity::read(height, position[2]);
			casa::MPosition::Types type;
			casa::MPosition::getType(type, position[3]);
			casa::MVPosition mvPos(height, lng, lat);
			casa::MPosition pos(mvPos, type);
			return pos;
		}

	}
}
