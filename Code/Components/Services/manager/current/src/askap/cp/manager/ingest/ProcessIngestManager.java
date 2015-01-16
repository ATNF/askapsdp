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
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.StringTokenizer;

import org.apache.log4j.Logger;

import askap.interfaces.cp.PipelineStartException;
import askap.util.ParameterSet;

/**
 *
 */
public class ProcessIngestManager extends AbstractIngestManager {

    /**
     * Logger
     */
    private static final Logger logger = Logger.getLogger(ProcessIngestManager.class
            .getName());

    /**
     * Ingest pipeline process
     */
    private Process itsIngestProcess = null;

    /**
     * Constructor
     */
    public ProcessIngestManager(ParameterSet parset) {
        super(parset);
    }

    /**
     * Executes the ingest pipeline
     *
     * @param workdir the directory that will form the working directory for
     *                the execution of the ingest pipeline.
     * @throws PipelineStartException if the ingest pipeline fails to start.
     */
    @Override
    protected synchronized void executeIngestPipeline(File workdir)
            throws PipelineStartException {
        // The superclass calls this method and is not meant to call execute
        // if the ingest pipeline is running
        assert (!isRunning());

        String command = parset().getString("ingest.command");
        String args = parset().getString("ingest.args");
        String logfile = parset().getString("ingest.logfile", "cpingest.log");

        List<String> cmdLine = buildCommandLine(command, args, logfile);

        try {
            ProcessBuilder pb = new ProcessBuilder(cmdLine)
                    .redirectErrorStream(true);
            pb.directory(workdir);
            itsIngestProcess = pb.start();
        } catch (IOException e) {
            logger.error("Failed to execute ingest pipeline process: "
                    + e.getMessage());
            throw new PipelineStartException(e.getMessage());
        }

        // This method is not meant to return until the ingest pipeline is
        // running and ready to accept data from the correlator. Since we don't
        // actually know when this is, for now just sleep for a few seconds
        // TODO: Need to adhere to the interface contract a bit better. Need to
        // block until the ingest pipeline is running or until an error occurs
        // which prevents it from starting.
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
        }

        if (!isRunning()) {
            logger.error("Ingest pipeline failed to start");
            throw new askap.interfaces.cp.PipelineStartException(
                    "Ingest pipeline failed to start");
        }
    }

    /**
     * Kills the ingest pipeline. Normally the ingest pipeline will shutdown
     * when the observation has concluded, so this is a mechanism to abort it
     * early.
     */
    @Override
    protected synchronized void abortIngestPipeline() {
        if (itsIngestProcess != null) {
            // Destroy the process, but assume this is non-blocking
            itsIngestProcess.destroy();

            // Wait until process has exited
            try {
                itsIngestProcess.waitFor();
            } catch (InterruptedException e) {
            }

            // When we get here, the process has exited
            itsIngestProcess = null;
        }
    }

    /**
     * Returns true if the ingest pipeline is running, otherwise false.
     */
    @Override
    public synchronized boolean isRunning() {
        if (itsIngestProcess == null) {
            return false;
        } else {
            try {
                itsIngestProcess.exitValue();
                itsIngestProcess = null;
                return false;
            } catch (IllegalThreadStateException e) {
                // Means the process has not exited
                return true;
            }
        }

    }

    /**
     * Builds the command line to be provided to the ProcessBuilder
     *
     * @param command
     * @param args
     * @param logfile
     * @return
     */
    private static List<String> buildCommandLine(String command, String args,
                                                 String logfile) {
        List<String> lst = new ArrayList<String>();
        lst.add(command);
        StringTokenizer st = new StringTokenizer(args);
        while (st.hasMoreTokens()) {
            lst.add(st.nextToken());
        }

        lst.add(">");
        lst.add(logfile);
        lst.add("2>&1");
        return lst;
    }
}
