/// @file MSSink.h
///
/// @copyright (c) 2010 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

#ifndef ASKAP_CP_INGEST_MSSINK_H
#define ASKAP_CP_INGEST_MSSINK_H

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"
#include "Common/ParameterSet.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "casa/aips.h"
#include "casa/BasicSL.h"
#include "casa/Quanta.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Matrix.h"
#include "cpcommon/VisChunk.h"

// Local package includes
#include "ingestpipeline/ITask.h"
#include "configuration/Configuration.h" // Includes all configuration attributes too

namespace askap {
namespace cp {
namespace ingest {

/// @brief A sink task for the central processor ingest pipeline which writes
/// the data out to a measurement set.
/// 
/// When constructing this class a measurement set is created, the default tables
/// are created and the ANTENNA, FEEDS, and OBSERVATION tables are populated
/// based on the "Configuration" instance passed to the constructor.
///
/// As observing takes place process() is called for each integration cycle. If
/// the VisChunk passed to process() is the first chunk for a new scan then rows
/// are added to the SPECTRAL WINDOW, POLARIZATION and DATA DESCRIPTION tables.
/// The visibilities and related data are also written into the main table.
class MSSink : public askap::cp::ingest::ITask {
    public:
        /// @brief Constructor.
        /// @param[in] parset   the parameter set used to configure this task.
        /// @param[in] config   an object containing the system configuration.
        MSSink(const LOFAR::ParameterSet& parset,
                const Configuration& config);

        /// @brief Destructor.
        virtual ~MSSink();

        /// @brief Writes out the data in the VisChunk parameter to the 
        /// measurement set.
        ///
        /// @param[in,out] chunk    the instance of VisChunk to write out. Note
        ///                         the VisChunk pointed to by "chunk" nor the pointer
        ///                         itself are modified by this function.
        virtual void process(askap::cp::common::VisChunk::ShPtr chunk);

    private:
        // Initialises the ANTENNA table
        void initAntennas(void);

        // Initialises the FEED table
        void initFeeds(const FeedConfig& feeds, const casa::Int antennaID);

        // Initialises the OBSERVATION table
        void initObs(void);

        // Create the measurement set
        void create(void);

        // Add observation table row
        casa::Int addObs(const casa::String& telescope,
                const casa::String& observer,
                const double obsStartTime,
                const double obsEndTime);

        // Add field table row
        casa::Int addField(const casa::String& fieldName,
                const casa::MDirection& fieldDirection,
                const casa::String& calCode);

        // Add feeds table rows
        void addFeeds(const casa::Int antennaID,
                const casa::Vector<double>& x,
                const casa::Vector<double>& y,
                const casa::Vector<casa::String>& polType);

        // Add antenna table row
        casa::Int addAntenna(const casa::String& station,
                const casa::Vector<double>& antXYZ,
                const casa::String& name,
                const casa::String& mount,
                const casa::Double& dishDiameter);

        // Add data description table row
        casa::Int addDataDesc(const casa::Int spwId, const casa::Int polId);

        // Add spectral window table row
        casa::Int addSpectralWindow(const casa::String& name,
                const int nChan,
                const casa::Quantity& startFreq,
                const casa::Quantity& freqInc);

        // Add polarisation table row
        casa::Int addPolarisation(const casa::Vector<casa::Stokes::StokesTypes>& stokesTypes);

        // Find or add a FIELD table entry for the provided scan index number.
        casa::Int findOrAddField(const casa::Int scanId);

        // Find or add a DATA DESCRIPTION (including SPECTRAL INDEX and POLARIZATION)
        // table entry for the provided scan index number.
        casa::Int findOrAddDataDesc(askap::cp::common::VisChunk::ShPtr chunk);

        // Compares the given row in the spectral window table with the spectral window
        // setup as defined in the Scan.
        bool isSpectralWindowRowEqual(askap::cp::common::VisChunk::ShPtr chunk,
                const casa::uInt row) const;

        // Compares the given row in the polarisation table with the polarisation
        // setup as defined in the Scan.
        bool isPolarisationRowEqual(askap::cp::common::VisChunk::ShPtr chunk,
                const casa::uInt row) const;

        // Helper function to compare MDirections
        static bool equal(const casa::MDirection &dir1, const casa::MDirection &dir2);

        // Parameter set
        const LOFAR::ParameterSet itsParset;

        // Configuration object
        const Configuration itsConfig;

        // The index number of the scan for the previous VisChunk. Some things
        // (such as spectral window or field) are allowed to change from scan
        // to scan, this allows a new scan to be detected
        casa::Int itsPreviousScanIndex;

        // The current field row. This is cached until the scan index is
        // incremented
        casa::Int itsFieldRow;

        // The current data description row. This is cached until the scan
        // index is incremented
        casa::Int itsDataDescRow;

        // Measurement set
        boost::scoped_ptr<casa::MeasurementSet> itsMs;

        // No support for assignment
        MSSink& operator=(const MSSink& rhs);

        // No support for copy constructor
        MSSink(const MSSink& src);
};

}
}
}

#endif
