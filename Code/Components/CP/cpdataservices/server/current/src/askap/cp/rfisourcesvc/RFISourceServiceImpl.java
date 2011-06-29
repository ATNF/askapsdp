/**
 *  Copyright (c) 2011 CSIRO - Australia Telescope National Facility (ATNF)
 *  
 *  Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 *  PO Box 76, Epping NSW 1710, Australia
 *  atnf-enquiries@csiro.au
 * 
 *  This file is part of the ASKAP software distribution.
 * 
 *  The ASKAP software distribution is free software: you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the License,
 *  or (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
package askap.cp.rfisourcesvc;

// Java imports
import java.util.List;

// ASKAPsoft imports
import org.apache.log4j.Logger;
import Ice.Current;
import askap.cp.rfisourcesvc.persist.PersistenceInterface;
import askap.interfaces.rfisourcesvc.EntryDoesNotExistException;
import askap.interfaces.rfisourcesvc.RFIEntry;
import askap.interfaces.rfisourcesvc._IRFISourceServiceDisp;

/**
 * Implementation of the RFI Source Service ICE interface.
 * @author Ben Humphreys
 */
public class RFISourceServiceImpl extends _IRFISourceServiceDisp {

	/** 
	 * Logger
	 */
	private static Logger logger = Logger.getLogger(RFISourceServiceImpl.class
			.getName());
	
	/**
	 * Class which provides access to the persistence layer
	 */
	private askap.cp.rfisourcesvc.persist.PersistenceInterface itsPersistance;
	
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	/**
	 * Constructor
	 */
	public RFISourceServiceImpl(Ice.Communicator ic) {
		logger.info("Creating RFI Source Service");
		itsPersistance = new PersistenceInterface();
	}

	/**
	 * finalize
	 */
	public void finalize() {
		logger.info("Destroying RFI Source Service");
	}
	
	/**
	 * @see askap.interfaces.rfisourcesvc._IRFISourceServiceOperations#addEntry(askap.interfaces.rfisourcesvc.RFIEntry, Ice.Current)
	 */
	@Override
	public long addEntry(RFIEntry entry, Current cur) {
		return itsPersistance.addEntry(entry);
	}

	/**
	 * @see askap.interfaces.rfisourcesvc._IRFISourceServiceOperations#getAllEntries(Ice.Current)
	 */
	@Override
	public List<RFIEntry> getAllEntries(Current cur) {
		return itsPersistance.getAllEntries();
	}

	/**
	 * @see askap.interfaces.rfisourcesvc._IRFISourceServiceOperations#modifyEntry(askap.interfaces.rfisourcesvc.RFIEntry, Ice.Current)
	 */
	@Override
	public void modifyEntry(RFIEntry entry, Current cur)
			throws EntryDoesNotExistException {
		itsPersistance.modifyEntry(entry);
	}
}
