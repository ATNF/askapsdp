// @file CalibrationParameters.ice
//
// @copyright (c) 2009 CSIRO
// Australia Telescope National Facility (ATNF)
// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
// PO Box 76, Epping NSW 1710, Australia
// atnf-enquiries@csiro.au
//
// This file is part of the ASKAP software distribution.
//
// The ASKAP software distribution is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef ASKAP_CALIBRATION_PARAMETERS_ICE
#define ASKAP_CALIBRATION_PARAMETERS_ICE

#include <CommonTypes.ice>

module askap 
{

module interfaces
{

module calparams 
{

/** Jones Index */
struct JonesIndex {
    short antennaID;
    short beamID;
};

/**
 * Jones J-Term 
 */
struct JonesJTerm {
    // Gain for polarisation 1
    DoubleComplex g1;

    // Flag for polarisation 1 gain, indicating validity.
    bool g1Valid;

    // Gain for polarisation 2
    DoubleComplex g2;

    // Flag for polarisation 2 gain, indicating validity.
    bool g2Valid;
};
 
// Only one independent value, i.e. leakage + cross-pol phase
// so DoubleComplex == JonesDTerm;

/** Sequence of J-terms, i.e. a bandpass */
["java:type:java.util.ArrayList<askap.interfaces.calparams.JonesJTerm>"]
sequence<JonesJTerm> JonesJTermSeq;

struct FrequencyDependentJTerm {
    /** Bandpass solution. The sequence is size nChan */
    JonesJTermSeq bandpass;
};

/** Gain solution indexed with JonesIndex */
dictionary<JonesIndex,JonesJTerm> GainSolution;

/** Leakage solution indexed with JonesIndex */
dictionary<JonesIndex,DoubleComplex> LeakageSolution;

/** Bandpass solution indexed with JonesIndex */
dictionary<JonesIndex,FrequencyDependentJTerm> BandpassSolution;
 
/** Structure containing gains solution plus a timestamp */
struct TimeTaggedGainSolution {
    /** Absolute time expressed as microseconds since MJD=0. */
    long timestamp;

    /** Gain solution */
    GainSolution gain;
};

/** Structure containing leakage solution plus a timestamp */
struct TimeTaggedLeakageSolution {
    /** Absolute time expressed as microseconds since MJD=0. */
    long timestamp;

    /** Leakage solution */
    LeakageSolution leakage;
};

/** Structure containing bandpass solution plus a timestamp */
struct TimeTaggedBandpassSolution {
    /** Absolute time expressed as microseconds since MJD=0. */
    long timestamp;

    /** Number of channels */
    int nChan;

    /** Bandpass solution */
    BandpassSolution bandpass;
};

// Design Note: Structures above describe the calibration solutions as such (i.e.
// a number of parameters for a given time). There could be a situation when parameters
// corresponding to different antenna/beam (indexed by JonesIndex) were determined at a
// different time. Therefore, it would be handy to have a buffer of the parameters which
// are actually applied (it may be a responsibility of another piece of code to poll the
// database and populate this buffer with the most recent solutions for every
// calibration parameter. The ingest pipeline would access this buffer and just applyall
// values. In the future, the structure is likely to change, because it should be aligned
// with the actual architecture. Interfaces below are given as an example or a starting
// point. They essentially pass the same information as the interfaces given above but
// in somewhat transposed form.

/** Status enum for a calibration parameter */
enum CalParamStatus {

    /**
     * The appropriate parameter has a default value (i.e. 1.0 for gains), neither
     * automatic calibration pipeline, nor manual adjustment was done. The related
     * timestamp field is meaningless.
     */
    DEFAULT,

    /**
     * The value is set by operator (or any actor other than the calibration
     * pipeline) at the given time.
     * Design Note:  Do we need a special option which ensures that the appropriate
     * value is not be overwritten by the calibration pipeline?
     */
    USERDEFINED,

    /**
     * The value is a result of the calibration pipeline execution at the given
     * time and it is considered to be valid.
     */
    DERIVED,

    /**
     * The pipeline failed to determine the value for some reason. This option is
     * present here for consistency, although the merging logic should probably
     * keep the previous value/status for the given antenna/beam if the solution
     * for an update fails for some reason. The time field shows when the solution
     * for an update has been attempted.
     */
    FAILED,

    /**
     * The value is invalidated by some other mechanism, i.e. the operator
     * intervention. The merging logic should probably keep it invalid when
     * the next solution arrives, unlike the FAILED case which should be replaced
     * by the new soltuion if it is valid. The ingest pipeline behavior may also be
     * different in this case (i.e. it may flag this beam/antenna in the dataset
     * prepared for the calibration pipeline)
     */
    INVALID
};

/** All calibration information for a given antenna/beam combination */
struct CalibrationParameters {
    /** Parallel-hand gain */
    JonesJTerm gain;

    /** Cross-pol leakage and phase (D-term) */
    DoubleComplex leakage;

    /** Bandpass */
    FrequencyDependentJTerm bandpass;

    // Design Note: If we need a separate delay term of a frequency-dependent D-term,
    // they could be added here. Beamformer weights for this particular beam may be
    // given here too (perhaps per coarse channel)

    /**
     * Time the gain solution was determined.
     * Absolute time expressed as microseconds since MJD=0.
     */
    long gainTimestamp;

    /**
     * Time the leakage solution was determined.
     * Absolute time expressed as microseconds since MJD=0.
     */
    long leakageTimestamp;

    /**
     * Time the bandpass solution was determined.
     * Absolute time expressed as microseconds since MJD=0.
     */
    long bandpassTimestamp;

    /** Status flag for the gain solution */
    CalParamStatus gainStatus;

    /** Status flag for the leakage solution */
    CalParamStatus leakageStatus;

    /** Status flag for the bandpass solution */
    CalParamStatus bandpassStatus;
};

/** Map type for calibration parameters */
dictionary<JonesIndex,CalibrationParameters> CalibrationParametersMap;

};

};

};

#endif // #ifndef ASKAP_CALIBRATION_PARAMETERS_ICE
