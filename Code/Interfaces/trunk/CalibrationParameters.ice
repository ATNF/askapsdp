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
DoubleComplex JonesDTerm;

// sequence of J-terms, i.e. a bandpass
sequence<JonesGTerm> JonesJTermSeq;
 
struct FrequencyDependentJTerm {
  int nchan;
  JonesGTermSeq bandpass;
  // some frequency vs. channel mapping
  // parameters can come here, e.g. freqID
  // but we don't need them for processing
  // However, it may be good to be able to apply last bandpass
  // determined for a particular frequency configuration
};

// gain solution indexed with JonesIndex
dictionary<JonesIndex,JonesJTerm> GainSolution;

// leakage solution indexed with JonesIndex
dictionary<JonesIndex,JonesDTerm> LeakageSolution;

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

}; // module calibparams

}; // module interfaces

}; // module askap

#endif // #ifndef ASKAP_CALIBRATION_PARAMETERS_ICE

