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
	long id;
	
	JonesIndex itsJonesIndex;
	
	boolean g1Valid;
	double g1Real;
	double g1Imag;
	
	boolean g2Valid;
	double g2Real;
	double g2Imag;
	
	/**
	 * @return the id
	 */
	public Long getId() {
		return id;
	}
	
	/**
	 * @param id the id to set
	 */
	public void setId(Long id) {
		this.id = id;
	}
	
	/**
	 * @return the itsJonesIndex
	 */
	public JonesIndex getJonesIndex() {
		return itsJonesIndex;
	}
	
	/**
	 * @param itsJonesIndex the itsJonesIndex to set
	 */
	public void setJonesIndex(JonesIndex jonesIndex) {
		this.itsJonesIndex = jonesIndex;
	}
	
	/**
	 * @return the g1Valid
	 */
	public boolean isG1Valid() {
		return g1Valid;
	}
	
	/**
	 * @param g1Valid the g1Valid to set
	 */
	public void setG1Valid(boolean g1Valid) {
		this.g1Valid = g1Valid;
	}
	
	/**
	 * @return the g1Real
	 */
	public double getG1Real() {
		return g1Real;
	}
	
	/**
	 * @param g1Real the g1Real to set
	 */
	public void setG1Real(double g1Real) {
		this.g1Real = g1Real;
	}
	
	/**
	 * @return the g1Imag
	 */
	public double getG1Imag() {
		return g1Imag;
	}
	
	/**
	 * @param g1Imag the g1Imag to set
	 */
	public void setG1Imag(double g1Imag) {
		this.g1Imag = g1Imag;
	}
	
	/**
	 * @return the g2Valid
	 */
	public boolean isG2Valid() {
		return g2Valid;
	}
	
	/**
	 * @param g2Valid the g2Valid to set
	 */
	public void setG2Valid(boolean g2Valid) {
		this.g2Valid = g2Valid;
	}
	
	/**
	 * @return the g2Real
	 */
	public double getG2Real() {
		return g2Real;
	}
	
	/**
	 * @param g2Real the g2Real to set
	 */
	public void setG2Real(double g2Real) {
		this.g2Real = g2Real;
	}
	
	/**
	 * @return the g2Imag
	 */
	public double getG2Imag() {
		return g2Imag;
	}
	
	/**
	 * @param g2Imag the g2Imag to set
	 */
	public void setG2Imag(double g2Imag) {
		this.g2Imag = g2Imag;
	}
}
