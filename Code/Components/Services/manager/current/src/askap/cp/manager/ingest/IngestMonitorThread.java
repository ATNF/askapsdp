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
package askap.cp.manager.ingest;

import askap.cp.manager.monitoring.MonitorPointStatus;
import askap.cp.manager.monitoring.MonitoringSingleton;

/**
 * This class implement a thread for monitoring the ingest pipeline. The ingest
 * monitor thread is responsible for keeping the cp related monitoring
 * point data current.
 *
 * @author Ben Humphreys
 */
public class IngestMonitorThread implements Runnable {
    private final AbstractIngestManager ingestManager;

    /**
     * Constructor
     *
     * @param manager a AbstractIngestManager instance that this class
     *                calls to get the status oof the ingest pipeline.
     * @throws NullPointerException if the "manger" parameter is null
     */
    public IngestMonitorThread(AbstractIngestManager manager) {
        if (manager == null) {
            throw new NullPointerException("Manager reference must be non-null");
        }
        ingestManager = manager;
    }

    /**
     * Entry point
     */
    @Override
    public void run() {
        while (!Thread.currentThread().isInterrupted()) {
            MonitoringSingleton mon = MonitoringSingleton.getInstance();
            if (mon == null) return;

            boolean running = ingestManager.isRunning();
            mon.update("ingest.running", running, MonitorPointStatus.OK);
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                // Polling more quickly is fine in the event
                // the sleep is interrupted
            }
        }
    }
}
