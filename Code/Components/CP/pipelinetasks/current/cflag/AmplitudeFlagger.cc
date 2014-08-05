/// @file AmplitudeFlagger.cc
///
/// @copyright (c) 2013 CSIRO
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
#include "Common/ParameterSet.h"
#include "casa/aipstype.h"
#include "measures/Measures/MDirection.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "ms/MeasurementSets/MSColumns.h"
#include "measures/Measures/Stokes.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Vector.h"

#include <casa/Utilities/GenSort.h>

#include "boost/tuple/tuple.hpp"
#include "boost/tuple/tuple_comparison.hpp"

// Local package includes
#include "cflag/FlaggingStats.h"

ASKAP_LOGGER(logger, ".AmplitudeFlagger");

using namespace askap;
using namespace casa;
using namespace askap::cp::pipelinetasks;

std::map<rowKey, casa::Vector<casa::Complex> > aveSpectra;
std::map<rowKey, casa::Vector<casa::Int> > countSpectra;
std::map<rowKey, casa::Vector<casa::Bool> > maskSpectra;

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
        : itsStats("AmplitudeFlagger"), itsHasHighLimit(false),
          itsHasLowLimit(false), itsAutoThresholds(false),
          itsIntegrateSpectra(false),
          itsAveAll(false), itsAveragesAreNormalised(true),
          itsThresholdFactor(5.0), itsSpectraFactor(5.0)
{
    if (parset.isDefined("high")) {
        itsHasHighLimit = true;
        itsHighLimit = parset.getFloat("high");
    }
    if (parset.isDefined("low")) {
        itsHasLowLimit = true;
        itsLowLimit = parset.getFloat("low");
    }
    if (parset.isDefined("autoThresholds")) {
        itsAutoThresholds = parset.getBool("autoThresholds");
    }
    if (parset.isDefined("thresholdFactor")) {
        itsThresholdFactor = parset.getFloat("thresholdFactor");
    }
    if (parset.isDefined("integrateSpectra")) {
        itsIntegrateSpectra = parset.getBool("integrateSpectra");
    }
    if (parset.isDefined("integrateSpectra.thresholdFactor")) {
        itsSpectraFactor = parset.getFloat("integrateSpectra.thresholdFactor");
    }
    if (parset.isDefined("aveAll")) {
        itsAveAll = parset.getBool("aveAll");
    }

    if (!itsHasHighLimit && !itsHasLowLimit && !itsAutoThresholds) {
        ASKAPTHROW(AskapError, "No amplitude thresholds have been defined");
    }
    if (itsHasHighLimit && itsHasLowLimit && itsAutoThresholds) {
        ASKAPLOG_WARN_STR(logger, "Amplitude thresholds defined. No autoThresholds");
    }

    // Converts Stokes vector string to StokesType
    if (parset.isDefined("stokes")) {
        vector<string> strvec = parset.getStringVector("stokes");

        for (size_t i = 0; i < strvec.size(); ++i) {
            itsStokes.insert(Stokes::type(strvec[i]));
        }
    }
}

FlaggingStats AmplitudeFlagger::stats(void) const
{
    return itsStats;
}

casa::Bool AmplitudeFlagger::processingRequired(const casa::uInt pass)
{
    if (itsIntegrateSpectra) {
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

    const casa::Vector<casa::Int> stokesTypesInt = getStokesType(msc, row);

    // normalise averages if needed
    if ( itsIntegrateSpectra && !itsAveragesAreNormalised && (pass==1) ) {
        ASKAPLOG_INFO_STR(logger, "At the start of the second pass");
        ASKAPLOG_INFO_STR(logger, " - normalising averages");
        finaliseAverages(aveSpectra, countSpectra, maskSpectra);
    }

    // Iterate over rows (one row is one correlation product)
    for (size_t corr = 0; corr < data.nrow(); ++corr) {

        // If this row doesn't contain a product we are meant to be flagging,
        // then ignore it
        if (!itsStokes.empty() &&
            (itsStokes.find(Stokes::type(stokesTypesInt(corr))) ==
                itsStokes.end())) {
            continue;
        }
 
        // return a tuple that indicate which average this row is in
        rowKey key = getRowKey(msc, row, corr);

        // if this is the first instance of this type of spectrum, initialise it.
        if ( itsIntegrateSpectra && (pass==0) &&
               (aveSpectra.find(key) == aveSpectra.end()) ) {
            aveSpectra[key].resize(data.row(0).shape());
            aveSpectra[key].set(0.0);
            countSpectra[key].resize(data.row(0).shape());
            countSpectra[key].set(0);
            maskSpectra[key].resize(data.row(0).shape());
            maskSpectra[key].set(casa::True);
        }

        // need temporary indicators that can be updated if necessary
        bool hasLowLimit = itsHasLowLimit;
        bool hasHighLimit = itsHasHighLimit;

        // get the spectrum
        casa::Vector<casa::Float>
            spectrumAmplitudes = casa::amplitude(data.row(corr));
        casa::Vector<casa::Bool>
            unflaggedMask = (flags.row(corr)==casa::False);
 
        if ( itsAutoThresholds && (pass==0) ) {

            // check that there is something to flag and return if there isn't
            casa::Bool hasUnflaggedData;
            hasUnflaggedData = (std::find(unflaggedMask.begin(),
                unflaggedMask.end(), casa::True) != unflaggedMask.end());
            if ( !hasUnflaggedData ) {
                itsStats.visAlreadyFlagged += data.ncolumn();
                continue;
            }

            casa::MaskedArray<casa::Float>
                maskedAmplitudes(spectrumAmplitudes, unflaggedMask);
            casa::Vector<casa::Float>
                statsVector = AmplitudeFlagger::getRobustStats(maskedAmplitudes);
            casa::Float median = statsVector[0];
            casa::Float sigma_IQR = statsVector[1];

            if ( !hasLowLimit ) {
                itsLowLimit = median-itsThresholdFactor*sigma_IQR;
                hasLowLimit = casa::True;
            }
            if ( !hasHighLimit ) {
                itsHighLimit = median+itsThresholdFactor*sigma_IQR;
                hasHighLimit = casa::True;
            }
   
            // check min and max relative to thresholds, and do not loop over
            // data again if they are good if indicies were also sorted, could
            // just test where the sorted amplitudes break the threshold...
            if ((statsVector[2] >= itsLowLimit) &&
                (statsVector[3] <= itsHighLimit)) {
                continue;
            }
   
        }

        if ( pass==0 ) {
            for (size_t chan = 0; chan < data.ncolumn(); ++chan) {
                if (flags(corr, chan)) {
                    itsStats.visAlreadyFlagged++;
                    continue;
                }

                const float amp = spectrumAmplitudes(chan);

                if ((hasLowLimit && (amp < itsLowLimit)) ||
                    (hasHighLimit && (amp > itsHighLimit))) {
                    flags(corr, chan) = true;
                    wasUpdated = true;
                    itsStats.visFlagged++;
                }
                else if ( itsIntegrateSpectra ) {
                    aveSpectra[key][chan] += amp;
                    countSpectra[key][chan]++;
                    itsAveragesAreNormalised = casa::False;
                }

            }
        }
        else if ( pass==1 ) {
            // apply any new flags in maskSpectrum
            for (size_t chan = 0; chan < data.ncolumn(); ++chan) {
                // flags is true for flags, maskSpectrum is false for flags
                if (!flags(corr, chan) && !maskSpectra[key][chan]) {
                    flags(corr, chan) = true;
                    wasUpdated = true;
                    itsStats.visFlagged++;
                }
            }
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

rowKey AmplitudeFlagger::getRowKey(
    casa::MSColumns& msc,
    const casa::uInt row,
    const casa::uInt corr)
{

    // specify which fields to keep separate and which to average over
    // any set to zero will be averaged over
    if (itsAveAll) {
        return boost::make_tuple(0,0,0,0,0,0);
    } else {
        return boost::make_tuple(msc.fieldId()(row),
                                 msc.feed1()(row),
                                 msc.feed2()(row),
                                 msc.antenna1()(row),
                                 msc.antenna2()(row),
                                 corr);
    }

}

void AmplitudeFlagger::finaliseAverages(
    std::map<rowKey, casa::Vector<casa::Complex> > &aveSpectra,
    std::map<rowKey, casa::Vector<casa::Int> > &countSpectra, 
    std::map<rowKey, casa::Vector<casa::Bool> > &maskSpectra)
{

    for (std::map<rowKey, casa::Vector<casa::Int> >::iterator
             it=countSpectra.begin(); it!=countSpectra.end(); ++it) {

        // get the spectra
        casa::Vector<casa::Complex> aveSpectrum = aveSpectra[it->first];
        casa::Vector<casa::Int> countSpectrum = countSpectra[it->first];
        casa::Vector<casa::Bool> maskSpectrum = maskSpectra[it->first];

        for (size_t chan = 0; chan < aveSpectrum.size(); ++chan) {
            if (it->second[chan]>0) {
                aveSpectrum[chan] /= casa::Complex(countSpectrum[chan]);
                countSpectrum[chan] = 1;
                maskSpectrum[chan] = casa::True;
            }
            else {
                maskSpectrum[chan] = casa::False;
            }
        }

        itsAveragesAreNormalised = casa::True;

        // generate the flagging stats
        // could fill the un-flagged spectrum directly in the preceding loop.
        casa::MaskedArray<casa::Float>
            maskedAmplitudes(casa::amplitude(aveSpectrum), maskSpectrum);
        casa::Vector<casa::Float>
            statsVector = AmplitudeFlagger::getRobustStats(maskedAmplitudes);
        casa::Float median = statsVector[0];
        casa::Float sigma_IQR = statsVector[1];

        // check min and max relative to thresholds.
        // do not loop over data again if they are good
        if ((statsVector[2] < median-itsSpectraFactor*sigma_IQR) ||
            (statsVector[3] > median+itsSpectraFactor*sigma_IQR)) {

            for (size_t chan = 0; chan < aveSpectrum.size(); ++chan) {
                if (maskSpectrum[chan] == casa::False) continue;
                if ((aveSpectrum[chan] < median-itsSpectraFactor*sigma_IQR) ||
                    (aveSpectrum[chan] > median+itsSpectraFactor*sigma_IQR)) {
                    maskSpectrum[chan] = casa::False;
                }
            }

        }

    }

}

