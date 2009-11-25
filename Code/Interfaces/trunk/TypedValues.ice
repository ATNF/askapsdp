// @file TypedValues.ice
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

#ifndef ASKAP_TYPEDVALUES_ICE
#define ASKAP_TYPEDVALUES_ICE

module askap
{

module interfaces
{
    /**
     * Enum to represent data types.
     **/
    enum TypedValueType {TypeNull, TypeFloat, TypeDouble, TypeInt, TypeLong, TypeString, TypeBoolean, TypeComplex};
    
    /**
     * Base class for typed data types.
     **/
    class TypedValue { 
        TypedValueType type;
    };
    
    /**
     * Class for a float type.
     **/
    class TypedValueFloat extends TypedValue {
        float value;
    };
    
    /**
     * Class for a double precision float type.
     **/
    class TypedValueDouble extends TypedValue {
        double value;
    };
    
    /**
     * Class for an integer type.
     **/ 
    class TypedValueInt extends TypedValue {
        int value;
    };
    
    /**
     * Class for a long integer type.
     **/
    class TypedValueLong extends TypedValue {
        long value;
    };
    
    /**
     * Class for a string type.
     **/
    class TypedValueString extends TypedValue {
        string value;
    };
    
    /**
     * Class for a boolean type.
     **/
    class TypedValueBoolean extends TypedValue {
        bool value;
    };

    /**
     * Class for a single precision floating point complex number.
     **/
    class TypedValueFloatComplex extends TypedValue {
        float real;
        float imag;
    };

    /**
     * Class for a double precision floating point complex number.
     **/
    class TypedValueComplex extends TypedValue {
        double real;
        double imag;
    };
    
    /**
     * Dictionary of typed data.
     **/
    dictionary <string,TypedValue> TypedValueMap;
    
    /**
     * Time-tagged dictionary of typed data.
     *
     * Timestamp is a Binary Atomic Time representing microseconds since
     * Modified Julian Date zero.
     */
    struct TimeTaggedTypedValueMap {
        long timestamp;
        TypedValueMap data;
    };
    
    module datapublisher
    {
        /**
         * Interface for a publisher of named typed values.
         **/
        interface ITypedValueMapPublisher {
            /**
             * Publish a new map of named typed values.
             **/
             void publish(TypedValueMap values);
        };


        /**
         * Interface for a publisher of time-tagged named typed values.
         **/
        interface ITimeTaggedTypedValueMapPublisher {
            /**
             * Publish a new map of named typed values.
             **/
             void publish(TimeTaggedTypedValueMap values);
        };
    };
};
};

#endif
