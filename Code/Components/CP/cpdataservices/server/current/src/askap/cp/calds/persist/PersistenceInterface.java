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

// Java imports
import java.util.Map;

// ASKAPsoft imports
import org.apache.log4j.Logger;
import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.hibernate.Transaction;
import org.hibernate.cfg.Configuration;
import askap.interfaces.DoubleComplex;
import askap.interfaces.calparams.TimeTaggedBandpassSolution;
import askap.interfaces.calparams.TimeTaggedGainSolution;
import askap.interfaces.calparams.TimeTaggedLeakageSolution;
import askap.interfaces.calparams.JonesIndex;
import askap.interfaces.calparams.JonesJTerm;
import askap.interfaces.calparams.FrequencyDependentJTerm;

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
		itsSession.save(new GainSolutionBean(solution.timestamp));
		for (Map.Entry<JonesIndex,JonesJTerm> entry : solution.gain.entrySet()) {
			JonesIndex key = entry.getKey();
			JonesJTerm value = entry.getValue();
			itsSession.save(new GainSolutionElementBean(solution.timestamp,
					key.antennaID, key.beamID,
					value.g1.real, value.g1.imag, value.g1Valid,
					value.g2.real, value.g2.imag, value.g2Valid));
		}
		tx.commit();
	}
	
	public void addLeakageSolution(TimeTaggedLeakageSolution solution) {
		Transaction tx = itsSession.beginTransaction();
		itsSession.save(new LeakageSolutionBean(solution.timestamp));
		for (Map.Entry<JonesIndex,DoubleComplex> entry : solution.leakage.entrySet()) {
			JonesIndex key = entry.getKey();
			DoubleComplex value = entry.getValue();
			itsSession.save(new LeakageSolutionElementBean(solution.timestamp,
					key.antennaID, key.beamID,
					value.real, value.imag));
		}
		tx.commit();
	}
	
	public void addBandpassSolution(TimeTaggedBandpassSolution solution) {
		Transaction tx = itsSession.beginTransaction();
		itsSession.save(new BandpassSolutionBean(solution.timestamp, solution.nChan));
		for (Map.Entry<JonesIndex,FrequencyDependentJTerm> entry : solution.bandpass.entrySet()) {
			JonesIndex key = entry.getKey();
			FrequencyDependentJTerm value = entry.getValue();
			int chan = 1;
			for (JonesJTerm jterm: value.bandpass) {
				itsSession.save(new BandpassSolutionElementBean(solution.timestamp,
						key.antennaID, key.beamID, chan,
						jterm.g1.real, jterm.g1.imag, jterm.g1Valid,
						jterm.g2.real, jterm.g2.imag, jterm.g2Valid));
				chan++;
			}
		}
		tx.commit();
	}

}
