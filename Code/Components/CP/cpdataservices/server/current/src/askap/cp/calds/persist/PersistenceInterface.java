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
 * 
 * @author Ben Humphreys <ben.humphreys@csiro.au>
 */
package askap.cp.calds.persist;

// ASKAPsoft imports
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.log4j.Logger;
import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.hibernate.Transaction;
import org.hibernate.cfg.Configuration;
import askap.interfaces.calparams.JonesJTerm;
import askap.interfaces.calparams.TimeTaggedBandpassSolution;
import askap.interfaces.calparams.TimeTaggedGainSolution;
import askap.interfaces.calparams.TimeTaggedLeakageSolution;

public class PersistenceInterface {
	/** Logger. */
	private static Logger logger = Logger.getLogger(PersistenceInterface.class
			.getName());
	
	/**
	 * Hibernate session
	 */
	private Session itsSession = null;
	
	/**
	 * Constructor
	 */
	public PersistenceInterface() {
		logger.debug("Creating " + PersistenceInterface.class.getName() );
		
		// A SessionFactory is set up for the persistence interface
		Configuration config = new Configuration();
		config.configure("calibration-hibernate.cfg.xml");
		SessionFactory sessionFactory = config.buildSessionFactory();
		itsSession = sessionFactory.openSession();
	}
	
	/**
	 * Constructor.
	 * Used for testing.
	 * 
	 * @param session
	 */
	public PersistenceInterface(org.hibernate.Session session) {
		itsSession = session;
	}

	protected void finalize() {
		itsSession.close();
	}
	
	public void addGainSolution(TimeTaggedGainSolution solution) {
		Transaction tx = itsSession.beginTransaction();
		//itsSession.save(solution);
		tx.commit();
	}
	
	public void addLeakageSolution(TimeTaggedLeakageSolution solution) {
		//Transaction tx = itsSession.beginTransaction();
		//itsSession.save(solution);
		//tx.commit();
	}
	
	public void addBandpassSolution(TimeTaggedBandpassSolution solution) {
		//Transaction tx = itsSession.beginTransaction();
		//itsSession.save(solution);
		//tx.commit();
	}
	
	/*
	 * 
	 */
	TimeTaggedGainSolutionBean iceToBean(TimeTaggedGainSolution solution) {
		List<GainSolutionElementBean> solutionElements = new ArrayList<GainSolutionElementBean>();
		Map<askap.interfaces.calparams.JonesIndex, JonesJTerm> gainsMap = solution.gain;
		
		for (Entry<askap.interfaces.calparams.JonesIndex, JonesJTerm> entry : gainsMap.entrySet()) {
			GainSolutionElementBean eb = new GainSolutionElementBean();
			
			solutionElements.add(eb);
		}
		
		return new TimeTaggedGainSolutionBean(solution.timestamp, solutionElements);
	
	}

}
