/// @file TypedValueMapConstMapper.cc
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

// Include own header file first
#include "TypedValueMapConstMapper.h"

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "casa/aips.h"

#include "measures/Measures/MDirection.h"

// CP Ice interfaces
#include "TypedValues.h"

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::interfaces;

TypedValueMapConstMapper::TypedValueMapConstMapper(const TypedValueMap& map) :
    itsConstMap(map)
{
}

int TypedValueMapConstMapper::getInt(const std::string& key) const
{
    return get<int, TypeInt, TypedValueIntPtr>(key);
}

long TypedValueMapConstMapper::getLong(const std::string& key) const
{
    // ::Ice::Long is 64-bit (even on 32-bit x86) whereas casa::Long will be 
    // 32-bit. Using this mapper on such a system will likely lead to grief.
#ifndef __LP64__
    ASKAPTHROW(AskapError, "This platform does not support 64-bit long");
#else
    return get<long, TypeLong, TypedValueLongPtr>(key);
#endif
}

casa::String TypedValueMapConstMapper::getString(const std::string& key) const
{
    return get<casa::String, TypeString, TypedValueStringPtr>(key);
}

casa::Bool TypedValueMapConstMapper::getBool(const std::string& key) const
{
    return get<casa::Bool, TypeBool, TypedValueBoolPtr>(key);
}

casa::Float TypedValueMapConstMapper::getFloat(const std::string& key) const
{
    return get<casa::Float, TypeFloat, TypedValueFloatPtr>(key);
}

casa::Double TypedValueMapConstMapper::getDouble(const std::string& key) const
{
    return get<casa::Double, TypeDouble, TypedValueDoublePtr>(key);
}

casa::Complex TypedValueMapConstMapper::getFloatComplex(const std::string& key) const
{
    askap::interfaces::FloatComplex val =
        get<askap::interfaces::FloatComplex, TypeFloatComplex,
        TypedValueFloatComplexPtr>(key);

    return casa::Complex(val.real, val.imag);
}

casa::DComplex TypedValueMapConstMapper::getDoubleComplex(const std::string& key) const
{
    askap::interfaces::FloatComplex val =
        get<askap::interfaces::FloatComplex, TypeFloatComplex,
        TypedValueFloatComplexPtr>(key);

    return casa::DComplex(val.real, val.imag);
}

casa::MDirection TypedValueMapConstMapper::getDirection(const std::string& key) const
{
    askap::interfaces::Direction val =
        get<askap::interfaces::Direction, TypeDirection,
        TypedValueDirectionPtr>(key);

    return convertDirection(val);
}

std::vector<casa::Int> TypedValueMapConstMapper::getIntSeq(const std::string& key) const
{
    return get<std::vector<casa::Int>, TypeIntSeq, TypedValueIntSeqPtr>(key);
}

std::vector<casa::Long> TypedValueMapConstMapper::getLongSeq(const std::string& key) const
{
    // ::Ice::Long is 64-bit (even on 32-bit x86) whereas casa::Long will be 
    // 32-bit. Using this mapper on such a system will likely lead to grief.
#ifndef __LP64__
    ASKAPTHROW(AskapError, "This platform does not support 64-bit long");
#else
    //return get<std::vector<casa::Long>, TypeLongSeq, TypedValueLongSeqPtr>(key);
    LongSeq seq = get<LongSeq, TypeLongSeq, TypedValueLongSeqPtr>(key);
    return std::vector<casa::Long>(seq.begin(), seq.end());
#endif
}

std::vector<casa::String> TypedValueMapConstMapper::getStringSeq(const std::string& key) const
{
    askap::interfaces::StringSeq val =
        get<askap::interfaces::StringSeq, TypeStringSeq,
        TypedValueStringSeqPtr>(key);

    return std::vector<casa::String>(val.begin(), val.end());
}

std::vector<casa::Bool> TypedValueMapConstMapper::getBoolSeq(const std::string& key) const
{
    return get<std::vector<casa::Bool>, TypeBoolSeq, TypedValueBoolSeqPtr>(key);
}

std::vector<casa::Float> TypedValueMapConstMapper::getFloatSeq(const std::string& key) const
{
    return get<std::vector<casa::Float>, TypeFloatSeq, TypedValueFloatSeqPtr>(key);
}

std::vector<casa::Double> TypedValueMapConstMapper::getDoubleSeq(const std::string& key) const
{
    return get<std::vector<casa::Double>, TypeDoubleSeq, TypedValueDoubleSeqPtr>(key);
}

std::vector<casa::Complex> TypedValueMapConstMapper::getFloatComplexSeq(const std::string& key) const
{
    askap::interfaces::FloatComplexSeq val =
        get<askap::interfaces::FloatComplexSeq, TypeFloatComplexSeq,
        TypedValueFloatComplexSeqPtr>(key);

    // Populate this vector before returning it
    std::vector<casa::Complex> container;

    askap::interfaces::FloatComplexSeq::const_iterator it = val.begin();
    for (it = val.begin(); it != val.end(); ++it) {
        container.push_back(casa::Complex(it->real, it->imag));
    }

    return container;
}

std::vector<casa::DComplex> TypedValueMapConstMapper::getDoubleComplexSeq(const std::string& key) const
{
    askap::interfaces::DoubleComplexSeq val =
        get<askap::interfaces::DoubleComplexSeq, TypeDoubleComplexSeq,
        TypedValueDoubleComplexSeqPtr>(key);

    // Populate this vector before returning it
    std::vector<casa::DComplex> container;

    askap::interfaces::DoubleComplexSeq::const_iterator it = val.begin();
    for (it = val.begin(); it != val.end(); ++it) {
        container.push_back(casa::DComplex(it->real, it->imag));
    }

    return container;
}

std::vector<casa::MDirection> TypedValueMapConstMapper::getDirectionSeq(const std::string& key) const
{
    askap::interfaces::DirectionSeq val =
        get<askap::interfaces::DirectionSeq, TypeDirectionSeq,
        TypedValueDirectionSeqPtr>(key);

    // Populate this vector before returning it
    std::vector<casa::MDirection> container;

    askap::interfaces::DirectionSeq::const_iterator it = val.begin();
    for (it = val.begin(); it != val.end(); ++it) {
        container.push_back(convertDirection(*it));
    }

    return container;
}

template <class T, askap::interfaces::TypedValueType TVType, class TVPtr>
T TypedValueMapConstMapper::get(const std::string& key) const
{
    if (itsConstMap.count(key) == 0) {
        ASKAPTHROW(AskapError, "Specified key (" << key << ") does not exist");
    }
    const TypedValuePtr tv = itsConstMap.find(key)->second;
    if (tv->type != TVType) {
        ASKAPTHROW(AskapError, "Specified key (" << key << ") not of type Int");
    }

    return TVPtr::dynamicCast(tv)->value;
}

casa::MDirection TypedValueMapConstMapper::convertDirection(const askap::interfaces::Direction& dir) const
{
    switch (dir.sys) {
        case J2000 :
            return casa::MDirection(casa::Quantity(dir.coord1, "rad"),
                    casa::Quantity(dir.coord2, "rad"),
                    casa::MDirection::Ref(casa::MDirection::J2000));
            break;
        case AZEL :
            return casa::MDirection(casa::Quantity(dir.coord1, "rad"),
                    casa::Quantity(dir.coord2, "rad"),
                    casa::MDirection::Ref(casa::MDirection::AZEL));
            break;
    }

    // This is the default case
   ASKAPTHROW(AskapError, "Coordinate system not supported");
}
