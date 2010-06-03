// @file SoftwareInstrumentService.ice
//
// @copyright (c) 2010 CSIRO
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

#ifndef ASKAP_SOFTWAREINSTRUMENTSERVICE_ICE
#define ASKAP_SOFTWAREINSTRUMENTSERVICE_ICE

#include <CommonTypes.ice>
#include <DataServiceExceptions.ice>

module askap
{

module interfaces
{

module schedblock
{

    /**
     * The Version to associate with an instance of the SI.
     **/
    struct Version
    {
        int major;
        int minor;
    };


    /** The psoobile status a SI can have. This tags tth SI for the various
     * different users/use cases.
     **/
    enum SIStatus
    {
        PUBLIC,
        ENGINEERING,
        DISABLED
    };


    /**
     * The interface to create/access and maintain Software Instruments (SI).
     **/
    interface ISoftwareInstrumentService
    {

        /**
         * Create a new Science Instrument with the given parameters
         *
         * @param name The SI name
         * @param template The ObsParameterTemplate describing the SI
         * @param obsproc The ObsProcedure script
         * @param status The SI Status
         * @param version The version number to set
         *
         **/
        long create(string name,
                    askap::interfaces::ParameterMap template,
                    string obsproc, SIStatus status, Version version);

        /**
         * Make an identical copy and return th new id
         **/
        long clone(long ssid)
            throws NoSuchSoftwareInstrumentException;

        /**
         * Modify the Status of the specified SI
         **/
        void setStatus(long siid, SIStatus status)
            throws NoSuchSoftwareInstrumentException;

        /**
         * Modify the name of the specified SI
         **/
        void setName(long siid, string name)
            throws NoSuchSoftwareInstrumentException;

        /**
         * Set the version of the given SI
         **/
        void setVersion(long siid, Version version)
            throws NoSuchSoftwareInstrumentException;


        /**
         * Update the SI's template and procedure. It is required to specify
         * a new version as well.
         **/
        void setTemplate(long siid, Version version,
                         askap::interfaces::ParameterMap template,
                         string obsproc);

        /**
         * Get a sequence of all available Software Instrument names.
         **/
        StringSeq getNames();

        /**
         * Get the Software Instrument names filtering by SI Status
         * version.
         **/
        StringSeq getByStatus(SIStatus status);

        /**
         * Get the Software Instrument id for a named SI with an optional
         * version.
         * uses the  default version if version empty
         **/
        long getByName(string name, Version version);


        /**
         * Retrieve the parameters makign up the template
         **/
        askap::interfaces::ParameterMap getObsParamTemplate(long ssid)
            throws NoSuchSoftwareInstrumentException;

        /**
         * Get the script associated with this Science Instrument
         **/
        string getObsProcedure(long ssid)
            throws NoSuchSoftwareInstrumentException;

        /**
         * Get the name of the specified SI
         **/
        string getName(long siid) throws NoSuchSoftwareInstrumentException;

        /**
         * Get the version of the specified SI
         **/
        Version getVersion(long siid) throws NoSuchSoftwareInstrumentException;

    };

};
};
};

#endif
