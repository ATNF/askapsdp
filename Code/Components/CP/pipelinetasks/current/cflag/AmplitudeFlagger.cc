/// @file AmplitudeFlagger.cc
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

// Include own header file first
#include "AmplitudeFlagger.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <limits>
#include <set>
#include <vector>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "boost/tuple/tuple.hpp"
#include "boost/tuple/tuple_comparison.hpp"
#include "Common/ParameterSet.h"
#include "casa/aipstype.h"
#include "measures/Measures/MDirection.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"
#include "measures/Measures/Stokes.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Vector.h"
#include "casa/Utilities/GenSort.h"

// Local package includes
#include "cflag/FlaggingStats.h"

ASKAP_LOGGER(logger, ".AmplitudeFlagger");

using namespace askap;
using namespace casa;
using namespace askap::cp::pipelinetasks;

vector< boost::shared_ptr<IFlagger> > AmplitudeFlagger::build(
        const LOFAR::ParameterSet& parset,
        const casa::MeasurementSet& /*ms*/)
{
    vector< boost::shared_ptr<IFlagger> > flaggers;
    const string key = "amplitude_flagger.enable";
    if (parset.isDefined(key) && parset.getBool(key)) {
        const LOFAR::ParameterSet subset = parset.makeSubset("amplitude_flagger.");
        flaggers.push_back(boost::shared_ptr<IFlagger>(new AmplitudeFlagger(subset)));
    }
    return flaggers;
}

AmplitudeFlagger::AmplitudeFlagger(const LOFAR::ParameterSet& parset)
        : itsStats("AmplitudeFlagger"),
          itsHasHighLimit(false), itsHasLowLimit(false),
          itsAutoThresholds(false), itsThresholdFactor(5.0),
          itsIntegrateSpectra(false), itsSpectraFactor(5.0),
          itsIntegrateTimes(false), itsTimesFactor(5.0),
          itsAveAll(false), itsAveAllButPol(false), itsAveAllButBeam(false),
          itsAverageFlagsAreReady(true)
{
    // check parset
    AmplitudeFlagger::loadParset(parset);
    // log parameter summary
    AmplitudeFlagger::logParsetSummary(parset);
}

FlaggingStats AmplitudeFlagger::stats(void) const
{
    return itsStats;
}

casa::Bool AmplitudeFlagger::processingRequired(const casa::uInt pass)
{
    if (itsIntegrateSpectra || itsIntegrateTimes) {
        return (pass<2);
    } else {
        return (pass<1);
    }
}

casa::Vector<casa::Int> AmplitudeFlagger::getStokesType(
        casa::MSColumns& msc, const casa::uInt row)
{
    const casa::ROMSDataDescColumns& ddc = msc.dataDescription();
    const int dataDescId = msc.dataDescId()(row);
    const casa::ROMSPolarizationColumns& polc = msc.polarization();
    const unsigned int descPolId = ddc.polarizationId()(dataDescId);
    return polc.corrType()(descPolId);
}

void AmplitudeFlagger::processRow(casa::MSColumns& msc, const casa::uInt pass,
                                  const casa::uInt row, const bool dryRun)
{
    const Matrix<casa::Complex> data = msc.data()(row);
    Matrix<casa::Bool> flags = msc.flag()(row);

    // Only need to write out the flag matrix if it was updated
    bool wasUpdated = false;
    // Only set flagRow if all corr are flagged
    // Only looking for row flags in "itsAveTimes" data. Could generalise.
    bool leaveRowFlag = false;

    const casa::Vector<casa::Int> stokesTypesInt = getStokesType(msc, row);

    // normalise averages and search them for peaks to flag
    if ( !itsAverageFlagsAreReady && (pass==1) ) {
        ASKAPLOG_INFO_STR(logger, "Finalising averages at the start of pass "
            <<pass+1);
        setFlagsFromIntegrations();
    }

    // Iterate over rows (one row is one correlation product)
    for (size_t corr = 0; corr < data.nrow(); ++corr) {

        // If this row doesn't contain a product we are meant to be flagging,
        // then ignore it
        if (!itsStokes.empty() &&
            (itsStokes.find(Stokes::type(stokesTypesInt(corr))) ==
                itsStokes.end())) {
            leaveRowFlag = true;
            continue;
        }

        // return a tuple that indicate which integration this row is in
        rowKey key = getRowKey(msc, row, corr);

        // update a counter for this row and the storage vectors
        // do it before any processing that is dependent on "pass"
        if ( itsIntegrateTimes ) {
            updateTimeVectors(key, pass);
        }

        // if this is the first instance of this key, initialise storage vectors
        if ( itsIntegrateSpectra && (pass==0) &&
               (itsAveSpectra.find(key) == itsAveSpectra.end()) ) {
            initSpectrumVectors(key, data.row(0).shape());
        }

        // need temporary indicators that can be updated if necessary
        bool hasLowLimit = itsHasLowLimit;
        bool hasHighLimit = itsHasHighLimit;

        // get the spectrum
        casa::Vector<casa::Float>
            spectrumAmplitudes = casa::amplitude(data.row(corr));

        // set a mask (only needed when averaging, so move this if need be)
        casa::Vector<casa::Bool>
            unflaggedMask = (flags.row(corr)==casa::False);

        if ( itsAutoThresholds ) {
            // check that there is something to flag and continue if there isn't
            if (std::find(unflaggedMask.begin(),
                     unflaggedMask.end(), casa::True) == unflaggedMask.end()) {
                itsStats.visAlreadyFlagged += data.ncolumn();
                if ( itsIntegrateTimes ) {
                   itsMaskTimes[key][itsCountTimes[key]] = casa::False;
                }
                continue;
            }
        }

        // individual flagging and averages are only done during the first pass
        // could change this
        if ( pass==0 ) {

            if ( itsAutoThresholds ) {
         
                // combine amplitudes with mask and get the median-based statistics
                casa::MaskedArray<casa::Float>
                    maskedAmplitudes(spectrumAmplitudes, unflaggedMask);
                casa::Vector<casa::Float>
                    statsVector = getRobustStats(maskedAmplitudes);
                casa::Float median = statsVector[0];
                casa::Float sigma_IQR = statsVector[1];
         
                // set cutoffs
                if ( !hasLowLimit ) {
                    itsLowLimit = median-itsThresholdFactor*sigma_IQR;
                    hasLowLimit = casa::True;
                }
                if ( !hasHighLimit ) {
                    itsHighLimit = median+itsThresholdFactor*sigma_IQR;
                    hasHighLimit = casa::True;
                }
         
                // check min and max relative to thresholds, and do not loop over
                // data again if they are good. If indicies were also sorted, could
                // just test where the sorted amplitudes break the threshold...
                // ** cannot do this when averages are needed, or they'll be skipped **
                if (!itsIntegrateSpectra && !itsIntegrateTimes &&
                        (statsVector[2] >= itsLowLimit) &&
                        (statsVector[3] <= itsHighLimit)) {
                    continue;
                }

            }

            // only need these if itsIntegrateTimes
            casa::Double aveTime = 0.0;
            casa::uInt countTime = 0;

            // look for individual peaks and do any integrations
            for (size_t chan = 0; chan < data.ncolumn(); ++chan) {
                if (flags(corr, chan)) {
                    itsStats.visAlreadyFlagged++;
                    continue;
                }

                // look for individual peaks
                const float amp = spectrumAmplitudes(chan);
                if ((hasLowLimit && (amp < itsLowLimit)) ||
                    (hasHighLimit && (amp > itsHighLimit))) {
                    flags(corr, chan) = true;
                    wasUpdated = true;
                    itsStats.visFlagged++;
                }
                else if ( itsIntegrateSpectra || itsIntegrateTimes ) {
                    if ( itsIntegrateSpectra ) {
                        // do spectra integration
                        itsAveSpectra[key][chan] += amp;
                        itsCountSpectra[key][chan]++;
                        itsAverageFlagsAreReady = casa::False;
                    }
                    if ( itsIntegrateTimes ) {
                        // do time-series integration
                        aveTime += amp;
                        countTime++;
                    }
                }

            }
            if ( itsIntegrateTimes ) {
                // normalise this average
                if ( countTime>0 ) {
                    itsAveTimes[key][itsCountTimes[key]] =
                        aveTime/casa::Double(countTime);
                    itsMaskTimes[key][itsCountTimes[key]] = casa::True;
                    itsAverageFlagsAreReady = casa::False;
                }
                else {
                    itsMaskTimes[key][itsCountTimes[key]] = casa::False;
                }
            }

        }
        else if ( (pass==1) &&  ( itsIntegrateSpectra || itsIntegrateTimes ) ) {
            // only flag unflagged data, so that new flags can be counted.
            // "flags" is true for flags, "mask*" are false for flags
            if ( itsIntegrateTimes ) {
                // apply itsMaskTimes flags. Could just use flagRow,
                // but not sure that all applications support flagRow
                if ( !itsMaskTimes[key][itsCountTimes[key]] ) {
                    for (size_t chan = 0; chan < data.ncolumn(); ++chan) {
                        if (!flags(corr, chan)) {
                            flags(corr, chan) = true;
                            wasUpdated = true;
                            itsStats.visFlagged++;
                        }
                    }
                    // everything is flagged, so move to the next "corr"
                    continue;
                }
                // need this to be false for all "corr" to warrent flagRow
                else leaveRowFlag = true;
            }
            // apply itsIntegrateSpectra flags
            if ( itsIntegrateSpectra ) {
                for (size_t chan = 0; chan < data.ncolumn(); ++chan) {
                    if ( !flags(corr, chan) && !itsMaskSpectra[key][chan] ) {
                        flags(corr, chan) = true;
                        wasUpdated = true;
                        itsStats.visFlagged++;
                    }
                }
            }
        }

    }

    if (wasUpdated && itsIntegrateTimes && !leaveRowFlag && (pass==1)) {
        itsStats.rowsFlagged++;
        if (!dryRun) {
            msc.flagRow().put(row, true);
        }
    }
    if (wasUpdated && !dryRun) {
        msc.flag().put(row, flags);
    }
}


// return the median, the interquartile range, and the min/max of a masked array
casa::Vector<casa::Float>AmplitudeFlagger::getRobustStats(
    casa::MaskedArray<casa::Float> maskedAmplitudes)
{
    casa::Vector<casa::Float> statsVector(4);
   
    // Grab unflagged frequency channels and sort their amplitudes.
    // From casacore comments:
    // "use HeapSort as it's performance is guaranteed, quicksort is often
    // extremely slow (O(n*n)) for inputs with many successive duplicates".

    // extract all of the unflagged amplitudes
    casa::Array<casa::Float>
        sortedAmplitudes = maskedAmplitudes.getCompressedArray();

    // sort the unflagged amplitudes
    casa::Int numUnflagged=GenSort<casa::Float>::sort(sortedAmplitudes,
        Sort::Ascending, Sort::HeapSort);
   
    // estimate stats, assuming Gaussian noise dominates the frequency channels.
    // (50% of a Gaussian dist. is within 0.67448 sigma of the mean...)
    statsVector[0] = sortedAmplitudes.data()[numUnflagged/2]; // median
    statsVector[1] = (sortedAmplitudes.data()[3*numUnflagged/4] - 
        sortedAmplitudes.data()[numUnflagged/4]) / 1.34896; // sigma from IQR
    statsVector[2] = sortedAmplitudes.data()[0]; // min
    statsVector[3] = sortedAmplitudes.data()[numUnflagged-1]; // max

    return(statsVector);   

}

// Generate a tuple for a given row and polarisation
rowKey AmplitudeFlagger::getRowKey(
    casa::MSColumns& msc,
    const casa::uInt row,
    const casa::uInt corr)
{

    // specify which fields to keep separate and which to average over
    // any set to zero will be averaged over
    casa::Int field = 0;
    casa::Int feed1 = 0;
    casa::Int feed2 = 0;
    casa::Int ant1  = 0;
    casa::Int ant2  = 0;
    casa::Int pol   = 0;
    if (itsAveAll) {
        if (itsAveAllButPol) {
            pol = corr;
        }
        if (itsAveAllButBeam) {
            feed1 = msc.feed1()(row);
            feed2 = msc.feed2()(row);
        }
    } else {
        field = msc.fieldId()(row);
        feed1 = msc.feed1()(row);
        feed2 = msc.feed2()(row);
        ant1  = msc.antenna1()(row);
        ant2  = msc.antenna2()(row);
        pol   = corr;
    }

    return boost::make_tuple(field,feed1,feed2,ant1,ant2,pol);

}

void AmplitudeFlagger::updateTimeVectors(const rowKey &key, const casa::uInt pass)
{
    if (itsCountTimes.find(key) == itsCountTimes.end()) {
        itsCountTimes[key] = 0; // init counter for this key
    }
    else {
        itsCountTimes[key]++;
    }
    if ( pass==0 ) {
        itsAveTimes[key].resize(itsCountTimes[key]+1,casa::True);
        itsMaskTimes[key].resize(itsCountTimes[key]+1,casa::True);
        itsMaskTimes[key][itsCountTimes[key]] = casa::True;
    }
}

void AmplitudeFlagger::initSpectrumVectors(const rowKey &key, const casa::IPosition &shape)
{
    itsAveSpectra[key].resize(shape);
    itsAveSpectra[key].set(0.0);
    itsCountSpectra[key].resize(shape);
    itsCountSpectra[key].set(0);
    itsMaskSpectra[key].resize(shape);
    itsMaskSpectra[key].set(casa::True);
}

// Set flags based on integrated quantities
void AmplitudeFlagger::setFlagsFromIntegrations(void)
{

    if ( itsIntegrateSpectra ) {

        for (std::map<rowKey, casa::Vector<casa::Double> >::iterator
             it=itsAveSpectra.begin(); it!=itsAveSpectra.end(); ++it) {

            // get the spectra
            casa::Vector<casa::Float> aveSpectrum(it->second.shape());
            casa::Vector<casa::Int> countSpectrum = itsCountSpectra[it->first];
            casa::Vector<casa::Bool> maskSpectrum = itsMaskSpectra[it->first];
         
            for (size_t chan = 0; chan < aveSpectrum.size(); ++chan) {
                if (countSpectrum[chan]>0) {
                    aveSpectrum[chan] = it->second[chan] /
                                        casa::Double(countSpectrum[chan]);
                    countSpectrum[chan] = 1;
                    maskSpectrum[chan] = casa::True;
                }
                else {
                    maskSpectrum[chan] = casa::False;
                }
            }
         
            // generate the flagging stats. could fill the unflagged spectrum
            // directly in the preceding loop, but the full vector is needed below
            casa::MaskedArray<casa::Float>
                maskedAmplitudes(aveSpectrum, maskSpectrum);
            casa::Vector<casa::Float>
                statsVector = getRobustStats(maskedAmplitudes);
            casa::Float median = statsVector[0];
            casa::Float sigma_IQR = statsVector[1];
         
            // check min and max relative to thresholds.
            // do not loop over data again if all unflagged channels are good
            if ((statsVector[2] < median-itsSpectraFactor*sigma_IQR) ||
                (statsVector[3] > median+itsSpectraFactor*sigma_IQR)) {
         
                for (size_t chan = 0; chan < aveSpectrum.size(); ++chan) {
                    if (maskSpectrum[chan]==casa::False) continue;
                    if ((aveSpectrum[chan]<median-itsSpectraFactor*sigma_IQR) ||
                        (aveSpectrum[chan]>median+itsSpectraFactor*sigma_IQR)) {
                        maskSpectrum[chan]=casa::False;
                    }
                }
         
            }
         
        }

    }

    if ( itsIntegrateTimes ) {

        for (std::map<rowKey, casa::Vector<casa::Float> >::iterator
             it=itsAveTimes.begin(); it!=itsAveTimes.end(); ++it) {

            // reset the counter for this key
            itsCountTimes[it->first] = -1;
         
            // get the spectra
            casa::Vector<casa::Float> aveTime = it->second;
            casa::Vector<casa::Bool> maskTime = itsMaskTimes[it->first];
         
            // generate the flagging stats
            casa::MaskedArray<casa::Float> maskedAmplitudes(aveTime, maskTime);
            casa::Vector<casa::Float>
                statsVector = getRobustStats(maskedAmplitudes);
            casa::Float median = statsVector[0];
            casa::Float sigma_IQR = statsVector[1];
         
            // check min and max relative to thresholds.
            // do not loop over data again if all unflagged times are good
            if ((statsVector[2] < median-itsTimesFactor*sigma_IQR) ||
                (statsVector[3] > median+itsTimesFactor*sigma_IQR)) {
         
                for (size_t t = 0; t < aveTime.size(); ++t) {
                    if (maskTime[t] == casa::False) continue;
                    if ((aveTime[t] < median-itsTimesFactor*sigma_IQR) ||
                        (aveTime[t] > median+itsTimesFactor*sigma_IQR)) {
                        maskTime[t] = casa::False;
                    }
                }
         
            }

        }

    }

    itsAverageFlagsAreReady = casa::True;

}

// load relevant parset parameters
void AmplitudeFlagger::loadParset(const LOFAR::ParameterSet& parset)
{

    if (parset.isDefined("high")) {
        itsHasHighLimit = true;
        itsHighLimit = parset.getFloat("high");
    }
    if (parset.isDefined("low")) {
        itsHasLowLimit = true;
        itsLowLimit = parset.getFloat("low");
    }
    if (parset.isDefined("dynamicBounds")) {
        itsAutoThresholds = parset.getBool("dynamicBounds");
    }
    if (parset.isDefined("threshold")) {
        itsThresholdFactor = parset.getFloat("threshold");
    }
    if (parset.isDefined("integrateSpectra")) {
        itsIntegrateSpectra = parset.getBool("integrateSpectra");
        if (parset.isDefined("integrateSpectra.threshold")) {
            itsSpectraFactor = parset.getFloat("integrateSpectra.threshold");
        }
    }
    if (parset.isDefined("integrateTimes")) {
        itsIntegrateTimes = parset.getBool("integrateTimes");
        if (parset.isDefined("integrateTimes.threshold")) {
            itsTimesFactor = parset.getFloat("integrateTimes.threshold");
        }
    }
    if (parset.isDefined("aveAll")) {
        itsAveAll = parset.getBool("aveAll");
        if (parset.isDefined("aveAll.noPol")) {
            itsAveAllButPol = parset.getBool("aveAll.noPol");
        }
        if (parset.isDefined("aveAll.noBeam")) {
            itsAveAllButBeam = parset.getBool("aveAll.noBeam");
        }
    }

    // Converts Stokes vector string to StokesType
    if (parset.isDefined("stokes")) {
        vector<string> strvec = parset.getStringVector("stokes");

        for (size_t i = 0; i < strvec.size(); ++i) {
            itsStokes.insert(Stokes::type(strvec[i]));
        }
    }

}

// add a summary of the relevant parset parameters to the log
void AmplitudeFlagger::logParsetSummary(const LOFAR::ParameterSet& parset)
{

    ASKAPLOG_INFO_STR(logger, "Parameter Summary:");

    if (!itsHasHighLimit && !itsHasLowLimit && !itsAutoThresholds &&
        !itsIntegrateSpectra && !itsIntegrateTimes) {
        ASKAPTHROW(AskapError, "No amplitude flagging has been defined");
    }
    if (itsAutoThresholds) {
        if (itsHasHighLimit && itsHasLowLimit) {
            ASKAPLOG_WARN_STR(logger,
                "Amplitude thresholds defined. No auto-threshold");
        }
        if (itsHasHighLimit) {
            ASKAPLOG_INFO_STR(logger, "High threshold set to "<<itsHighLimit);
        } else {
            ASKAPLOG_INFO_STR(logger,
                "High threshold set automatically with threshold factor of "<<
                itsThresholdFactor);
        }
        if (itsHasLowLimit) {
            ASKAPLOG_INFO_STR(logger, "Low threshold set to "<<itsLowLimit);
        } else {
            ASKAPLOG_INFO_STR(logger,
                "Low threshold set automatically with threshold factor of "<<
                itsThresholdFactor);
        }
    }
    if (itsIntegrateSpectra) {
        ASKAPLOG_INFO_STR(logger,
            "Searching for outliers in integrated spectra with a "
            <<itsSpectraFactor<<"-sigma cutoff");
    }
    if (itsIntegrateTimes) {
        ASKAPLOG_INFO_STR(logger,
            "Searching for outliers in integrated time series with a "
            <<itsTimesFactor<<"-sigma cutoff");
    }
    if (itsAveAll && (itsIntegrateSpectra || itsIntegrateTimes)) {
        if (itsAveAllButPol || itsAveAllButBeam) {
            ASKAPLOG_INFO_STR(logger,
                " - except for the following, will ignore properites when integrating");
            if (itsAveAllButPol) {
                ASKAPLOG_INFO_STR(logger,
                    "   * keeping polarisations separate");
            }
            if (itsAveAllButBeam) {
                ASKAPLOG_INFO_STR(logger,
                    "   * keeping beams separate");
            }
        }
        else {
            ASKAPLOG_INFO_STR(logger,
                " - ignoring visibility properites when integrating");
        }
    }

}

