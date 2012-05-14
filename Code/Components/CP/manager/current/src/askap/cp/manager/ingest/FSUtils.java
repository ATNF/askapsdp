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
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.channels.FileChannel;
import java.util.Enumeration;

// ASKAPsoft imports
import askap.util.ParameterSet;

public class FSUtils {
	
	/**
	 * @param dir
	 */
	public static boolean mkdir(File dir) {
		return dir.mkdir();
	}

	/**
	 * @param src
	 * @param dest
	 * @throws IOException
	 */
	public static void copyfile(File src, File dest) throws IOException {
		if (!dest.exists()) {
			dest.createNewFile();
		}

		FileChannel srcchan = new FileInputStream(src).getChannel();
		FileChannel destchan = new FileOutputStream(dest).getChannel();

		long count = 0;
		long size = srcchan.size();              
		while ((count += destchan.transferFrom(srcchan, count, size-count)) < size);

		srcchan.close();
		destchan.close();
	}

	/**
	 * @param filename
	 * @param parset
	 * @throws IOException
	 */
	@SuppressWarnings("rawtypes")
	public static void create(File filename, ParameterSet parset) throws IOException {
		FileWriter fstream = new FileWriter(filename);
		BufferedWriter out = new BufferedWriter(fstream);

		for (Enumeration e = parset.keys(); e.hasMoreElements(); /**/) {
			String key = (String) e.nextElement();
			String value = parset.getProperty(key);
			out.write(key + " = " + value + '\n');
		}

		out.close();
		fstream.close();
	}
}
