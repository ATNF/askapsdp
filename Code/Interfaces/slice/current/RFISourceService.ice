// @file RFISourceService.ice
//
// @copyright (c) 2011 CSIRO
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

#ifndef ASKAP_RFI_SOURCE_SERVICE_ICE
#define ASKAP_RFI_SOURCE_SERVICE_ICE

#include <CommonTypes.ice>
#include <IService.ice>

module askap
{
module interfaces
{
module rfisourcesvc
{
    /**
     * An RFI database entry 
     **/
    struct RFIEntry
    {
        /**
         * Unique index number
         * Units: N/A
         **/
        long id;

        /**
         * Active
         * Units: N/A
         **/
        bool active;

        /**
         * Sky Frequency (Centre)
         * Units: Hz
         **/
        double skyFrequency;

        /**
         * Bandwidth
         * Thus the frequency range is: skyFrequency +/- bandwidth/2
         * Units: Hz
         **/
        double bandwidth;

        /**
         * Direction Specific Flag
         * Units: N/A
         **/
        bool dirSpecific;

        /**
         * Azimuth
         * Note: This is only valid if dirSpecific is true
         * Units: Degrees
         **/
        double azimuth;

        /**
         * Azimuth Range
         * Note: This is only valid if dirSpecific is true
         * Thus the affected azimuth range is: azimuth +/- azimuthRange/2
         * Units: Degrees
         **/
        double azimuthRange;

        /**
         * Elevation
         * Note: This is only valid if dirSpecific is true
         * Units: Degrees
         **/
        double elevation;

        /**
         * Elevation Range
         * Note: This is only valid if dirSpecific is true
         * Thus the affected elevation range is:  elevation +/- elevationRange/2
         * Units: Degrees
         **/
        double elevationRange;

        /**
         * Time Specific Flag
         * Units: N/A
         **/
        bool timeSpecific;

        /**
         * Start time (in seconds since midnight UT)
         * Note: This is only valid if timeSpecific is true
         * Units: Seconds
         **/
        int startTime;

        /**
         * End time (in seconds since midnight UT)
         * Note: This is only valid if timeSpecific is true
         * Units: Seconds
         **/
        int endTime;

        /**
         * Date Reported (in seconds since MJD=0)
         * Units: seconds
         **/
        long dateReported;

        /**
         * Source
         * A description of the source of the RFI (if known), otherwise
         * an empty string
         * Units: N/A
         **/
        string source;

        /**
         * Miscellaneous Comments
         * Comments, otherwise an empty string.
         * Units: N/A
         **/
        string comments;
    };

    /**
     * A sequence of RFI source entries
     **/
    ["java:type:java.util.ArrayList<askap.interfaces.rfisourcesvc.RFIEntry>"]
    sequence<RFIEntry> RFIEntrySeq;

    /**
     * This exception is thrown when an operation on an entry fails because the
     * entry ID is not found.
     **/
    exception EntryDoesNotExistException extends askap::interfaces::AskapIceException
    {
    };

    /**
     * Interface to the RFI Source Service.
     **/
    interface IRFISourceService extends askap::interfaces::services::IService
    {
        /**
         * Add an entry to the RFI source database.
         *
         * @param   entry the new entry to add to the database. Note the "id"
         *          field is ignored, an id is assigned by the service.  Also
         *          the dateReported element if set to -1 will be auto-assigned
         *          by the service.
         * @return the entry ID assigned to the added entry.
         **/
        long addEntry(RFIEntry entry);

        /**
         * Modify an entry in the RFI source database.
         *
         * @param   entry the entry to modify to the database. The entry
         * with the id contained in the RFIEntry parameter is replaced with
         * contents of the RFIEntry parameter.
         **/
        void modifyEntry(RFIEntry entry) throws EntryDoesNotExistException;

        /**
         * Returns all entries in the RFI source database.
         * 
         * @return all entries in the RFI source database.
         **/
        RFIEntrySeq getAllEntries();
    };

};
};
};

#endif
