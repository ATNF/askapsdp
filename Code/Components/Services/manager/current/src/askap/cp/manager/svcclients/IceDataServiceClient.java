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
package askap.cp.manager.svcclients;

import org.apache.log4j.Logger;

import askap.interfaces.schedblock.ISchedulingBlockServicePrx;
import askap.interfaces.schedblock.ISchedulingBlockServicePrxHelper;
import askap.interfaces.schedblock.NoSuchSchedulingBlockException;
import askap.util.ParameterSet;

public class IceDataServiceClient implements IDataServiceClient {
    /**
     * The interval (in seconds) between connection retry attempts.
     * This includes retrying connection to the Ice Locator service
     * of the TOS Data service.
     */
    private static final int RETRY_INTERVAL = 5;

    /**
     * Logger.
     */
    private static final Logger logger = Logger.getLogger(IceDataServiceClient.class.getName());

    private ISchedulingBlockServicePrx itsProxy = null;

    public IceDataServiceClient(Ice.Communicator ic, String iceIdentity) {
        logger.info("Obtaining proxy to DataServiceClient");

        while (itsProxy == null) {
            final String baseWarn = " - will retry in " + RETRY_INTERVAL
                    + " seconds";
            try {
                Ice.ObjectPrx obj = ic.stringToProxy(iceIdentity);
                itsProxy = ISchedulingBlockServicePrxHelper.checkedCast(obj);
            } catch (Ice.ConnectionRefusedException e) {
                logger.warn("Connection refused" + baseWarn);
            } catch (Ice.NoEndpointException e) {
                logger.warn("No endpoint exception" + baseWarn);
            } catch (Ice.NotRegisteredException e) {
                logger.warn("Not registered exception" + baseWarn);
            } catch (Ice.ConnectFailedException e) {
                logger.warn("Connect failed exception" + baseWarn);
            } catch (Ice.DNSException e) {
                logger.warn("DNS exception" + baseWarn);
            } catch (Ice.SocketException e) {
                logger.warn("Socket exception" + baseWarn);
            }
            if (itsProxy == null) {
                try {
                    Thread.sleep(RETRY_INTERVAL * 1000);
                } catch (InterruptedException e) {
                    // In this rare case this might happen, faster polling is ok
                }
            }
        }
        logger.info("Obtained proxy to DataServiceClient");
    }

    /**
     * @see askap.cp.manager.svcclients.IDataServiceClient#getObsParameters(long)
     */
    public ParameterSet getObsParameters(long sbid)
            throws NoSuchSchedulingBlockException {
        return new ParameterSet(itsProxy.getObsParameters(sbid));
    }
}
