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

public class GainSolutionElementBean {
	private long itsSolutionID;
	private short itsAntennaID;
	private short itsBeamID;
	
	private double itsG1Real;
	private double itsG1Imag;
	private boolean itsG1Valid;
	
	private double itsG2Real;
	private double itsG2Imag;
	private boolean itsG2Valid;
	
	/**
	 * Constructor
	 * @note This no-args constructor is needed by Hibernate
	 */
	public GainSolutionElementBean() {
		itsSolutionID = -1;
		itsAntennaID = -1;
		itsBeamID = -1;
		itsG1Valid = false;
		itsG2Valid = false;
	}
	
	public GainSolutionElementBean(long solutionID,
			short antennaID, short beamID,
			double g1Real, double g1Imag, boolean g1Valid,
			double g2Real, double g2Imag, boolean g2Valid) {
		itsSolutionID = solutionID;
		itsAntennaID = antennaID;
		itsBeamID = beamID;
		itsG1Real = g1Real;
		itsG1Imag = g1Imag;
		itsG1Valid = g1Valid;
		itsG2Real = g2Real;
		itsG2Imag = g2Imag;
		itsG2Valid = g2Valid;
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
	 * @return the itsAntennaID
	 */
	public short getAntennaID() {
		return itsAntennaID;
	}

	/**
	 * @param itsAntennaID the itsAntennaID to set
	 */
	public void setAntennaID(short antennaID) {
		itsAntennaID = antennaID;
	}

	/**
	 * @return the itsBeamID
	 */
	public short getBeamID() {
		return itsBeamID;
	}

	/**
	 * @param itsBeamID the itsBeamID to set
	 */
	public void setBeamID(short beamID) {
		itsBeamID = beamID;
	}

	/**
	 * @return the itsG1Real
	 */
	public double getG1Real() {
		return itsG1Real;
	}

	/**
	 * @param itsG1Real the itsG1Real to set
	 */
	public void setG1Real(double g1Real) {
		itsG1Real = g1Real;
	}

	/**
	 * @return the itsG1Imag
	 */
	public double getG1Imag() {
		return itsG1Imag;
	}

	/**
	 * @param itsG1Imag the itsG1Imag to set
	 */
	public void setG1Imag(double g1Imag) {
		itsG1Imag = g1Imag;
	}

	/**
	 * @return the itsG1Valid
	 */
	public boolean isG1Valid() {
		return itsG1Valid;
	}

	/**
	 * @param itsG1Valid the itsG1Valid to set
	 */
	public void setG1Valid(boolean g1Valid) {
		itsG1Valid = g1Valid;
	}

	/**
	 * @return the itsG2Real
	 */
	public double getG2Real() {
		return itsG2Real;
	}

	/**
	 * @param itsG2Real the itsG2Real to set
	 */
	public void setG2Real(double g2Real) {
		itsG2Real = g2Real;
	}

	/**
	 * @return the itsG2Imag
	 */
	public double getG2Imag() {
		return itsG2Imag;
	}

	/**
	 * @param itsG2Imag the itsG2Imag to set
	 */
	public void setG2Imag(double g2Imag) {
		itsG2Imag = g2Imag;
	}

	/**
	 * @return the itsG2Valid
	 */
	public boolean isG2Valid() {
		return itsG2Valid;
	}

	/**
	 * @param itsG2Valid the itsG2Valid to set
	 */
	public void setG2Valid(boolean g2Valid) {
		itsG2Valid = g2Valid;
	}

}
