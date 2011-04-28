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
	
	private double itsD12Real;
	private double itsD12Imag;
	private double itsD21Real;
	private double itsD21Imag;

	/**
	 * Constructor
	 * @note This no-args constructor is needed by Hibernate
	 */
	public LeakageSolutionElementBean() {
		itsSolutionID = -1;
		itsAntennaID = -1;
		itsBeamID = -1;
		itsD12Real = -1.0;
		itsD12Imag = -1.0;
		itsD21Real = -1.0;
		itsD21Imag = -1.0;
	}
	
	public LeakageSolutionElementBean(long solutionID,
			short antennaID, short beamID,
			double d12Real, double d12Imag,
			double d21Real, double d21Imag) {
		itsSolutionID = solutionID;
		itsAntennaID = antennaID;
		itsBeamID = beamID;
		itsD12Real = d12Real;
		itsD12Imag = d12Imag;
		itsD21Real = d21Real;
		itsD21Imag = d21Imag;
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

	////////////////////////
	// d12
	////////////////////////
	
	/**
	 * @return the d12Real
	 */
	public double getD12Real() {
		return itsD12Real;
	}

	/**
	 * @param d12Real the d12Real to set
	 */
	public void setD12Real(double d12Real) {
		itsD12Real = d12Real;
	}

	/**
	 * @return the d12Imag
	 */
	public double getD12Imag() {
		return itsD12Imag;
	}

	/**
	 * @param d12Imag the d12Imag to set
	 */
	public void setD12Imag(double d12Imag) {
		itsD12Imag = d12Imag;
	}

	////////////////////////
	// d21
	////////////////////////
	
	/**
	 * @return the d21Real
	 */
	public double getD21Real() {
		return itsD21Real;
	}

	/**
	 * @param d21Real the d21Real to set
	 */
	public void setD21Real(double d21Real) {
		itsD21Real = d21Real;
	}

	/**
	 * @return the d21Imag
	 */
	public double getD21Imag() {
		return itsD21Imag;
	}

	/**
	 * @param d21Imag the d21Imag to set
	 */
	public void setD21Imag(double d21Imag) {
		itsD21Imag = d21Imag;
	}
}
