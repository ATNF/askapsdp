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
 */
package askap.cp.manager.ingest;

// Java core imports

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;


// ASKAPsoft imports
import askap.util.ParameterSet;

public class FSUtils {
    /**
     * Creates a file, populating it with the contents of the parameter set. The
     * parameter set is just a Map<String, String> and the file created will be an
     * ASCII file with contents like:
     * <pre>
     * key1 = value1
     * key2 = value2
     * <and so on>
     * </pre>
     * <p/>
     * The output will be sorted alphabetically.
     *
     * @param filename filename to create
     * @param parset   the parameter set which will used to populate the file.
     * @throws IOException if the file creating or writing to the file fails.
     */
    public static void create(File filename, ParameterSet parset) throws IOException {
        FileWriter fstream = new FileWriter(filename);
        BufferedWriter out = new BufferedWriter(fstream);

        List<String> lst = new ArrayList<String>();

        // Add each key/value to a list for sorting
        for (String key : parset.keys()) {
            String value = parset.getString(key);
            lst.add(key + " = " + value + '\n');
        }

        Collections.sort(lst);
        for (String s : lst) {
            out.write(s);
        }

        out.close();
        fstream.close();
    }
}
