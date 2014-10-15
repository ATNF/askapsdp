/**
 *  Copyright (c) 2014 CSIRO - Australia Telescope National Facility (ATNF)
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
package askap.cp.manager.monitoring;

import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import askap.interfaces.monitoring.MonitorPoint;
import askap.util.ServiceManager;
import askap.util.TypedValueUtils;

/**
 * This singleton allows monitoring data to be submitted by some producer
 * and exposes the monitoring data via an Ice service interface. The typical
 * consumer of this data is the monitoring archiver or user interfaces.
 * 
 * Data is stored in a map, mapping a monitoring point name to the last point
 * data provided via the update() method. The Ice service object then accesses
 * this data via the get() method.
 * 
 * This object only retains the most recent state for a monitoring point, so a
 * call to update() will only be able to fetch the current (or most recently
 * supplied) monitoring point data.
 * 
 * @author Ben Humphreys
 */
public class MonitoringSingleton {

	/**
	 * Singleton instance of the MonitoringSingleton
	 */
	private static MonitoringSingleton instance;

	/**
	 * Contains current values for monitoring data. The key is the monitoring
	 * point name, while the value is the point data.
	 */
	private Map<String, MonitorPoint> pointdata = new HashMap<String, MonitorPoint>();
	
	/**
	 * Encapsulates management of the MonitoringProvider Ice Service
	 */
	private ServiceManager iceServiceManager;

	/**
	 * Initialises the singleton instance of the MonitoringSingleton
	 * 
	 * @param ic		ice communicator
	 * @param parset	parameter set
	 */
	public static synchronized void init(Ice.Communicator ic,
			String serviceName, String adapterName) {
		instance = new MonitoringSingleton(ic, serviceName, adapterName);
	}
	
	/**
	 * Destroys the singleton instance of the MonitoringSingleton. After this
	 * call is made, the only method that should be called is init()
	 */
	public static synchronized void destroy() {
		instance.iceServiceManager.stop();
		instance.iceServiceManager = null;
		instance = null;
	}

	/**
	 * @return the singleton instance of the MonitoringSingleton
	 * or null if the singleton is not initialised (likely indicating
	 * monitoring is not enabled)
	 */
	public static synchronized MonitoringSingleton getInstance() {
		return instance;
	}

	/**
	 * Constructor
	 * 
	 * @param ic		ice communicator
	 * @param parset	parameter set
	 */
	private MonitoringSingleton(Ice.Communicator ic,
			String serviceName, String adapterName) {
		iceServiceManager = new ServiceManager();
		MonitoringProviderImpl svc = new MonitoringProviderImpl(this);
		iceServiceManager.start(ic, svc, serviceName, adapterName);
	}

	/**
	 * Get the monitoring points associated with the supplied pointnames.
	 * 
	 * @note If the a point name in the input array is not present in the
	 * set of monitoring points that particular point will be omitted from
	 * the result set. As such, the returned vector will have length equal
	 * to or less than the "pointnames" array.
	 * 
	 * @param pointnames	an array of points for which the monitoring data
	 * 						will be fetched.
	 * @return 	the list of monitoring points requested. However non-existent
	 * 			points are omitted from the resulting set.
	 */
	public Vector<MonitorPoint> get(String[] pointnames) {
		Vector<MonitorPoint> out = new Vector<MonitorPoint>();

		// Take the "pointdata" map lock to ensure a consistent
		// view of all points (even if we used a thread-safe map, we still
		// want to take the lock to ensure calls to submit & get are atomic.
		synchronized (pointdata) {
			for (String name : pointnames) {
				final MonitorPoint value = pointdata.get(name);
				if (value != null) {
					out.add(value);
				}
			}
		}

		return out;
	}
	
	/**
	 * Submit an update to a monitoring point (without a unit)
	 */
	public <T> void update(String name, T value, MonitorPointStatus status) {
		update(name, value, status, "");
	}

	/**
	 * Submit an update to a monitoring point.
	 * 
	 * If a value for this point is already set it will be replaced with
	 * the supplied data.
	 * 
	 * This method adds a "cp.manager." prefix to all monitoring points.
	 * 
	 * @param name		a name identifying the monitoring point.
	 * @param value		the value a point has (e.g. some measurement or state)
	 * @param status	the status of the point
	 * @param unit		unit associated with the value
	 */
	public <T> void update(String name, T value, MonitorPointStatus status,
			String unit) {
		MonitorPoint point = new MonitorPoint();
		point.name = "cp.manager." + name;
		point.value = TypedValueUtils.object2TypedValue(value);
		point.status = toIceStatus(status);
		point.unit = unit;

		synchronized (pointdata) {
			pointdata.put(name, point);
		}
	}

	/**
	 * Maps from a native "MonitorPointStatus" to an Ice "PointStatus"
	 * 
	 * @param in 	the native point status
	 * @return 		an Ice point status
	 */
	private askap.interfaces.monitoring.PointStatus toIceStatus(
			MonitorPointStatus in) {
		switch (in) {
		case INVALID:
			return askap.interfaces.monitoring.PointStatus.INVALID;
		case MAJORALARM:
			return askap.interfaces.monitoring.PointStatus.MAJORALARM;
		case MINORALARM:
			return askap.interfaces.monitoring.PointStatus.MINORALARM;
		case OK:
			return askap.interfaces.monitoring.PointStatus.OK;
		default:
			throw new RuntimeException("Unmapped PointStatus: " + in);
		}
	}
}
