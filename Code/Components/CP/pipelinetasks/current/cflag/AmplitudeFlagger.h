/// @file AmplitudeFlagger.h
///
/// @copyright (c) 2013,2014 CSIRO
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

#ifndef ASKAP_CP_PIPELINETASKS_AMPLITUDEFLAGGER_H
#define ASKAP_CP_PIPELINETASKS_AMPLITUDEFLAGGER_H

// System includes
#include <set>
#include <vector>

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "boost/shared_ptr.hpp"
#include "casa/aipstype.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"
#include "measures/Measures/Stokes.h"
#include "casa/Arrays/Vector.h"

#include "boost/tuple/tuple.hpp"
#include "boost/tuple/tuple_comparison.hpp"

// Local package includes
#include "cflag/IFlagger.h"
#include "cflag/FlaggingStats.h"

namespace askap {
namespace cp {
namespace pipelinetasks {

//                   fieldID    feed1      feed2      antenna1   antenna2   polarisation
typedef boost::tuple<casa::Int, casa::Int, casa::Int, casa::Int, casa::Int, casa::Int> rowKey;

/// @brief Applies flagging based on amplitude thresholding.
class AmplitudeFlagger : public IFlagger {
    public:

        /// @brief Constructs zero or more instances of the AmplitudeFlagger.
        /// The flagger is responsible for reading the "parset" and constructing
        /// zero or more instances of itself, depending on the configuration.
        ///
        /// @throw AskapError   If an upper or lower threshold is not specified
        ///                     in the parset.
        static vector< boost::shared_ptr<IFlagger> > build(
                const LOFAR::ParameterSet& parset,
                const casa::MeasurementSet& ms);

        /// @brief Constructor
        /// @throw AskapError   If an upper or lower threshold is not specified
        ///                     in the parset.
        AmplitudeFlagger(const LOFAR::ParameterSet& parset);

        /// @see IFlagger::processRow()
        virtual void processRow(casa::MSColumns& msc, const casa::uInt pass,
                                const casa::uInt row, const bool dryRun);

        /// @see IFlagger::stats()
        virtual FlaggingStats stats(void) const;

        /// @see IFlagger::stats()
        virtual casa::Bool processingRequired(const casa::uInt pass);

    private:

        // load and log relevant parset parameters
        void loadParset(const LOFAR::ParameterSet& parset);
        void logParsetSummary(const LOFAR::ParameterSet& parset);

        /// Returns a vector of stokes types for a given row in the main table
        /// of the measurement set. This will have the same dimension and
        /// ordering as the data/flag matrices.
        casa::Vector<casa::Int> getStokesType(casa::MSColumns& msc,
                                              const casa::uInt row);

        // Flagging statistics
        FlaggingStats itsStats;

        // True if an upper amplitude limit has been set, otherwise false
        bool itsHasHighLimit;
        // True if lower amplitude limit has been set, otherwise false
        bool itsHasLowLimit;
        // The upper amplitude limit
        casa::Float itsHighLimit;
        // The lower amplitude limit
        casa::Float itsLowLimit;

        // Automatically set either of these limits that are unset
        bool itsAutoThresholds;
        // sigma multiplier used to set cutoffs
        casa::Float itsThresholdFactor;

        // Generate averaged spectra and search these for peaks to flag
        bool itsIntegrateSpectra;
        // sigma multiplier used to set cutoffs
        casa::Float itsSpectraFactor;

        // Generate averaged time series and search these for peaks to flag
        bool itsIntegrateTimes;
        // sigma multiplier used to set cutoffs
        casa::Float itsTimesFactor;

        // When integrating, do not separate spectra based on baseline, etc.
        bool itsAveAll;
        // When integrating, do separate spectra for different polarisations
        bool itsAveAllButPol;
        // When integrating, do separate spectra for different beams
        bool itsAveAllButBeam;

        // When integrating, used to limit flag generation to a single call to
        // "processRow"
        bool itsAverageFlagsAreReady;

        // The set of correlation products for which these flagging rules should
        // be applied. An empty list means apply to all correlation products.
        std::set<casa::Stokes::StokesTypes> itsStokes;

        // Maps of storage vectors for averaging spectra and generating flags
        std::map<rowKey, casa::Vector<casa::Double> > itsAveSpectra;
        std::map<rowKey, casa::Vector<casa::Bool> > itsMaskSpectra;
        std::map<rowKey, casa::Vector<casa::Int> > itsCountSpectra;

        // Maps of storage vectors for averaging time series and generating flags
        std::map<rowKey, casa::Vector<casa::Float> > itsAveTimes;
        std::map<rowKey, casa::Vector<casa::Bool> > itsMaskTimes;
        std::map<rowKey, casa::Int> itsCountTimes;

        // Generate a tuple for a given row and polarisation
        rowKey getRowKey(casa::MSColumns& msc, const casa::uInt row,
            const casa::uInt corr);

        // Calculate the median, the interquartile range, the min and the max
        // of a masked array
        casa::Vector<casa::Float>
            getRobustStats(casa::MaskedArray<casa::Float> maskedAmplitudes);

        // Set flags based on integrated quantities
        void setFlagsFromIntegrations(void);

};

}
}
}

#endif
