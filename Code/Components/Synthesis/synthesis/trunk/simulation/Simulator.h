#ifndef CONRAD_SYNTHESIS_SIMULATOR_H
#define CONRAD_SYNTHESIS_SIMULATOR_H

//# Includes
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MDirection.h>
#include <tables/Tables/TiledDataStManAccessor.h>
#include <ms/MeasurementSets/MeasurementSet.h>

namespace conrad
{
	namespace synthesis
	{

		class Simulator
		{
			public:

				// Constructor from name only
				Simulator(const casa::String&);

				// Constructor from existing MS
				Simulator(casa::MeasurementSet&);

				// Copy constructor - for completeness only
				Simulator(const Simulator & mss);

				//# Destructor
				~Simulator();

				//# Operators
				// Assignment
				Simulator & operator=(const Simulator &);

				// Set maximum amount of data (bytes) to be written into any one
				// scratch column hypercube
				void setMaxData(const double maxData=2e9)
				{
					maxData_p=maxData;
				}
				;

				// set the antenna and array data. These are written immediately to the
				// existing MS. The same model is used for the other init infor.
				void initAnt(const casa::String& telname,
				    const casa::Vector<double>& x, const casa::Vector<double>& y,
				    const casa::Vector<double>& z,
				    const casa::Vector<double>& dishDiameter,
				    const casa::Vector<double>& offset,
				    const casa::Vector<casa::String>& mount,
				    const casa::Vector<casa::String>& name,
				    const casa::String& coordsystem, const casa::MPosition& mRefLocation);

				// set the observed fields
				void initFields(const casa::String& sourceName,
				    const casa::MDirection& sourceDirection, const casa::String& calCode);

				// set the Feeds;  brain dead version
				void initFeeds(const casa::String& mode);

				// set the Feeds;  Smart version
				void
				    initFeeds(const casa::String& mode, const casa::Vector<double>& x,
				        const casa::Vector<double>& y,
				        const casa::Vector<casa::String>& pol);

				// set the spectral windows information
				void initSpWindows(const casa::String& spWindowName, const int& nChan,
				    const casa::Quantity& startFreq, const casa::Quantity& freqInc,
				    const casa::Quantity& freqRes,
				    const casa::String& stokes);

				void setFractionBlockageLimit(const double fraclimit)
				{
					fractionBlockageLimit_p = fraclimit;
				}

				void setElevationLimit(const casa::Quantity& ellimit)
				{
					elevationLimit_p = ellimit;
				}

				void setAutoCorrelationWt(const float autocorrwt)
				{
					autoCorrelationWt_p = autocorrwt;
				}

				void settimes(const casa::Quantity& qIntegrationTime,
				    const bool useHourAngles, const casa::MEpoch& mRefTime);

				void observe(const casa::String& sourceName,
				    const casa::String& spWindowName, const casa::Quantity& qStartTime,
				    const casa::Quantity& qStopTime);

			private:

				// Prevent use of default constructor
				Simulator()
				{
				}
				;

				//# Data Members
				double fractionBlockageLimit_p;
				casa::Quantity elevationLimit_p;
				float autoCorrelationWt_p;
				casa::String telescope_p;
				casa::Quantity qIntegrationTime_p;
				bool useHourAngle_p;
				bool hourAngleDefined_p;
				casa::MEpoch mRefTime_p;
				double t_offset_p;
				double dataWritten_p;
				int hyperCubeID_p;
				bool hasHyperCubes_p;
				int lastSpWID_p;

				casa::MeasurementSet* ms_p;

				casa::TiledDataStManAccessor dataAcc_p, scratchDataAcc_p, sigmaAcc_p,
				    flagAcc_p, imweightAcc_p;

				double maxData_p;

				void local2global(casa::Vector<double>& xReturned,
				    casa::Vector<double>& yReturned, casa::Vector<double>& zReturned,
				    const casa::MPosition& mRefLocation,
				    const casa::Vector<double>& xIn, const casa::Vector<double>& yIn,
				    const casa::Vector<double>& zIn);

				void longlat2global(casa::Vector<double>& xReturned,
				    casa::Vector<double>& yReturned, casa::Vector<double>& zReturned,
				    const casa::MPosition& mRefLocation,
				    const casa::Vector<double>& xIn, const casa::Vector<double>& yIn,
				    const casa::Vector<double>& zIn);

				// Returns the fractional blockage of one antenna by another
				// We will want to put this somewhere else eventually, but I don't yet know where!
				// Till then.
				// fraction1: fraction of antenna 1 that is blocked by 2
				// fraction2: fraction of antenna 2 that is blocked by 1
				// hint: at least one of the two will be 0.0
				void blockage(double &fraction1, double &fraction2,
				    const casa::Vector<double>& uvw, // uvw in same units as diam!
				    const double diam1, const double diam2);

				casa::String formatDirection(const casa::MDirection&);
				casa::String formatTime(const double);

				void addHyperCubes(const int id, const int nBase, const int nChan,
				    const int nCorr);

				void defaults();

		};

	}

}
#endif

