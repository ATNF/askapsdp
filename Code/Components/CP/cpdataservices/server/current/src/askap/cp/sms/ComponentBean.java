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
package askap.cp.sms;

/**
 *
 */
public class ComponentBean {
    /**
     * Unique component index number
     **/
    private long id = -1;

    /**
     * Right ascension in the J2000 coordinate system
     * Units: degrees
     **/
    private double rightAscension = 0.0;

    /**
     * Declination in the J2000 coordinate system
     * Units: degrees
     **/
    private double declination = 0.0;

    /**
     * Position angle. Counted east from north.
     * Units: radians
     **/
    private double positionAngle = 0.0;

    /**
     * Major axis
     * Units: arcsecs
     **/
    private double majorAxis = 0.0;

    /**
     * Minor axis
     * Units: arcsecs
     **/
    private double minorAxis = 0.0;

    /**
     * Flux at 1400 Mhz
     * Units: Jy (log10 of flux in Jy???)
     **/
    private double i1400 = 0.0;

	public long getId() {
		return id;
	}

	@SuppressWarnings("unused")
	private void setId(long id) {
		this.id = id;
	}

	public double getRightAscension() {
		return rightAscension;
	}

	public void setRightAscension(double rightAscension) {
		this.rightAscension = rightAscension;
	}

	public double getDeclination() {
		return declination;
	}

	public void setDeclination(double declination) {
		this.declination = declination;
	}

	public double getPositionAngle() {
		return positionAngle;
	}

	public void setPositionAngle(double positionAngle) {
		this.positionAngle = positionAngle;
	}

	public double getMajorAxis() {
		return majorAxis;
	}

	public void setMajorAxis(double majorAxis) {
		this.majorAxis = majorAxis;
	}

	public double getMinorAxis() {
		return minorAxis;
	}

	public void setMinorAxis(double minorAxis) {
		this.minorAxis = minorAxis;
	}

	public double getI1400() {
		return i1400;
	}

	public void setI1400(double i1400) {
		this.i1400 = i1400;
	}


}
