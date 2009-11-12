// @file MoniCA.ice
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

#ifndef ASKAP_MONICA_ICE
#define ASKAP_MONICA_ICE

module atnf {
  module atoms {
    module mon {
      module comms {
        ////////////
        //Data structure definitions
        sequence<string> stringarray;

        /**
         * PointDescriptionIce contains pickled fields which fully describe
         * a specific point
         **/
        struct PointDescriptionIce {
          stringarray names;
          string      source;
          string      description;
          string      shortdescription;
          string      units;
          bool        enabled;
          stringarray inputtransactions;
          stringarray outputtransactions;
          stringarray translations;
          stringarray limits;
          stringarray archivepolicies;
          long        period;
          int         archivelongevity;
        };
        sequence<PointDescriptionIce> pointarray;

        /** Enum to represent data type **/
        enum DataType {DTNull, DTFloat, DTDouble, DTInt, DTLong, DTString, DTBoolean, DTAbsTime, DTRelTime, DTAngle};

        /** Classes to hold different data types **/
        class DataValue { 
          DataType type;
        };
        class DataValueFloat extends DataValue {
          float value;
        };        
        class DataValueDouble extends DataValue {
          double value;
        };
        class DataValueInt extends DataValue {
          int value;
        };        
        class DataValueLong extends DataValue {
          long value;
        };
        class DataValueString extends DataValue {
          string value;
        };
        class DataValueBoolean extends DataValue {
          bool value;
        };
        class DataValueRelTime extends DataValue {
          long value;
        };
        class DataValueAbsTime extends DataValue {
          long value;
        };
        class DataValueAngle extends DataValue {
          double value;
        };
        
        /** PointDataIce encapsulates a single record/datum **/
        struct PointDataIce {
          string    name;
          long      timestamp;
          DataValue value;
          bool      alarm;
        };
        sequence<PointDataIce> pointdataset;
        sequence<pointdataset> pointdatasetarray;
        
        
       
        /** 
         * The main interface between clients and the server
         **/
        interface MoniCAIce {
          ////////////
          //Operations for getting/setting point metadata
          ////////////

          /** Return the names of all points on the system **/
          idempotent stringarray getAllPointNames();

          /** Return full details for the specified points **/
          idempotent pointarray getPoints(stringarray names);

          /** Return full details for all points on the system **/
          idempotent pointarray getAllPoints();

          /** Add/update the definitions for the specified points **/
          bool addPoints(pointarray newpoints, string username, string passwd);
          
          ////////////
          //Operations relating to getting/setting data
          ///////////

          /**
           * Return historical data for the given points
           * Set maxsamples to zero to impose no limit
           **/
          idempotent pointdatasetarray getArchiveData(stringarray names, long start, long end, long maxsamples);

          /** Get latest data for the given points **/
          idempotent pointdataset getData(stringarray names);

          /** Set new values for the given points. **/
          bool setData(stringarray names, pointdataset rawvalues, string username, string passwd);

          ////////////
          //Operations relating to 'SavedSetups'. These are basically pickled
          //dictionaries which are currently used to contain predefined page
          //layouts for the GUI client (but which may have other uses in the 
          //future).
          ////////////

          /** Get the list of all saved page setups from the server **/
          idempotent stringarray getAllSetups();

          /** Add/update a new setup to the server's list **/
          bool addSetup(string setup, string username, string passwd);
          
          ////////////
          //Some miscellaneous operations
          ////////////

          /** Obtain public key and modulus to use for authenticated operations **/
          idempotent stringarray getEncryptionInfo();

          /** Get current time from server **/
          idempotent long getCurrentTime();
        };
      };
    };
  };
};

#endif
