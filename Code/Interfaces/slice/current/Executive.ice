// @file Executive.ice
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

#ifndef ASKAP_EXECUTIVE_ICE
#define ASKAP_EXECUTIVE_ICE

#include <CommonTypes.ice>
#include <IService.ice>

module askap
{

module interfaces
{

module executive
{
  /**
   * Service interface to the askap Executive. This can be used
   * to run the Executive through e.g. an Operator Display or a
   * script.
   **/
  interface IExecutiveService extends askap::interfaces::services::IService
  {
    /**
     * Start the executive - enable access to a queue of SchedulingBlocks
     * and execute them
     **/
    ["ami"] void start();
    /**
     * Stop the executive - finishing gracefully after the current
     * execution of a SchedulingBlock has finished.
     **/
    ["ami"] void stop();
    /**
     * Abort the Executive, interrupting the current SchedulingBlock.
     **/
    ["ami"] void abort();

    /**
     * TEMPORARY method to allow loading a "Scheduling Block" made up from
     * the Procedure string and a ParameterMap (parset).
     **/
    ["ami"] void upload(string procedure, askap::interfaces::ParameterMap pmap);
  };
};
};
};

#endif
