/// @file
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>

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
		/// @brief Simulator for synthesis observing 
		///
		/// @details Cloned from casa::NewMSSimulator to allow substantial changes.
		/// @ingroup simulation

		class Simulator
		{
			public:

				/// Constructor from name only
				/// @param msname Name of measurement set to be constructed
				Simulator(const casa::String& msname);

				/// Constructor from existing MS
				/// @param ms Existing MeasurementSet object
				Simulator(casa::MeasurementSet& ms);

				/// Copy constructor - for completeness only
				/// @param mss Simulator to be copied
				Simulator(const Simulator & mss);

				//# Destructor
				~Simulator();

				/// Assignment
				/// @param mss Simulator to be assigned from 
				Simulator & operator=(const Simulator & mss);

				/// Set maximum amount of data (bytes) to be written into any one
				/// scratch column hypercube
				/// @param maxdata Maximum number of bytes to write per hypercube
				void setMaxData(const double maxdata=2e9)
				{
					maxData_p=maxdata;
				}

				/// @brief Set the antenna and array data. 
				/// @details These are written immediately to the
				/// existing MS. The same model is used for the other init information
				/// @param telname Telescope name e.g. "ASKAP"
				/// @param x X coordinate for antennas (m)
				/// @param y Y coordinate for antennas (m)
				/// @param z Z coordinate for antennas (m)
				/// @param dishdiameter Dish diameters (m)
				/// @param offset Offset (m)
				/// @param mount Mount type (alt-az or equatorial)
				/// @param name Antenna name e.g. ASKAP23
				/// @param coordsystem Coordinate system local or global
				/// @param location MPosition of local coordinate system
				void initAnt(const casa::String& telname,
				    const casa::Vector<double>& x, const casa::Vector<double>& y,
				    const casa::Vector<double>& z,
				    const casa::Vector<double>& dishdiameter,
				    const casa::Vector<double>& offset,
				    const casa::Vector<casa::String>& mount,
				    const casa::Vector<casa::String>& name,
				    const casa::String& coordsystem, const casa::MPosition& location);

				/// @brief Set the observed fields
				/// @param sourcename Source name
				/// @param sourcedirection MDirection for source
				/// @param calcode Calibrator code
				void
				    initFields(const casa::String& sourcename,
				        const casa::MDirection& sourcedirection,
				        const casa::String& calcode);

				/// @brief Set the Feeds
				/// @param mode "Perfect X Y" or "Perfect R L"
				/// @param x Offset of feed (radians)
				/// @param y Offset of feed (radians)
				/// @param pol Polarisation of feed e.g. "R L"
				void
				    initFeeds(const casa::String& mode, const casa::Vector<double>& x,
				        const casa::Vector<double>& y,
				        const casa::Vector<casa::String>& pol);

				/// @brief Set the spectral windows information
				/// @param spwindowname Name of spectral window e.g. "Continuum6"
				/// @param nchan Number of channels e.g. 16
				/// @param startfreq Starting frequency e.g. "1.420GHz"
				/// @param freqinc Frequency increment e.g. "1MHz"
				/// @param freqres Frequency resolution e.g. "1MHz"
				/// @param stokes Stokes to be measured e.g. "XX XY YX YY"
				void initSpWindows(const casa::String& spwindowname, const int& nchan,
				    const casa::Quantity& startfreq, const casa::Quantity& freqinc,
				    const casa::Quantity& freqres, const casa::String& stokes);

				/// @brief Set maximum tolerable blockage before flagging
				/// @param fraclimit Maximim tolerable blockage (fraction) e.g. 0.1
				void setFractionBlockageLimit(const double fraclimit)
				{
					fractionBlockageLimit_p = fraclimit;
				}

				/// @brief Set minimum allowed elevation before flagging
				/// @param ellimit Minimum allowed elevation e.g. "8deg"
				void setElevationLimit(const casa::Quantity& ellimit)
				{
					elevationLimit_p = ellimit;
				}

				/// @brief Set autocorrelation weight (zero for no auto's)
				/// @param autocorrwt Weight of autocorrelation data
				void setAutoCorrelationWt(const float autocorrwt)
				{
					autoCorrelationWt_p = autocorrwt;
				}

				/// @brief Set meaning of times for observing
				/// @param integrationtime Integration time e.g. "10s"
				/// @param usehourangles Use hour angles for observing times?
				/// @param reftime Reference time to which all times are referred
				void settimes(const casa::Quantity& integrationtime,
				    const bool usehourangles, const casa::MEpoch& reftime);

				/// @brief Observe a given source 
				/// @param sourcename Source to be observed (as defined in initFields)
				/// @param spwindowname Spectral window (as defined in InitSpw)
				/// @param starttime Starting time e.g. "1h"
				/// @param stoptime Stopping time e.g. "1h30m"
				/// @details Generate the empty data rows for the observing. All the
				/// relevant information must have been filled in by previous init
				/// calls. 
				void observe(const casa::String& sourcename,
				    const casa::String& spwindowname, const casa::Quantity& starttime,
				    const casa::Quantity& stoptime);

			private:

				/// Prevent use of default constructor
				Simulator()
				{
				}

				/// Fractional blockage limit
				double fractionBlockageLimit_p;
				/// Elevation limit
				casa::Quantity elevationLimit_p;
				/// Autocorrelation weight
				float autoCorrelationWt_p;
				/// Telescope name
				casa::String telescope_p;
				/// Integration time as a Quantity
				casa::Quantity qIntegrationTime_p;
				/// Use hour angles?
				bool useHourAngle_p;
				/// Is the hour angle defined?
				bool hourAngleDefined_p;
				/// Reference time
				casa::MEpoch mRefTime_p;
				/// Offset time as a double
				double t_offset_p;
				/// Amount of data written
				double dataWritten_p;
				/// Hyper cube ID for data columns
				int hyperCubeID_p;
				/// Are we using hyper cubes?
				bool hasHyperCubes_p;
				/// Last spectral window written
				int lastSpWID_p;

				/// Measurement set points
				casa::MeasurementSet* ms_p;

				/// Data accessors for the various storage managers
				casa::TiledDataStManAccessor dataAcc_p;
				/// Data accessors for the various storage managers
				casa::TiledDataStManAccessor scratchDataAcc_p;
				/// Data accessors for the various storage managers
				casa::TiledDataStManAccessor sigmaAcc_p;
				/// Data accessors for the various storage managers
				casa::TiledDataStManAccessor flagAcc_p;
				/// Data accessors for the various storage managers
				casa::TiledDataStManAccessor imweightAcc_p;

				/// Maximum data to be written (only needed for 32 bit)
				double maxData_p;

				/// Convert local coordinates to global
				/// @param xreturned Converted x coordinate
				/// @param yreturned Converted y coordinate
				/// @param zreturned Converted z coordinate
				/// @param location Reference location
				/// @param xin Input x coordinate
				/// @param yin Input y coordinate
				/// @param zin Input z coordinate
				void local2global(casa::Vector<double>& xreturned,
				    casa::Vector<double>& yreturned, casa::Vector<double>& zreturned,
				    const casa::MPosition& location, const casa::Vector<double>& xin,
				    const casa::Vector<double>& yin, const casa::Vector<double>& zin);

				/// Convert global coordinates to local
				/// @param xreturned Converted x coordinate
				/// @param yreturned Converted y coordinate
				/// @param zreturned Converted z coordinate
				/// @param location Reference location
				/// @param xin Input x coordinate
				/// @param yin Input y coordinate
				/// @param zin Input z coordinate
				void longlat2global(casa::Vector<double>& xreturned,
				    casa::Vector<double>& yreturned, casa::Vector<double>& zreturned,
				    const casa::MPosition& location, const casa::Vector<double>& xin,
				    const casa::Vector<double>& yin, const casa::Vector<double>& zin);

				/// Returns the fractional blockage of one antenna by another
				/// @param fraction1 Fraction of 1 blocked by 2
				/// @param fraction2 Fraction of 2 blocked by 1
				/// @param uvw UVW coordinates (same units as diameter)
				/// @param diam1 Diameter of antenna 1
				/// @param diam2 Diameter of antenna 2
				void blockage(double &fraction1, double &fraction2,
				    const casa::Vector<double>& uvw, // uvw in same units as diam!
				    const double diam1, const double diam2);

				/// Provide nicely formatted direction
				/// @param direction Direction to be formatted
				casa::String formatDirection(const casa::MDirection& direction);

				/// Provide nicely formatted time
				/// @param time Time to be formatted
				casa::String formatTime(const double time);

				/// Add hypercubes
				/// @param id Id to be used
				/// @param nbase Number of baselines
				/// @param nchan Number of channels
				/// @param ncorr Number of correlations
				void addHyperCubes(const int id, const int nbase, const int nchan,
				    const int ncorr);

				/// Restore default values
				void defaults();

		};

	}

}
#endif

