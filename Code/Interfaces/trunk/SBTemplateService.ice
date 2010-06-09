// @file SBTemplateService.ice
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

#ifndef ASKAP_SBTEMPLATESERVICE_ICE
#define ASKAP_SBTEMPLATESERVICE_ICE

#include <CommonTypes.ice>
#include <DataServiceExceptions.ice>

module askap
{

module interfaces
{

module schedblock
{

    /**
     * This exception is thrown when a version incompatability has been
     * encountered. The base class includes a "reason" data member (of type
     * string) which shall be used to indicate why the state transition failed.
     **/
    exception VersionException extends askap::interfaces::AskapIceException
    {
    };


    /**
     * The Version to associate with an instance of the SBTemplate.
     **/
    struct Version
    {
        int major;
        int minor;
    };


    /**
     * The possible status a SBTemplate can have. This tags the SBTemplate
     * for the various different users/use cases.
     **/
    enum SBTemplateStatus
    {
        PUBLIC,
        ENGINEERING,
        DISABLED
    };


    /**
     * The interface to create/access and maintain Scheduling Block Templates.
     * These describe the Observation Parameters and Procedure which are
     * versioned for tracebility. These templates have a name associated with
     * them. When accessing the interface by name the latest version of the
     * SBTemplate is returned. Template and procedure can't be separated as the
     * template describes the parameters used in the prtocedure.
     *
     **/
    interface ISBTemplateService
    {

        /**
         * Create a new SB Template with the given parameters.
         *
         * @param name The SBTemplate name
         * @param template The ObsParameterTemplate describing the SBTemplate
         * @param obsproc The ObsProcedure script
         * @param status The SBTemplate Status
         * @param vers The version number to set
         *
         **/
        long create(string name,
                    askap::interfaces::ParameterMap template,
                    string obsproc, SBTemplateStatus status, Version vers)
            throws VersionException;

        /**
         * Modify the Status of the specified SBTemplate.
         *
         * @param sbtid the id of the template to access
         * @param status the new SBTemplateStatus
         *
         **/
        void setStatus(long sbtid, SBTemplateStatus status)
            throws NoSuchSBTemplateException;

        /**
         * Modify the name of the specified SBTemplate.
         *
         * @param sbtid the id of the template to access
         * @param name the new name of the template
         *
         **/
        void setName(long sbtid, string name)
            throws NoSuchSBTemplateException;

        /**
         * Set the version of the given SBTemplate.
         *
         * @param sbtid the id of the template to access
         *
         **/
        void setVersion(long sbtid, Version vers)
            throws NoSuchSBTemplateException,
                   VersionException;

        /**
         * Update the SBTemplate's template and procedure. It is required to
         * specify a new version as well.
         *
         * @param sbtid the id of the template to access
         * @param vers The version number to set
         * @param template The ObsParameterTemplate describing the SBTemplate
         * @param obsproc The ObsProcedure script
         *
         **/
        void setTemplate(long sbtid, Version vers,
                         askap::interfaces::ParameterMap template,
                         string obsproc)
            throws NoSuchSBTemplateException,
                   VersionException;

        /**
         * Get a sequence of all available SB Template names.
         **/
        StringSeq getNames();

        /**
         * Get the SB Template names filtering by SBTemplateStatus.
         *
         * @param status the SBTemplateStatus to filter by
         *
         **/
        StringSeq getByStatus(SBTemplateStatus status);

        /**
         * Get the SB Template id for a named SBTemplate at the latest version.
         *
         * @param name the name of the template
         *
         **/
        long getByName(string name) throws NoSuchSBTemplateException;

        /**
         * Get the SB Template id for a named SBTemplate the specified version.

         * @param name the name of the template
         * @param vers The version number to retrieve
         *
         **/
        long getByVersion(string name, Version vers)
            throws NoSuchSBTemplateException,
                    VersionException;

        /**
         * Retrieve the parameters making up the template
         **/
        askap::interfaces::ParameterMap getObsParamTemplate(long sbtid)
            throws NoSuchSBTemplateException;

        /**
         * Get the script associated with this template.
         *
         * @param sbtid The id of the template to access
         *
         **/
        string getObsProcedure(long sbtid)
            throws NoSuchSBTemplateException;

        /**
         * Get the name of the specified SBTemplate.
         *
         * @param sbtid the id of the template to access
         *
         **/
        string getName(long sbtid) throws NoSuchSBTemplateException;

        /**
         * Get the version of the specified SBTemplate.
         *
         * @param sbtid the id of the template to access
         *
         **/
        Version getVersion(long sbtid) throws NoSuchSBTemplateException;

    };

};
};
};

#endif
