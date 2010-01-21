// @file
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

module calibparams 
{

struct JonesIndex {
   short AntennaID;
   short BeamID;
};
 
// sequence may be an overkill here, but one
// have to account for the possibility that
// either g11 or g22 is undefined. Bool flag instead?
struct JonesJTerm {
    // gain for polarisation 1
    DoubleComplexSeq g1;
    // gain for polarisation 2
    DoubleComplexSeq g2;
};
 
// only one independent value, i.e. leakage + cross-pol phase
// so DoubleComplex == JonesDTerm;

// sequence of J-terms, i.e. a bandpass
sequence<JonesJTerm> JonesJTermSeq;

struct FrequencyDependentJTerm {
  int nchan;
  JonesJTermSeq bandpass;
  // some frequency vs. channel mapping
  // parameters can come here, e.g. freqID
  // but we don't need them for processing
  // However, it may be good to be able to apply last bandpass
  // determined for a particular frequency configuration
};

// gain solution indexed with JonesIndex
dictionary<JonesIndex,JonesJTerm> GainSolution;

// leakage solution indexed with JonesIndex
dictionary<JonesIndex,DoubleComplex> LeakageSolution;

// bandpass solution indexed with JonesIndex
dictionary<JonesIndex,FrequencyDependentJTerm> BandpassSolution;
 
 
// it looks like there is no benefit from introducing a polymorphism for
// the following types
struct TimeTaggedGainSolution {
  long timestamp;
  GainSolution gains;
};
 
struct TimeTaggedLeakageSolution {
   long timestamp;
   LeakageSolution leakages;
};
 
struct TimeTaggedBandpassSolution {
  long timestamp;
  BandpassSolution bandpasses;
};

// structures above describe the calibration solutions as such (i.e. a number of parameters for a given time)
// There could be a situation when parameters corresponding to different antenna/beam (indexed by JonesIndex) were
// determined at a different time. Therefore, it would be handy to have a buffer of the parameters which are actually
// applied (it may be a responsibility of another piece of code to poll the database and populate this buffer with 
// the most recent solutions for every calibration parameter. The frontend would access this buffer and just apply
// all values. In the future, the structure is likely to change, because it should be aligned with the actual architecture.
// Interfaces below are given as an example or a starting point. They essentially pass the same information as the interfaces
// given above but in somewhat transposed form

// status enum for a calibration parameter
// DEFAULT - the appropriate parameter has a default value (i.e. 1.0 for gains), neither automatic calibration pipeline, nor
//           manual adjustment was done. The appropriate time field is meaningless.
// USERDEFINED - the value is set by operator (or any actor other than the calibration pipeline) at the given time. Do we need
//               a special option which ensures that the appropriate value is not be overwritten by the calibration pipeline?
// DERIVED - the value is a result of the calibration pipeline execution at the given time and it is considered to be valid
// FAILED - the pipeline failed to determine the value for some reason. This option is present here for consistency, although the
//          merging logic should probably keep the previous value/status for the given antenna/beam if the solution for an update
//          fails for some reason. The time field shows when the solution for an update has been attempted.
// INVALID - the value is invalidated by some other mechanism, i.e. the operator intervention. The merging logic should probably keep
//           it invalid when the next solution arrives, unlike the FAILD case which should be replaced by the new soltuion if it is
//           valid. The frontend behavior may also be different in this case (i.e. it may flag this beam/antenna in the dataset
//           prepared for the calibration pipeline)
enum CalParamStatus { DEFAULT, USERDEFINED, DERIVED, FAILED, INVALID };

// all calibration information for a given antenna/beam combination
struct CalibrationParameters {
  // parallel-hand gain
  JonesJTerm gain;
  // cross-pol leakage and phase (D-term)
  DoubleComplex leakage;
  // bandpass
  FrequencyDependentJTerm bandpass;
  // if we need a separate delay term of a frequency-dependent D-term, they could be added here.
  // Beamformer weights for this particular beam may be given here too (perhaps per coarse channel)

  // time tags for each parameter showing when it was actually determined (so we can always trace the whole solution in the
  // database)
  long gainTime;
  long leakageTime;
  long bandpassTime;

  // we need some kind of flag to describe the status of each stored parameter
  CalParamStatus gainStatus;
  CalParamStatus leakageStatus;
  CalParamStatus bandpassStatus;
};

// we could have sequence of sequences here to get one item per antenna/beam, but flattened sequence seems to be more appropriate.
// dictionary indexed with JonesIndex could also be used, but a fixed-size array would make the logic simpler (as there should be
// one structure for every antenna/beam anyway)
sequence<CalibrationParameters> CalibrationParametersSeq;

// collection of all calibration parameters applied at a given time
struct AppliedCalibration {
  // a flattened sequence is used (say beam varies first, but it can be changed if necessary), so it would be good
  // to have the numbers of antennas/beams here. However, these are not supposed to change at all. We can even have a full
  // blown 36-antenna case for BETA. 
  short nAntennas;
  short nBeams;
  // time stamp when the calibration is going to be applied or has been applied
  long time;
  // sequence number for this calibration (we probably could get away with just time field, but it may be easier to index
  // each update to the calibration to keep track of it). This number is incremented any time there is a change to at least
  // one of the calibration parameters
  CalibrationParametersSeq data;
};

}; // module calibparams

}; // module interfaces

}; // module askap

#endif // #ifndef ASKAP_CALIBRATION_PARAMETERS_ICE

