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
     * This enum described the possible version number fields. These will be
     * used when specifing which version to increment.
     **/
    enum VersionType
    {
        MAJOR,
        MINOR
    };


    /**
     * The Version to associate with an instance of the SBTemplate.
     * Use Version.major = 0, Version.minor = 0 for an unversioned request
     **/
    struct Version
    {
        int major;
        int minor;
    };


    /**
     * The possible status a SBTemplate can have. It is used to tag the
     * SBTemplate for the various users/use cases.
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
     * template describes the parameters used in the procedure.
     * <p>
     * Version numbering is done in the service. The consumer has to specify
     * which part of the version (major or minor) they want to increment.
     *
     **/
    interface ISBTemplateService
    {

        /**
         * Create a new SBTemplate with the given parameters. It throws a
         * VersionException if the template name exists already. The new
         * template will be versioned 0.1.
         *
         * @param name The SBTemplate name
         * @param template The ObsParameterTemplate describing the SBTemplate
         * @param obsproc The ObsProcedure script
         * @param status The SBTemplate Status
         *
         **/
        long create(string name,
                    askap::interfaces::ParameterMap template,
                    string obsproc, SBTemplateStatus status)
            throws VersionException;

        /**
         * get the SBTemplate id for the latest version of the named template
         **/
        idempotent long getLatestVersionId(string name);

        /**
         * Modify the Status of the specified SBTemplate.
         *
         * @param name the name of the template to access
         * @param status the new SBTemplateStatus
         *
         **/
        void setStatus(string name, SBTemplateStatus status)
            throws NoSuchSBTemplateException;

        /**
         * Get the Status of the specified SBTemplate.
         *
         * @param name the name of the template to access
         * @return status the new SBTemplateStatus
         *
         **/
        SBTemplateStatus getStatus(string name)
            throws NoSuchSBTemplateException;

        /**
         * Update the SBTemplate's template and procedure. It is required to
         * specify what version (MAJOR ot MINOR) should be incremented.
         * This returns a NEW template id!
         *
         * @param name the name of the template to access
         * @param vtype The VersionType to bump
         * @param template The ObsParameterTemplate describing the SBTemplate
         * @param obsproc The ObsProcedure script
         * @returns a new template id
         *
         **/
        long updateTemplate(string name, VersionType vtype,
                            askap::interfaces::ParameterMap template,
                            string obsproc)
            throws NoSuchSBTemplateException,
                   VersionException;

        /**
         * Get a sequence of all available SBTemplate names.
         **/
        idempotent askap::interfaces::StringSeq getAll();

        /**
         * Get the SBTemplate names filtering by SBTemplateStatus.
         *
         * @param status the SBTemplateStatus to filter by
         *
         **/
        idempotent askap::interfaces::StringSeq getByStatus(SBTemplateStatus status);

        /**
         * Get the SBTemplate ids for a named SBTemplate in descending order of
         * version numbers, latest first.
         *
         * @param name the name of the template
         *
         **/
        idempotent askap::interfaces::LongSeq getByName(string name)
            throws NoSuchSBTemplateException;

        /**
         * Return the SBTemplate id for a named template at the given version.
         *
         * @param name the name of the template
         * @param vers The version of the template instance
         *
         **/
        idempotent long getByVersion(string name, Version vers)
            throws NoSuchSBTemplateException,
                    VersionException;

        /**
         * Retrieve the parameters making up the template
         **/
        idempotent askap::interfaces::ParameterMap getObsParamTemplate(long sbtid)
            throws NoSuchSBTemplateException;

        /**
         * Get the script associated with this template.
         *
         * @param name
         * @param version
         *
         **/
        idempotent string getObsProcedure(long sbtid)
            throws NoSuchSBTemplateException;

        /**
         * Get the SBTemplate name for specified SBTemplate instance
         *
         * @param name the name of the template to access
         *
         **/
        idempotent string getName(long sbtid) throws NoSuchSBTemplateException;

        /**
         * Get the version of the specified SBTemplate.
         *
         * @param sbtid the id of the template to access
         *
         **/
        idempotent Version getVersion(long sbtid)
            throws NoSuchSBTemplateException;

    };

};
};
};

#endif
