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
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
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
	/**
	 * Logger
	 * */
	private static Logger logger = Logger.getLogger(PersistenceInterface.class
			.getName());
	
	/**
	 * Size of batches for SQL inserts and updates
	 */
	private int itsBatchSize = 20;
	
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
	
	public long addGainSolution(TimeTaggedGainSolution solution) {
		Transaction tx = itsSession.beginTransaction();
		GainSolutionBean solbean = new GainSolutionBean(solution.timestamp);
		itsSession.save(solbean);
		int count = 0;
		for (Map.Entry<JonesIndex,JonesJTerm> entry : solution.gain.entrySet()) {
			JonesIndex key = entry.getKey();
			JonesJTerm value = entry.getValue();
			itsSession.save(new GainSolutionElementBean(solbean.getID(),
					key.antennaID, key.beamID,
					value.g1.real, value.g1.imag, value.g1Valid,
					value.g2.real, value.g2.imag, value.g2Valid));
			if (count == itsBatchSize) {
				itsSession.flush();
				itsSession.clear();
				count = 0;
			} else {
				count++;
			}
		}
		tx.commit();
		return solbean.getID();
	}
	
	public long addLeakageSolution(TimeTaggedLeakageSolution solution) {
		Transaction tx = itsSession.beginTransaction();
		LeakageSolutionBean solbean = new LeakageSolutionBean(solution.timestamp);
		itsSession.save(solbean);
		int count = 0;
		for (Map.Entry<JonesIndex,DoubleComplex> entry : solution.leakage.entrySet()) {
			JonesIndex key = entry.getKey();
			DoubleComplex value = entry.getValue();
			itsSession.save(new LeakageSolutionElementBean(solbean.getID(),
					key.antennaID, key.beamID,
					value.real, value.imag));
			if (count == itsBatchSize) {
				itsSession.flush();
				itsSession.clear();
				count = 0;
			} else {
				count++;
			}
		}
		tx.commit();
		return solbean.getID();
	}
	
	public long addBandpassSolution(TimeTaggedBandpassSolution solution) {
		Transaction tx = itsSession.beginTransaction();
		BandpassSolutionBean solbean = new BandpassSolutionBean(solution.timestamp, solution.nChan);
		itsSession.save(solbean);
		int count = 0;
		for (Map.Entry<JonesIndex,FrequencyDependentJTerm> entry : solution.bandpass.entrySet()) {
			JonesIndex key = entry.getKey();
			FrequencyDependentJTerm value = entry.getValue();
			int chan = 1;
			for (JonesJTerm jterm: value.bandpass) {
				itsSession.save(new BandpassSolutionElementBean(solbean.getID(),
						key.antennaID, key.beamID, chan,
						jterm.g1.real, jterm.g1.imag, jterm.g1Valid,
						jterm.g2.real, jterm.g2.imag, jterm.g2Valid));
				chan++;
				
				if (count == itsBatchSize) {
					itsSession.flush();
					itsSession.clear();
					count = 0;
				} else {
					count++;
				}
			}
		}
		tx.commit();
		return solbean.getID();
	}
	
	public TimeTaggedGainSolution getGainSolution(long id) {
		String query = "from GainSolutionBean where id = " + id;
		@SuppressWarnings("unchecked")
		List<GainSolutionBean> result = (List<GainSolutionBean>) itsSession.createQuery(query).list();
		if (result.size() == 0) {
			return null;
		} else if (result.size() > 1) {
			logger.warn("Multiple records returned for query: " + query);
		}
		GainSolutionBean bean = result.get(0);
		
		// Build the Ice type
		TimeTaggedGainSolution ice_sol = new TimeTaggedGainSolution();
		ice_sol.timestamp = bean.getTimestamp();
		ice_sol.gain = new HashMap<JonesIndex,JonesJTerm>();
		
		// Need to query the element table for each map entry
		query = "from GainSolutionElementBean where solution_id = " + id + " order by antennaID asc, beamID asc";
		@SuppressWarnings("unchecked")
		List<GainSolutionElementBean> elements = (List<GainSolutionElementBean>) itsSession.createQuery(query).list();
		for ( GainSolutionElementBean element : (List<GainSolutionElementBean>) elements ) {
			JonesIndex jind = new JonesIndex();
			jind.antennaID = element.getAntennaID();
			jind.beamID = element.getBeamID();
			JonesJTerm jterm = new JonesJTerm();
			
			jterm.g1 = new askap.interfaces.DoubleComplex();
			jterm.g1.real = element.getG1Real();
			jterm.g1.imag = element.getG1Imag();
			jterm.g1Valid = element.isG1Valid();
			
			jterm.g2 = new askap.interfaces.DoubleComplex();
			jterm.g2.real = element.getG2Real();
			jterm.g2.imag = element.getG2Imag();
			jterm.g2Valid = element.isG2Valid();

			ice_sol.gain.put(jind, jterm);
		}

		return ice_sol;
	}
	
	public TimeTaggedLeakageSolution getLeakageSolution(long id) {
		String query = "from LeakageSolutionBean where id = " + id;
		@SuppressWarnings("unchecked")
		List<LeakageSolutionBean> result = (List<LeakageSolutionBean>) itsSession.createQuery(query).list();
		if (result.size() == 0) {
			return null;
		} else if (result.size() > 1) {
			logger.warn("Multiple records returned for query: " + query);
		}
		LeakageSolutionBean bean = result.get(0);
		
		// Build the Ice type
		TimeTaggedLeakageSolution ice_sol = new TimeTaggedLeakageSolution();
		ice_sol.timestamp = bean.getTimestamp();
		ice_sol.leakage = new HashMap<JonesIndex,DoubleComplex>();
		
		// Need to query the element table for each map entry
		query = "from LeakageSolutionElementBean where solution_id = " + id + " order by antennaID asc, beamID asc";
		@SuppressWarnings("unchecked")
		List<LeakageSolutionElementBean> elements = (List<LeakageSolutionElementBean>) itsSession.createQuery(query).list();
		for ( LeakageSolutionElementBean element : (List<LeakageSolutionElementBean>) elements ) {
			JonesIndex jind = new JonesIndex();
			jind.antennaID = element.getAntennaID();
			jind.beamID = element.getBeamID();
			askap.interfaces.DoubleComplex leakage = new DoubleComplex();
			leakage.real = element.getLeakageReal();
			leakage.imag = element.getLeakageImag();

			ice_sol.leakage.put(jind, leakage);
		}

		return ice_sol;
	}
	
	public TimeTaggedBandpassSolution getBandpassSolution(long id) {
		String query = "from BandpassSolutionBean where id = " + id;
		@SuppressWarnings("unchecked")
		List<BandpassSolutionBean> result = (List<BandpassSolutionBean>) itsSession.createQuery(query).list();
		if (result.size() == 0) {
			return null;
		} else if (result.size() > 1) {
			logger.warn("Multiple records returned for query: " + query);
		}
		BandpassSolutionBean bean = result.get(0);
				
		// Build the Ice type
		TimeTaggedBandpassSolution ice_sol = new TimeTaggedBandpassSolution();
		ice_sol.timestamp = bean.getTimestamp();
		ice_sol.bandpass = new HashMap<JonesIndex,FrequencyDependentJTerm>();
		
		// Need to query the element table for each map entry
		query = "from BandpassSolutionElementBean where solution_id = " + id 
		+ " and chan = 1 order by antennaID asc, beamID asc";
		@SuppressWarnings("unchecked")
		List<BandpassSolutionElementBean> elements = (List<BandpassSolutionElementBean>) itsSession.createQuery(query).list();
		for ( BandpassSolutionElementBean element : (List<BandpassSolutionElementBean>) elements ) {
			JonesIndex jind = new JonesIndex();
			jind.antennaID = element.getAntennaID();
			jind.beamID = element.getBeamID();
			
			FrequencyDependentJTerm terms = new FrequencyDependentJTerm();
			terms.bandpass = new ArrayList<JonesJTerm>(bean.getnChan());
			
			// Now need to query just that JonesIndex for all channels
			query = "from BandpassSolutionElementBean where solution_id = " + id 
			+ " and antennaID = " + jind.antennaID 
			+ " and beamID = " + jind.beamID 
			+ " order by chan asc";

			ice_sol.bandpass.put(jind, terms);
			@SuppressWarnings("unchecked")
			List<BandpassSolutionElementBean> innerElements = (List<BandpassSolutionElementBean>) itsSession.createQuery(query).list();
			for ( BandpassSolutionElementBean innerElement : (List<BandpassSolutionElementBean>) innerElements ) {
				JonesJTerm jterm = new JonesJTerm();
				jterm.g1 = new askap.interfaces.DoubleComplex();
				jterm.g1.real = innerElement.getG1Real();
				jterm.g1.imag = innerElement.getG1Imag();
				jterm.g1Valid = innerElement.isG1Valid();

				jterm.g2 = new askap.interfaces.DoubleComplex();
				jterm.g2.real = innerElement.getG2Real();
				jterm.g2.imag = innerElement.getG2Imag();
				jterm.g2Valid = innerElement.isG2Valid();
				terms.bandpass.add(jterm);
			}
			assert terms.bandpass.size() == bean.getnChan();
			ice_sol.bandpass.put(jind, terms);
		}
		
		return ice_sol;
	}
}
