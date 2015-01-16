/**
 *  Copyright (c) 2011,2015 CSIRO - Australia Telescope National Facility (ATNF)
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

import askap.util.ParameterSet;

public class ConfigBuilder {

    /**
     * Given configuration information, builds a parameter set for the ingest
     * pipeline.
     * @param facilityConfig    facility configuration manager data
     * @param sbid      scheduling block id for the currently running scheduling
     *                  block
     * @return  a parameter set that will be accepted by the ingest pipeline
     */
    public static ParameterSet build(ParameterSet facilityConfig, long sbid) {
        ParameterSet config = new ParameterSet();

        // Adds scheduling block id
        config.add("sbid", Long.toString(sbid));

        // Use "cp.ingest" and "common" part of FCM, but those prefixes
        // get removed
        config.add(facilityConfig.subset("cp.ingest."));
        config.add(facilityConfig.subset("common."));

        return config;
    }
}
