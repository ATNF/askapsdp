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
import org.hibernate.Criteria;
import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.hibernate.Transaction;
import org.hibernate.cfg.Configuration;
import org.hibernate.criterion.Projections;
import askap.interfaces.DoubleComplex;
import askap.interfaces.calparams.TimeTaggedBandpassSolution;
import askap.interfaces.calparams.TimeTaggedGainSolution;
import askap.interfaces.calparams.TimeTaggedLeakageSolution;
import askap.interfaces.calparams.JonesIndex;
import askap.interfaces.calparams.JonesJTerm;
import askap.interfaces.calparams.JonesDTerm;

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
	 * 
	 * @param session	allows a hibernate session to be passed in. This exists to
	 *            		support unit testing. For non-unit test usage null should be
	 *            		passed.
	 */
	public PersistenceInterface(org.hibernate.Session session) {
		logger.debug("Creating " + PersistenceInterface.class.getName());

		if (session != null) {
			itsSession = session;
		} else {
			// A SessionFactory is set up for the persistence interface
			Configuration config = new Configuration();
			config.configure("calibration-hibernate.cfg.xml");
			SessionFactory sessionFactory = config.buildSessionFactory();
			itsSession = sessionFactory.openSession();
		}
	}

	protected void finalize() {
		itsSession.close();
	}

	public long addGainSolution(TimeTaggedGainSolution solution) {
		Transaction tx = itsSession.beginTransaction();
		GainSolutionBean solbean = new GainSolutionBean(solution.timestamp);
		itsSession.save(solbean);
		int count = 0;
		for (Map.Entry<JonesIndex, JonesJTerm> entry : solution.solutionMap.entrySet()) {
			JonesIndex key = entry.getKey();
			JonesJTerm value = entry.getValue();
			itsSession.save(new GainSolutionElementBean(solbean.getID(),
							key.antennaID, key.beamID, value.g1.real,
							value.g1.imag, value.g1Valid, value.g2.real,
							value.g2.imag, value.g2Valid));
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
		for (Map.Entry<JonesIndex, JonesDTerm> entry : solution.solutionMap.entrySet()) {
			JonesIndex key = entry.getKey();
			JonesDTerm value = entry.getValue();
			itsSession.save(new LeakageSolutionElementBean(solbean.getID(),
					key.antennaID, key.beamID,
					value.d12.real, value.d12.imag,
					value.d21.real, value.d21.imag));
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
		// TODO: Address hard-coding of number of channels. Need to better
		// understand how to manage the bandpass solution first.
		BandpassSolutionBean solbean = new BandpassSolutionBean(
				solution.timestamp, 16416);
		itsSession.save(solbean);
		int count = 0;
		for (Map.Entry<JonesIndex, List<JonesJTerm>> entry : solution.solutionMap.entrySet()) {
			JonesIndex key = entry.getKey();
			List<JonesJTerm> value = entry.getValue();
			int chan = 1;
			for (JonesJTerm jterm : value) {
				itsSession.save(new BandpassSolutionElementBean(
						solbean.getID(), key.antennaID, key.beamID, chan,
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
		ice_sol.solutionMap = new HashMap<JonesIndex, JonesJTerm>();

		// Need to query the element table for each map entry
		query = "from GainSolutionElementBean where solution_id = " + id
				+ " order by antennaID asc, beamID asc";
		@SuppressWarnings("unchecked")
		List<GainSolutionElementBean> elements = (List<GainSolutionElementBean>) itsSession.createQuery(query).list();
		for (GainSolutionElementBean element : (List<GainSolutionElementBean>) elements) {
			JonesIndex jind = new JonesIndex(element.getAntennaID(), element.getBeamID());
			JonesJTerm jterm = new JonesJTerm();

			jterm.g1 = new askap.interfaces.DoubleComplex(
					element.getG1Real(),
					element.getG1Imag());
			jterm.g1Valid = element.isG1Valid();

			jterm.g2 = new askap.interfaces.DoubleComplex(
					element.getG2Real(),
					element.getG2Imag());

			jterm.g2Valid = element.isG2Valid();

			ice_sol.solutionMap.put(jind, jterm);
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
		ice_sol.solutionMap = new HashMap<JonesIndex, JonesDTerm>();

		// Need to query the element table for each map entry
		query = "from LeakageSolutionElementBean where solution_id = " + id
				+ " order by antennaID asc, beamID asc";
		@SuppressWarnings("unchecked")
		List<LeakageSolutionElementBean> elements = (List<LeakageSolutionElementBean>) itsSession
				.createQuery(query).list();
		for (LeakageSolutionElementBean element : (List<LeakageSolutionElementBean>) elements) {
			JonesIndex jind = new JonesIndex(element.getAntennaID(), element.getBeamID());
			askap.interfaces.calparams.JonesDTerm leakage = new JonesDTerm(
					new DoubleComplex(element.getD12Real(), element.getD12Imag()),
					new DoubleComplex(element.getD21Real(), element.getD21Imag()));

			ice_sol.solutionMap.put(jind, leakage);
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
		ice_sol.solutionMap = new HashMap<JonesIndex, List<JonesJTerm>>();

		// Need to query the element table for each map entry
		query = "from BandpassSolutionElementBean where solution_id = " + id
				+ " and chan = 1 order by antennaID asc, beamID asc";
		@SuppressWarnings("unchecked")
		List<BandpassSolutionElementBean> elements = (List<BandpassSolutionElementBean>) itsSession.createQuery(query).list();
		for (BandpassSolutionElementBean element : (List<BandpassSolutionElementBean>) elements) {
			JonesIndex jind = new JonesIndex(element.getAntennaID(), element.getBeamID());

			List<JonesJTerm> terms = new ArrayList<JonesJTerm>(bean.getnChan());

			// Now need to query just that JonesIndex for all channels
			query = "from BandpassSolutionElementBean where solution_id = "
					+ id + " and antennaID = " + jind.antennaID
					+ " and beamID = " + jind.beamID + " order by chan asc";

			ice_sol.solutionMap.put(jind, terms);
			@SuppressWarnings("unchecked")
			List<BandpassSolutionElementBean> innerElements = (List<BandpassSolutionElementBean>) itsSession
					.createQuery(query).list();
			for (BandpassSolutionElementBean innerElement : (List<BandpassSolutionElementBean>) innerElements) {
				JonesJTerm jterm = new JonesJTerm();
				jterm.g1 = new askap.interfaces.DoubleComplex(
						innerElement.getG1Real(),
						innerElement.getG1Imag());
				jterm.g1Valid = innerElement.isG1Valid();

				jterm.g2 = new askap.interfaces.DoubleComplex(
						innerElement.getG2Real(),
						innerElement.getG2Imag());
				jterm.g2Valid = innerElement.isG2Valid();
				
				terms.add(jterm);
			}
			assert terms.size() == bean.getnChan();
			ice_sol.solutionMap.put(jind, terms);
		}

		return ice_sol;
	}

	public long getLatestGainSolution() {
		return getLatestSolution(GainSolutionBean.class);
	}

	public long getLatestLeakageSolution() {
		return getLatestSolution(LeakageSolutionBean.class);
	}

	public long getLatestBandpassSolution() {
		return getLatestSolution(BandpassSolutionBean.class);
	}

	private long getLatestSolution(Class<?> c) {
		Criteria criteria = itsSession.createCriteria(c).setProjection(
				Projections.max("id"));
		Long id = (Long) criteria.uniqueResult();
		if (id == null) {
			return -1;
		} else {
			return id;
		}
	}
}
