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

import java.io.File;

import org.apache.log4j.Logger;

import askap.util.ParameterSet;

/**
 * This dummy ingest pipeline manager does nothing when execute/abort
 * is called except log messages at level INFO.
 */
public class DummyIngestManager extends AbstractIngestManager {

    /**
     * Logger
     */
    private static final Logger logger = Logger.getLogger(DummyIngestManager.class.getName());

    /**
     * Constructor
     */
    public DummyIngestManager(ParameterSet parset) {
        super(parset);
    }

    /**
     * Dummy execute - does nothing except log a message
     */
    @Override
    protected void executeIngestPipeline(File workdir) {
        logger.info("DummyIngestPipeline: Execute");
    }

    /**
     * Dummy abort - does nothnig except log a message
     */
    @Override
    protected void abortIngestPipeline() {
        logger.info("DummyIngestPipeline: Abort");
    }

    /**
     * Always returns false. This essentially mimics an ingest pipeline
     * that starts and finishes immediately.
     */
    @Override
    public boolean isRunning() {
        return false;
    }
}
