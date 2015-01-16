/**
 *  Copyright (c) 2014-2015 CSIRO - Australia Telescope National Facility (ATNF)
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

import java.util.Vector;

import Ice.Current;
import askap.interfaces.monitoring.MonitorPoint;
import askap.interfaces.monitoring._MonitoringProviderDisp;

/**
 * An implementation of the Ice MonitoringProvider interface. This interface
 * is published as an Ice RPC and allows the monitoring archiver and user
 * interfaces to obtain monitoring point data from the component.
 *
 * @author Ben Humphreys
 */
public class MonitoringProviderImpl extends _MonitoringProviderDisp {

    private static final long serialVersionUID = 1L;

    /**
     * This object provides access to the monitoring data via the get() method
     */
    private final MonitoringSingleton dataprovider;

    /**
     * Constructor
     *
     * @param dataprovider the object that provides the monitoring data
     */
    public MonitoringProviderImpl(MonitoringSingleton dataprovider) {
        super();
        this.dataprovider = dataprovider;
    }

    /**
     * @see askap.interfaces.monitoring._MonitoringProviderOperations#get(java.lang.String[], Ice.Current)
     */
    @Override
    public MonitorPoint[] get(String[] pointnames, Current cur) {
        final Vector<MonitorPoint> vec = dataprovider.get(pointnames);
        return vec.toArray(new MonitorPoint[vec.size()]);
    }
}
