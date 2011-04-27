/// @file IceMapper.cc
///
/// @copyright (c) 2011 CSIRO
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
#include "IceMapper.h"

// Include package level header file
#include "askap_cpdataservices.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/aipstype.h"

// Local package includes
#include "calibrationclient/JonesJTerm.h"
#include "calibrationclient/JonesIndex.h"
#include "calibrationclient/GenericSolution.h"

// Using
using namespace askap;
using namespace askap::cp::caldataservice;

askap::interfaces::calparams::TimeTaggedGainSolution IceMapper::toIce(const askap::cp::caldataservice::GainSolution& sol)
{
    // Convert the casa type GainSolution to the ice GainSolution
    askap::interfaces::calparams::TimeTaggedGainSolution ice_sol;
    ice_sol.timestamp = sol.timestamp();

    typedef JonesIndex keyType;
    typedef JonesJTerm valueType;
    const std::map<keyType, valueType>& gains = sol.map();
    std::map<keyType, valueType>::const_iterator it;
    for (it = gains.begin(); it != gains.end(); ++it) {
        ice_sol.solutionMap[toIce(it->first)] = IceMapper::toIce(it->second);
    }

    // Post-conditions
    ASKAPCHECK(ice_sol.solutionMap.size() == sol.map().size(), "Map size mismatch");

    return ice_sol;
}

askap::interfaces::calparams::TimeTaggedLeakageSolution IceMapper::toIce(const askap::cp::caldataservice::LeakageSolution& sol)
{
    // Convert the casa type LeakageSolution to the ice LeakageSolution
    askap::interfaces::calparams::TimeTaggedLeakageSolution ice_sol;
    ice_sol.timestamp = sol.timestamp();

    typedef JonesIndex keyType;
    typedef casa::DComplex valueType;
    const std::map<keyType, valueType>& leakages = sol.map();
    std::map<keyType, valueType>::const_iterator it;
    for (it = leakages.begin(); it != leakages.end(); ++it) {
        ice_sol.solutionMap[toIce(it->first)] = IceMapper::toIce(it->second);
    }

    // Post-conditions
    ASKAPCHECK(ice_sol.solutionMap.size() == sol.map().size(), "Map size mismatch");

    return ice_sol;
}

askap::interfaces::calparams::TimeTaggedBandpassSolution IceMapper::toIce(const askap::cp::caldataservice::BandpassSolution& sol)
{
    // Convert the casa type BandpassSolution to the ice BandpassSolution
    askap::interfaces::calparams::TimeTaggedBandpassSolution ice_sol;
    ice_sol.timestamp = sol.timestamp();

    typedef JonesIndex keyType;
    typedef std::vector<JonesJTerm> valueType;
    const std::map< keyType, valueType >& bandpass = sol.map();
    std::map< keyType, valueType >::const_iterator it;
    for (it = bandpass.begin(); it != bandpass.end(); ++it) {
        // Handle the vector
        const valueType& terms = it->second;

        askap::interfaces::calparams::JonesJTermSeq ice_terms(terms.size());
        for (casa::uInt chan = 0; chan < terms.size(); ++chan) {
            ice_terms.push_back(IceMapper::toIce(terms[chan]));
        }

        ice_sol.solutionMap[toIce(it->first)] = ice_terms;
    }

    // Post-conditions
    ASKAPCHECK(ice_sol.solutionMap.size() == sol.map().size(), "Map size mismatch");

    return ice_sol;
}

askap::cp::caldataservice::GainSolution IceMapper::fromIce(const askap::interfaces::calparams::TimeTaggedGainSolution& ice_sol)
{
    // Convert the ice type GainSolution to the casa GainSolution
    askap::cp::caldataservice::GainSolution sol(ice_sol.timestamp);

    typedef askap::interfaces::calparams::JonesIndex keyType;
    typedef askap::interfaces::calparams::JonesJTerm valueType;
    const std::map<keyType, valueType> ice_gains = ice_sol.solutionMap;
    std::map<keyType, valueType>::const_iterator it;
    for (it = ice_gains.begin(); it != ice_gains.end(); ++it) {
        sol.map()[fromIce(it->first)] = fromIce(it->second);
    }

    // Post-conditions
    ASKAPCHECK(ice_sol.solutionMap.size() == sol.map().size(), "Map size mismatch");

    return sol;
}

askap::cp::caldataservice::LeakageSolution IceMapper::fromIce(const askap::interfaces::calparams::TimeTaggedLeakageSolution& ice_sol)
{
    // Convert the ice type LeakageSolution to the casa LeakageSolution
    askap::cp::caldataservice::LeakageSolution sol(ice_sol.timestamp);

    typedef askap::interfaces::calparams::JonesIndex keyType;
    typedef askap::interfaces::DoubleComplex valueType;
    const std::map<keyType, valueType> ice_leakages = ice_sol.solutionMap;
    std::map<keyType, valueType>::const_iterator it;
    for (it = ice_leakages.begin(); it != ice_leakages.end(); ++it) {
        sol.map()[fromIce(it->first)] = fromIce(it->second);
    }

    // Post-conditions
    ASKAPCHECK(ice_sol.solutionMap.size() == sol.map().size(), "Map size mismatch");

    return sol;
}

askap::cp::caldataservice::BandpassSolution IceMapper::fromIce(const askap::interfaces::calparams::TimeTaggedBandpassSolution& ice_sol)
{
    // Convert the ice type BandpassSolution to the casa BandpassSolution
    askap::cp::caldataservice::BandpassSolution sol(ice_sol.timestamp);

    typedef askap::interfaces::calparams::JonesIndex keyType;
    typedef askap::interfaces::calparams::JonesJTermSeq valueType;
    const std::map<keyType, valueType> ice_bandpass = ice_sol.solutionMap;
    std::map<keyType, valueType>::const_iterator it;
    for (it = ice_bandpass.begin(); it != ice_bandpass.end(); ++it) {
        // Handle the vector
        const valueType& ice_terms = it->second;

        std::vector<JonesJTerm> terms(ice_terms.size());
        for (casa::uInt chan = 0; chan < terms.size(); ++chan) {
            terms.push_back(IceMapper::fromIce(ice_terms[chan]));
        }

        sol.map()[fromIce(it->first)] = terms;
    }

    return sol;
}

///////////////////////////////////////////////
// Private
///////////////////////////////////////////////

askap::interfaces::DoubleComplex IceMapper::toIce(const casa::DComplex& val)
{
    askap::interfaces::DoubleComplex ice_val;;
    ice_val.real = val.real();
    ice_val.imag = val.imag();
    return ice_val;
}

casa::DComplex IceMapper::fromIce(const askap::interfaces::DoubleComplex& ice_val)
{
    return casa::DComplex(ice_val.real, ice_val.imag);
}

askap::interfaces::calparams::JonesIndex IceMapper::toIce(const askap::cp::caldataservice::JonesIndex& jindex)
{
    askap::interfaces::calparams::JonesIndex ice_jindex;
    ice_jindex.antennaID = jindex.antenna();
    ice_jindex.beamID = jindex.beam();
    return ice_jindex;
}

askap::cp::caldataservice::JonesIndex IceMapper::fromIce(const askap::interfaces::calparams::JonesIndex& ice_jindex)
{
    return askap::cp::caldataservice::JonesIndex(ice_jindex.antennaID, ice_jindex.beamID);
}

askap::interfaces::calparams::JonesJTerm IceMapper::toIce(const askap::cp::caldataservice::JonesJTerm& jterm)
{
    askap::interfaces::calparams::JonesJTerm ice_jterm;
    ice_jterm.g1 = toIce(jterm.g1());
    ice_jterm.g1Valid = jterm.g1IsValid();
    ice_jterm.g2 = toIce(jterm.g2());
    ice_jterm.g2Valid = jterm.g2IsValid();
    return ice_jterm;
}

askap::cp::caldataservice::JonesJTerm IceMapper::fromIce(const askap::interfaces::calparams::JonesJTerm& ice_jterm)
{
    return askap::cp::caldataservice::JonesJTerm(fromIce(ice_jterm.g1), ice_jterm.g1Valid,
                                                 fromIce(ice_jterm.g2), ice_jterm.g2Valid);
}
