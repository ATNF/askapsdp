/// @file TypedValueMapMapper.h
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

#ifndef ASKAP_CP_TYPEDVALUEMAPMAPPER_H
#define ASKAP_CP_TYPEDVALUEMAPMAPPER_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "casa/aips.h"
#include "measures/Measures/MDirection.h"

// CP ice interfaces
#include "TypedValues.h"

namespace askap {
namespace cp {

class TypedValueMapMapper {
    public:
        TypedValueMapMapper(const askap::interfaces::TypedValueMap& map);
        
        casa::Int getInt(const std::string& key) const;
        casa::Long getLong(const std::string& key) const;
        casa::String getString(const std::string& key) const;
        casa::Bool getBool(const std::string& key) const;
        casa::Float getFloat(const std::string& key) const;
        casa::Double getDouble(const std::string& key) const;
        casa::Complex getFloatComplex(const std::string& key) const;
        casa::DComplex getDoubleComplex(const std::string& key) const;
        casa::MDirection getDirection(const std::string& key) const;

        std::vector<casa::Int> getIntSeq(const std::string& key) const;
        std::vector<casa::Long> getLongSeq(const std::string& key) const;
        std::vector<casa::String> getStringSeq(const std::string& key) const;
        std::vector<casa::Bool> getBoolSeq(const std::string& key) const;
        std::vector<casa::Float> getFloatSeq(const std::string& key) const;
        std::vector<casa::Double> getDoubleSeq(const std::string& key) const;
        std::vector<casa::Complex> getFloatComplexSeq(const std::string& key) const;
        std::vector<casa::DComplex> getDoubleComplexSeq(const std::string& key) const;
        std::vector<casa::MDirection> getDirectionSeq(const std::string& key) const;

    private:
        // Methods
        template <class T, askap::interfaces::TypedValueType E, class P>
        T get(const std::string& key) const;

        casa::MDirection convertDirection(const askap::interfaces::Direction& dir) const;

        // Attributes
        const askap::interfaces::TypedValueMap itsMap;
};

};
};

#endif
