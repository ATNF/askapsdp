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
	private long itsTimestamp;
	private short itsAntennaID;
	private short itsBeamID;
	
	private double itsLeakageReal;
	private double itsLeakageImag;

	
	public LeakageSolutionElementBean(long timestamp,
			short antennaID, short beamID,
			double leakageReal, double leakageImag) {
		itsTimestamp = timestamp;
		itsAntennaID = antennaID;
		itsBeamID = beamID;
		itsLeakageReal = itsLeakageImag;
		itsLeakageImag = leakageImag;

	}

	/**
	 * @return the timestamp
	 */
	public long getTimestamp() {
		return itsTimestamp;
	}

	/**
	 * @param timestamp the timestamp to set
	 */
	public void setTimestamp(long timestamp) {
		this.itsTimestamp = timestamp;
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
		this.itsAntennaID = antennaID;
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
		this.itsBeamID = beamID;
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
		this.itsLeakageReal = leakageReal;
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
		this.itsLeakageImag = leakageImag;
	}

}
