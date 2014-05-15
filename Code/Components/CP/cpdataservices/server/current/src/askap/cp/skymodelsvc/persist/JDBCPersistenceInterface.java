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
package askap.cp.skymodelsvc.persist;

// Java imports
import java.util.ArrayList;
import java.util.List;
import java.sql.*;


// ASKAPsoft imports
import org.apache.log4j.Logger;
import askap.interfaces.skymodelservice.Component;

public class JDBCPersistenceInterface {
	/**
	 * Logger
	 */
	private static Logger logger = Logger.getLogger(JDBCPersistenceInterface.class
			.getName());

	/**
	 * SQL Connection
	 */
	Connection itsConn = null;

	/**
	 * Constructor
	 */
	public JDBCPersistenceInterface() {
		logger.debug("Creating " + JDBCPersistenceInterface.class.getName());

		try {
			String userName = "skymodeluser";
			String password = "askap";
			String url = "jdbc:mysql://gijane/skymodel";
			Class.forName ("com.mysql.jdbc.Driver").newInstance ();
			itsConn = DriverManager.getConnection (url, userName, password);
			logger.debug("Database connection established");
		} catch (Exception e) {
			logger.error("Cannot create SQL connection: " + e.getMessage());
		}
	}

	protected void finalize() {
		if (itsConn != null) {
			try {
				itsConn.close();
			} catch (SQLException e) {
				logger.warn("SQLException while closing SQL connection: " + e.getMessage());
			}
		}
	}

	/**
	 * Perform a cone search.
	 *  
	 * @param ra	the right ascension of the centre of the
	 * 				search area (Units: decimal degrees).
	 * 				Range: 0 to 359.9...
	 * @param dec	the declination of the centre of the search
	 * 				area (Units: decimal degrees). Range: +/- 90
	 * @param searchRadius	the search radius (Units: decimal degrees).
	 * @param fluxLimit	low limit on flux on sources returned. All returned
	 * 					sources shall have flux >= fluxLimit (Units: Jy).
	 * @return	a list of component IDs for those components in the sky model
	 * 			which match the search criteria.
	 */
	public List<Long> coneSearch(double ra, double dec,
			double searchRadius, double fluxLimit) {
		ArrayList<Long> ids = new ArrayList<Long>();

		final String query = "SELECT id,right_ascension,declination FROM components WHERE (i1400 >=" + fluxLimit 
				+ ") AND (declination >= " + (dec - searchRadius)
				+ ") AND (declination <= " + (dec + searchRadius) + ")";

		try {
			Statement s = itsConn.createStatement(java.sql.ResultSet.TYPE_FORWARD_ONLY,
					java.sql.ResultSet.CONCUR_READ_ONLY);
			s.setFetchSize(Integer.MIN_VALUE);
			s.executeQuery(query);
			ResultSet rs = s.getResultSet();
			long count = 0;
			while (rs.next())
			{		
				Component comp = new Component();
				comp.rightAscension = rs.getDouble("right_ascension");
				comp.declination = rs.getDouble("declination");

				if (angularSeparation(ra, dec, comp) <= searchRadius) {
					ids.add(rs.getLong("id"));
				}
				count++;
				if (count % 10000 == 0) {
					logger.debug("Processed record set: " + count);
				}
			}
			rs.close ();
			s.close ();
		} catch (SQLException e) {
			logger.error("Error executing query: " + e.getMessage());
		}

		return ids;
	}


	/**
	 * @param componentIds	a list of component IDs for which the component
	 * 						objects they identify are to be returned.
	 * @return a list of components.
	 */
	public List<Component> getComponents(final List<Long> componentIds) {
		ArrayList<Component> components = new ArrayList<Component>(componentIds.size());

		final String query = "SELECT * FROM components WHERE (id=?)";
		try {
			PreparedStatement s = itsConn.prepareStatement(query,
					ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_READ_ONLY);
			
			// Process each element of the input
			for (Long id : componentIds) {
				s.setLong(1, id);
				ResultSet rs = s.executeQuery();
				while (rs.next()) {		
					Component c = new Component();
					c.id = rs.getLong("id");
					c.rightAscension = rs.getDouble("right_ascension");
					c.declination = rs.getDouble("declination");
					c.positionAngle = rs.getDouble("position_angle");
					c.majorAxis = rs.getDouble("major_axis");
					c.minorAxis = rs.getDouble("minor_axis");
					c.i1400 = rs.getDouble("i1400");
					c.spectralIndex = rs.getDouble("spectral_index");
					c.spectralCurvature = rs.getDouble("spectral_curvature");
					components.add(c);
				}
				rs.close();
			}

			s.close();
		} catch (SQLException e) {
			logger.error("Error executing query: " + e.getMessage());
		}

		return components;
	}


	/**
	 * @param components a list of components to add to the sky model.
	 * @return a list of component IDs, with index matching that of the
	 * "components" parameter passed as input.
	 */
	public List<Long> addComponents(final List<Component> components) {
		ArrayList<Long> idList = new ArrayList<Long>();

		return idList;
	}


	/**
	 * @param componentIds	a vector of component IDs for which those
	 * 						components are to be removed from the sky model.
	 */
	public void removeComponents(List<Long> componentIds) {	
	}

	/**
	 * Calculates the angular separation between a position given by ra/dec
	 * and a component instance.
	 * 
	 * @param ra	right ascension in decimal degrees (Range: 0 to 359.9...)
	 * @param dec	declination in decimal degrees (Range: +/-90)
	 * @param comp	component to determine angular separation for
	 * @return	the angular separation between the position specified by the
	 * 			parameters ra and dec, and the specified component.
	 */
	static double angularSeparation(double ra, final double dec, final Component comp) {
		final double[] refXYZ = createXYZ(Math.toRadians(ra), Math.toRadians(dec));
		final double[] compXYZ = createXYZ(Math.toRadians(comp.rightAscension),
				Math.toRadians(comp.declination));

		final double d1 = Math.sqrt(square(refXYZ[0] - compXYZ[0]) +
				square(refXYZ[1] - compXYZ[1]) +
				square(refXYZ[2] - compXYZ[2]))/2.0;
		return Math.toDegrees(2 * Math.asin(d1 < 1.0 ? d1 : 1.0));
	}

	/**
	 * Computes the square of a number.
	 * @param val	the value to square.
	 * @return	the square of the "val" parameter.
	 */
	static double square(final double val) {
		return Math.pow(val, 2);
	}

	/**
	 * For a given ra/dec creates a 3-vector of direction cosines,
	 * the Cartesian coordinates for the given position.
	 * @param ra	right ascension in decimal degrees (Range: 0 to 359.9...)
	 * @param dec	declination in decimal degrees (Range: +/-90)
	 * @return	the Cartesian coordinates, x, y & z. Where x points 
	 * 			to the prime meridian equator, z points to the north pole,  
	 * 			and y is normal to x and z.
	 */
	static double[] createXYZ(final double ra, final double dec) {
		double[] xyz = new double[3];
		final double loc = Math.cos(dec);
		xyz[0] = Math.cos(ra) * loc;
		xyz[1] = Math.sin(ra) * loc;
		xyz[2] = Math.sin(dec);
		return xyz;
	}
}
