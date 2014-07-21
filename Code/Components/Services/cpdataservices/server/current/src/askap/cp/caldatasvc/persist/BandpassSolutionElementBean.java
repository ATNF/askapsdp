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
package askap.cp.caldatasvc.persist;

public class BandpassSolutionElementBean {
	private long itsSolutionID;
	private short itsAntennaID;
	private short itsBeamID;
	private int itsChan;
	
	private float itsG1Real;
	private float itsG1Imag;
	private boolean itsG1Valid;
	
	private float itsG2Real;
	private float itsG2Imag;
	private boolean itsG2Valid;
	
	/**
	 * Constructor
	 * @note This no-args constructor is needed by Hibernate
	 */
	public BandpassSolutionElementBean() {
		itsSolutionID = -1;
		itsAntennaID = -1;
		itsBeamID = -1;
		itsChan = -1;
		itsG1Valid = false;
		itsG2Valid = false;
	}
	
	public BandpassSolutionElementBean(long solutionID,
			short antennaID, short beamID, int chan,
			float g1Real, float g1Imag, boolean g1Valid,
			float g2Real, float g2Imag, boolean g2Valid) {
		itsSolutionID = solutionID;
		itsAntennaID = antennaID;
		itsBeamID = beamID;
		itsChan = chan;
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
	 * @return the chan
	 */
	public int getChan() {
		return itsChan;
	}

	/**
	 * @param chan the chan to set
	 */
	public void setChan(int chan) {
		itsChan = chan;
	}
	
	/**
	 * @return the g1Real
	 */
	public float getG1Real() {
		return itsG1Real;
	}

	/**
	 * @param g1Real the g1Real to set
	 */
	public void setG1Real(float g1Real) {
		itsG1Real = g1Real;
	}

	/**
	 * @return the g1Imag
	 */
	public float getG1Imag() {
		return itsG1Imag;
	}

	/**
	 * @param g1Imag the g1Imag to set
	 */
	public void setG1Imag(float g1Imag) {
		itsG1Imag = g1Imag;
	}

	/**
	 * @return the g1Valid
	 */
	public boolean isG1Valid() {
		return itsG1Valid;
	}

	/**
	 * @param g1Valid the g1Valid to set
	 */
	public void setG1Valid(boolean g1Valid) {
		itsG1Valid = g1Valid;
	}

	/**
	 * @return the g2Real
	 */
	public float getG2Real() {
		return itsG2Real;
	}

	/**
	 * @param g2Real the g2Real to set
	 */
	public void setG2Real(float g2Real) {
		itsG2Real = g2Real;
	}

	/**
	 * @return the g2Imag
	 */
	public float getG2Imag() {
		return itsG2Imag;
	}

	/**
	 * @param g2Imag the g2Imag to set
	 */
	public void setG2Imag(float g2Imag) {
		itsG2Imag = g2Imag;
	}

	/**
	 * @return the g2Valid
	 */
	public boolean isG2Valid() {
		return itsG2Valid;
	}

	/**
	 * @param g2Valid the g2Valid to set
	 */
	public void setG2Valid(boolean g2Valid) {
		itsG2Valid = g2Valid;
	}

}
