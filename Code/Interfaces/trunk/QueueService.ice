// @file QueueService.ice
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

#ifndef ASKAP_QUEUESERVICE_ICE
#define ASKAP_QUEUESERVICE_ICE

module askap
{

module interfaces
{

module schedblock
{

    /**
     * An interface to present a view of all SchedulingBlocks in the SCHEDULED
     * state sorted by scheduled date/time
     **/
    interface IQueueService
    {
        /**
         * Look at the first item in the Queue and determine if it is ready
         * for execution, i.e. the scheduled time has arrived. The item has
         * to be in SCHEDULED state. It returns the id of the SchedulingBlock
         * if it is raedy to be executed.
         * A typical use would be to call peek periodically until an id is
         * returned.
         **/
        long peek();
    };
};
};
};

#endif
