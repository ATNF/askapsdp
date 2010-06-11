/// @file TypedValueMapMapper.cc
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
#include "TypedValueMapMapper.h"

// System includes
#include <vector>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "measures/Measures/MDirection.h"

// CP Ice interfaces
#include "TypedValues.h"

// Local package includes
#include "TypedValueMapConstMapper.h"

// Using
using namespace askap;
using namespace askap::cp;
using namespace askap::interfaces;

TypedValueMapMapper::TypedValueMapMapper(TypedValueMap& map) :
    TypedValueMapConstMapper(map), itsMap(map)
{
}

void TypedValueMapMapper::setInt(const std::string& key, const casa::Int& val)
{
    set<casa::Int, TypeInt, TypedValueInt>(key, val);
}

void TypedValueMapMapper::setLong(const std::string& key, const casa::Long& val)
{
    // ::Ice::Long is 64-bit (even on 32-bit x86) whereas casa::Long will be 
    // 32-bit. Using this mapper on such a system will likely lead to grief.
#ifndef __LP64__
    ASKAPTHROW(AskapError, "This platform does not support 64-bit long");
#else
    set<casa::Long, TypeLong, TypedValueLong>(key, val);
#endif
}

void TypedValueMapMapper::setString(const std::string& key, const casa::String& val)
{
    set<casa::String, TypeString, TypedValueString>(key, val);
}

void TypedValueMapMapper::setBool(const std::string& key, const casa::Bool& val)
{
    set<casa::Bool, TypeBool, TypedValueBool>(key, val);
}

void TypedValueMapMapper::setFloat(const std::string& key, const casa::Float& val)
{
    set<casa::Float, TypeFloat, TypedValueFloat>(key, val);
}

void TypedValueMapMapper::setDouble(const std::string& key, const casa::Double& val)
{
    set<casa::Double, TypeDouble, TypedValueDouble>(key, val);
}

void TypedValueMapMapper::setFloatComplex(const std::string& key, const casa::Complex& val)
{
    askap::interfaces::FloatComplex obj;
    obj.real = val.real();
    obj.imag = val.imag();
    set<FloatComplex, TypeFloatComplex, TypedValueFloatComplex>(key, obj);
}

void TypedValueMapMapper::setDoubleComplex(const std::string& key, const casa::DComplex& val)
{
    askap::interfaces::DoubleComplex obj;
    obj.real = val.real();
    obj.imag = val.imag();
    set<DoubleComplex, TypeDoubleComplex, TypedValueDoubleComplex>(key, obj);
}

void TypedValueMapMapper::setDirection(const std::string& key, const casa::MDirection& val)
{
    askap::interfaces::Direction obj = convertDirection(val);
    set<Direction, TypeDirection, TypedValueDirection>(key, obj);
}


void TypedValueMapMapper::setIntSeq(const std::string& key, const std::vector<casa::Int>& val)
{
    set<std::vector<casa::Int>, TypeIntSeq, TypedValueIntSeq>(key, val);
}

void TypedValueMapMapper::setLongSeq(const std::string& key, const std::vector<casa::Long>& val)
{
    // ::Ice::Long is 64-bit (even on 32-bit x86) whereas casa::Long will be 
    // 32-bit. Using this mapper on such a system will likely lead to grief.
#ifndef __LP64__
    ASKAPTHROW(AskapError, "This platform does not support 64-bit long");
#else
    // Use assign on this sequence instead of calling set directly and having
    // the copy constructor do the conversion. This was a problem on Mac OSX 10.6
    LongSeq seq(val.begin(), val.end());
    set<LongSeq, TypeLongSeq, TypedValueLongSeq>(key, seq);
#endif
}

void TypedValueMapMapper::setStringSeq(const std::string& key, const std::vector<casa::String>& val)
{
    StringSeq seq(val.begin(), val.end());
    set<StringSeq, TypeStringSeq, TypedValueStringSeq>(key, seq);
}

void TypedValueMapMapper::setBoolSeq(const std::string& key, const std::vector<casa::Bool>& val)
{
    set<std::vector<casa::Bool>, TypeBoolSeq, TypedValueBoolSeq>(key, val);
}

void TypedValueMapMapper::setFloatSeq(const std::string& key, const std::vector<casa::Float>& val)
{
    set<std::vector<casa::Float>, TypeFloatSeq, TypedValueFloatSeq>(key, val);
}

void TypedValueMapMapper::setDoubleSeq(const std::string& key, const std::vector<casa::Double>& val)
{
    set<std::vector<casa::Double>, TypeDoubleSeq, TypedValueDoubleSeq>(key, val);
}

void TypedValueMapMapper::setFloatComplexSeq(const std::string& key, const std::vector<casa::Complex>& val)
{
    askap::interfaces::FloatComplexSeq seq;
    std::vector<casa::Complex>::const_iterator it;
    for (it = val.begin(); it != val.end(); ++it) {
        askap::interfaces::FloatComplex obj;
        obj.real = it->real();
        obj.imag = it->imag();
        seq.push_back(obj);
    }
    set<FloatComplexSeq, TypeFloatComplexSeq, TypedValueFloatComplexSeq>(key, seq);
}

void TypedValueMapMapper::setDoubleComplexSeq(const std::string& key, const std::vector<casa::DComplex>& val)
{
    askap::interfaces::DoubleComplexSeq seq;
    std::vector<casa::DComplex>::const_iterator it;
    for (it = val.begin(); it != val.end(); ++it) {
        askap::interfaces::DoubleComplex obj;
        obj.real = it->real();
        obj.imag = it->imag();
        seq.push_back(obj);
    }
    set<DoubleComplexSeq, TypeDoubleComplexSeq, TypedValueDoubleComplexSeq>(key, seq);
}

void TypedValueMapMapper::setDirectionSeq(const std::string& key, const std::vector<casa::MDirection>& val)
{
    DirectionSeq seq;
    std::vector<casa::MDirection>::const_iterator it;
    for (it = val.begin(); it != val.end(); ++it) {
        askap::interfaces::Direction obj = convertDirection(*it);
        seq.push_back(obj);
    }
    set<DirectionSeq, TypeDirectionSeq, TypedValueDirectionSeq>(key, seq);
}

template <class T, askap::interfaces::TypedValueType TVType, class TVClass>
void TypedValueMapMapper::set(const std::string& key, const T& val)
{
    itsMap[key] = new TVClass(TVType, val);
}


askap::interfaces::Direction TypedValueMapMapper::convertDirection(const casa::MDirection& dir) const
{
    askap::interfaces::Direction obj;
    obj.coord1 = dir.getAngle().getValue()(0);
    obj.coord2 = dir.getAngle().getValue()(1);

    switch (dir.getRef().getType()) {
        case casa::MDirection::J2000 :
            obj.sys = J2000;
            break;
        case casa::MDirection::AZEL :
            obj.sys = AZEL;
            break;
        default:
            ASKAPTHROW(AskapError, "Coordinate system not supported");

    }
    return obj;
}
