// @file ObsProgramService.ice
//
// @copyright (c) 2010-2012 CSIRO
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

#ifndef ASKAP_OBSPROGRAMSERVICE_ICE
#define ASKAP_OBSPROGRAMSERVICE_ICE

#include <CommonTypes.ice>
#include <DataServiceExceptions.ice>
#include <IService.ice>

module askap
{

module interfaces
{

module schedblock
{


    struct ObsProgram 
    {
	/**
	 * Unique name identifying the observing program
	 */
	string name;
	
	/**
	 * Name of the principle investigator
	 */
	string investigator;
	
	/**
	 * A reference to the project that this program is originated from. This
	 * is typically a reference to the proposal in the ATNF "Online Proposal
	 * Applications & Links" (OPAL) system.
	 */
	string projectRef;
    };
    
    interface IObsProgramService extends askap::interfaces::services::IService
    {       
	/**
	 * Create an observing program.
	 * @throws NameAlreadyExistsException if an observing program with the
	 *                                    specified name already exists.
	 */
	void create(ObsProgram op) throws NameAlreadyExistsException;
	
	/**
	 * Gets a structure describing the observing program.
	 * @throws NoSuchObsProgramException if no observing program with the
	 *                                   specified name exists.
	 */
	ObsProgram get(string name) throws NoSuchObsProgramException;
	
	/**
	 * Returns a sequence of all observing program names
	 */
	StringSeq getAll();             
    };

};
};
};
#endif
