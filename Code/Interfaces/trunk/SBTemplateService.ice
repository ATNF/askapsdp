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
     * The Version to associate with an instance of the SBTemplate.
     **/
    struct Version
    {
        int major;
        int minor;
    };


    /** 
     * The possible status a SBTemplate can have. This tags tth SBTemplate 
     * for the various different users/use cases.
     **/
    enum SBTemplateStatus
    {
        PUBLIC,
        ENGINEERING,
        DISABLED
    };


    /**
     * The interface to create/access and maintain SB Templates 
     * (SBTemplate).
     **/
    interface ISBTemplateService
    {

        /**
         * Create a new SB Template with the given parameters
         *
         * @param name The SBTemplate name
         * @param template The ObsParameterTemplate describing the SBTemplate
         * @param obsproc The ObsProcedure script
         * @param status The SBTemplate Status
         * @param version The version number to set
         *
         **/
        long create(string name,
                    askap::interfaces::ParameterMap template,
                    string obsproc, SBTemplateStatus status, Version version);

        /**
         * Make an identical copy and return th new id
         **/
        long clone(long sbtid)
            throws NoSuchSBTemplateException;

        /**
         * Modify the Status of the specified SBTemplate
         **/
        void setStatus(long sbtid, SBTemplateStatus status)
            throws NoSuchSBTemplateException;

        /**
         * Modify the name of the specified SBTemplate
         **/
        void setName(long sbtid, string name)
            throws NoSuchSBTemplateException;

        /**
         * Set the version of the given SBTemplate
         **/
        void setVersion(long sbtid, Version version)
            throws NoSuchSBTemplateException;


        /**
         * Update the SBTemplate's template and procedure. It is required to specify
         * a new version as well.
         **/
        void setTemplate(long sbtid, Version version,
                         askap::interfaces::ParameterMap template,
                         string obsproc);

        /**
         * Get a sequence of all available SB Template names.
         **/
        StringSeq getNames();

        /**
         * Get the SB Template names filtering by SB Status
         * version.
         **/
        StringSeq getByStatus(SBTemplateStatus status);

        /**
         * Get the SB Template id for a named SBTemplate with an 
	 * optional version (uses the  default version if version empty).
         **/
        long getByName(string name, Version version);

        /**
         * Retrieve the parameters making up the template
         **/
        askap::interfaces::ParameterMap getObsParamTemplate(long sbtid)
            throws NoSuchSBTemplateException;

        /**
         * Get the script associated with this template
         **/
        string getObsProcedure(long sbtid)
            throws NoSuchSBTemplateException;

        /**
         * Get the name of the specified SBTemplate
         **/
        string getName(long sbtid) throws NoSuchSBTemplateException;

        /**
         * Get the version of the specified SBTemplate
         **/
        Version getVersion(long sbtid) throws NoSuchSBTemplateException;

    };

};
};
};

#endif
