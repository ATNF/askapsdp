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

public class LeakageSolutionElementBean {
	private long itsSolutionID;
	private short itsAntennaID;
	private short itsBeamID;
	
	private double itsLeakageReal;
	private double itsLeakageImag;

	/**
	 * Constructor
	 * @note This no-args constructor is needed by Hibernate
	 */
	public LeakageSolutionElementBean() {
		itsSolutionID = -1;
		itsAntennaID = -1;
		itsBeamID = -1;
		itsLeakageReal = -1.0;
		itsLeakageImag = -1.0;
	}
	
	public LeakageSolutionElementBean(long solutionID,
			short antennaID, short beamID,
			double leakageReal, double leakageImag) {
		itsSolutionID = solutionID;
		itsAntennaID = antennaID;
		itsBeamID = beamID;
		itsLeakageReal = leakageReal;
		itsLeakageImag = leakageImag;

	}

	/**
	 * @return the solutionID
	 */
	public long getSolutionID() {
		return itsSolutionID;
	}

	/**
	 * @param solutionID the solution ID to set
	 */
	public void setSolutionID(long solutionID) {
		itsSolutionID = solutionID;
	}

	/**
	 * @return the antennaID
	 */
	public short getAntennaID() {
		return itsAntennaID;
	}

	/**
	 * @param antennaID the antennaID to set
	 */
	public void setAntennaID(short antennaID) {
		itsAntennaID = antennaID;
	}

	/**
	 * @return the beamID
	 */
	public short getBeamID() {
		return itsBeamID;
	}

	/**
	 * @param beamID the beamID to set
	 */
	public void setBeamID(short beamID) {
		itsBeamID = beamID;
	}

	/**
	 * @return the leakageReal
	 */
	public double getLeakageReal() {
		return itsLeakageReal;
	}

	/**
	 * @param leakageReal the leakageReal to set
	 */
	public void setLeakageReal(double leakageReal) {
		itsLeakageReal = leakageReal;
	}

	/**
	 * @return the leakageImag
	 */
	public double getLeakageImag() {
		return itsLeakageImag;
	}

	/**
	 * @param leakageImag the leakageImag to set
	 */
	public void setLeakageImag(double leakageImag) {
		itsLeakageImag = leakageImag;
	}

}
