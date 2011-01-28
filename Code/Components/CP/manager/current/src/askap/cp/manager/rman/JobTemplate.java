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
package askap.cp.manager.rman;

import java.util.Map;
import java.util.Hashtable;

/**
 */
public class JobTemplate {
	
    /**
     * The name of the job this template will create.
     */
    private String itsName;

    /**
     * The script (including full path) which will be executed
     * when the job is executed
     */
    private String itsPathToScript;

    /**
     * List of jobs that any job created with this template
     * will depend on
     */
    private Map<IJob, DependType> itsDependencies;

    /**
     * Dependency type.
     */
    enum DependType {
        /** Start after dependent job starts */
        AFTERSTART,

        /** Start after dependent job completes with no-error */
        AFTEROK,
        
        /** Start the job only if the dependent job fails */
        AFTERNOTOK
    }    
    
    /**
     * Constructor.
     * @param name the name of the job this template will create.
     */
    public JobTemplate(String name)
    {
    	itsName = name;
    	itsDependencies = new Hashtable<IJob, DependType>();
    }

    /**
     * Change the name of the job.
     * @param name name the new name of the job.
     */
    public void setName(String name)
    {
    	itsName = name;
    }

    /**
     * Get the name of the job.
     * @return the name of the job this template will create.
     */
    public String getName()
    {
    	return itsName;
    }

    /**
     * Set the script or executable to be executed when this job runs. This
     * should include the full path to the script/executable.
     * 
     * @param script the path and command. (e.g. /tmp/myscript.sh)
     */
    public void setScriptLocation(String script)
    {
    	itsPathToScript = script;
    }

    /**
     * Get the pathname/commandname of the script to be executed when this
     * job runs.
     * @return  the path and command. (e.g. /tmp/myscript.sh)
     */
    public String getScriptLocation()
    {
    	return itsPathToScript;
    }

    /**
     * Adds dependency information to this job template. This jobs created
     * with this template will then not start until the dependencies are
     * fulfilled.
     * 
     * @param dependency 	the job for which this one has a dependency.
     * @param type 			the type of dependency.
     */
    public void addDependency(IJob dependency, DependType type)
    {
    	itsDependencies.put(dependency, type);
    }

    /**
     * Remove a dependency from this job template.
     * @param dependency	the dependency to remove.
     */
    public void removeDependency(IJob dependency)
    {
    	itsDependencies.remove(dependency);
    }

    /**
     * Remove all dependencies from this job template.
     */
    public void removeAllDependencies()
    {
    	itsDependencies.clear();
    }

    
    /**
     * Get a map of all dependencies.
     * @return a map of all dependencies.
     */
    public Map<IJob, DependType> getDependencies()
    {
    	return itsDependencies;
    }
}
